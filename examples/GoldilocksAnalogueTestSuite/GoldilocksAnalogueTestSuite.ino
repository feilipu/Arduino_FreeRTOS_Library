#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <util/atomic.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <SPI.h>
#include <SD.h>

// This example only works with Goldilocks Analogue, as DAC and SPI SRAM is required.

// INSTALLATION OF THE 4 FOLLOWING LIBRARIES REQUIRED!

// From Library: FreeRTOS
#include <Arduino_FreeRTOS.h>
#include <list.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

// From Library: AVR Standard C Time Library
#include <time.h>

// From Library: Goldilocks Analogue DAC Library
#include <DAC.h>

// From Library: Goldilocks Analogue SPI RAM Library
#include <SPIRAM.h>
#include <SPIRAM_ringBuffer.h>

#include "GA_Header.h"

/*--------------------------------------------------*/
/*------------------- Globals ----------------------*/
/*--------------------------------------------------*/

DAC_value_t mod7_value; // location to store the ADC value before it is processed.

uint16_t ch_A_out; // storage for the values to be written to MCP4822
uint16_t ch_B_out;

SPIRAM_ringBuffer_t SRAM_delayBuffer; // structure to hold the SRAM ringbuffer info.

filter_t tx_filter; // instantiate a filter, which can be initialised to be a LPF (or BPF or HPF) later.

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

// change this to match your SD shield or module;
// GoldilocksAnalogue SD shield: pin 14
uint8_t const chipSelect = 14;

// Create a Semaphore binary flag for the Serial Port. To ensure only single access.
SemaphoreHandle_t xSerialSemaphore;

// define two tasks to operate this test suite.
static void TaskReport(void *pvParameters); // Report on the status reguarly using USART.
static void TaskAnalogue(void *pvParameters);   // Manage Analogue set-up, then sleep.

/*--------------------------------------------------*/
/*-------------------- Set Up ----------------------*/
/*--------------------------------------------------*/

void setup() {
  // put your setup code here, to run once:
  // Open serial communications and wait for port to open:
  Serial.begin(38400);

  setup_RTC_interrupt();

  {
    tm CurrTimeDate;       // set up an array for the RTC info.
    // <year yyyy> <month mm Jan=0> <date dd> <day d Sun=0> <hour hh> <minute mm> <second ss>

    CurrTimeDate.tm_year = (uint8_t)    ( 2016 - 1900 );
    CurrTimeDate.tm_mon = (uint8_t)     1; // January is month 0, February is month 1, etc
    CurrTimeDate.tm_mday = (uint8_t)    18;
    CurrTimeDate.tm_hour = (uint8_t)    17;
    CurrTimeDate.tm_min = (uint8_t)     16;
    CurrTimeDate.tm_sec = (uint8_t)     0;

    set_system_time( mktime( (ptm)&CurrTimeDate));
  }

  // Semaphores are useful to stop a task proceeding, where it should be stopped because it is using a resource, such as the Serial port.
  // But they should only be used whilst the scheduler is running.
  if ( xSerialSemaphore == NULL )          // Check to see if the Serial Semaphore has not been created.
  {
    xSerialSemaphore = xSemaphoreCreateBinary(); // binary semaphore for Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // make the Serial Port available
  }

  // we'll use the SD Card initialization code from the utility libraries
  // since we're just testing if the card is working!
  Serial.print(F("\nInitializing SD card..."));
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card inserted?"));
    Serial.println(F("* is your wiring correct?"));
    Serial.println(F("* did you change the chipSelect pin to match your shield or module?"));
  } else {
    Serial.println(F("Wiring is correct and a card is present."));
  }

  // print the type of card
  Serial.print(F("\nCard type: "));
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println(F("SD1"));
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println(F("SD2"));
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println(F("SDHC"));
      break;
    default:
      Serial.println(F("Unknown SD Card Type"));
      break;
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println(F("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card"));
    return;
  }

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print(F("\nVolume type is FAT"));
  Serial.println(volume.fatType(), DEC);
  Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                         // SD card blocks are always 512 bytes
  Serial.print(F("Volume size (bytes): "));
  Serial.println(volumesize);
  Serial.print(F("Volume size (Kbytes): "));
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print(F("Volume size (Mbytes): "));
  volumesize /= 1024;
  Serial.println(volumesize);

  Serial.println(F("\nFiles found on the card (name, date and size in bytes): "));
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);

  // Now set up two tasks to help us with testing.
  xTaskCreate(
    TaskReport
    ,  (const portCHAR *)"RedLED" // report reguarly on the status of memory, and the tick values.
    ,  256        // Stack size
    ,  NULL
    ,  1          // priority
    ,  NULL ); // */

  xTaskCreate(
    TaskAnalogue
    ,  (const portCHAR *) "Analogue"
    ,  256        // This stack size can be checked & adjusted by reading Highwater
    ,  NULL
    ,  1          // priority
    ,  NULL ); // */

  // Start the task scheduler, which takes over control of scheduling individual tasks.
  // The scheduler is started in initVariant() found in variantHooks.c
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

static void TaskAnalogue(void *pvParameters) // Prepare the DAC
{
  (void) pvParameters;
  TickType_t xLastWakeTime;
  /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
    API function. */
  xLastWakeTime = xTaskGetTickCount();

  // Create the SPI SRAM ring-buffer used by audio delay loop.
  SPIRAM_ringBuffer_InitBuffer( &SRAM_delayBuffer, (uint_farptr_t)(RAM0_ADDR), sizeof(uint8_t) * DELAY);

  Serial.print(F("DAC_Codec_init "));
  // initialise the USART 1 MSPIM bus specifically for DAC use, with the post-latch configuration.
  // pre-latch for audio or AC signals (FALSE), or post-latch for single value setting or DC values (TRUE).
  DAC_init(FALSE);

  Serial.print(F("will "));
  // Initialise the sample interrupt timer.
  // set up the sampling Timer3 to 48000Hz (or lower), runs at audio sampling rate in Hz.
  DAC_Timer3_init(SAMPLE_RATE);

  Serial.print(F("very "));
  // Initialise the filter to be a Low Pass Filter.
  tx_filter.cutoff = 0xc000;          // set filter to 3/8 of sample frequency.
  setIIRFilterLPF( &tx_filter );      // initialise transmit sample filter

  Serial.print(F("soon "));
  // set up ADC sampling on the ADC7 (Microphone).
  AudioCodec_ADC_init();
  
  Serial.print (F("be "));
  // Set the call back function to do the audio processing.
  // Done this way so that we can change the audio handling depending on what we want to achieve.
  DAC_setHandler(audioCodec_dsp, &ch_A_out, &ch_B_out);

  Serial.println(F("done."));

  //  vTaskSuspend(NULL);           // Well, we're pretty much done here. Let's suspend the Task.
  //  vTaskEndScheduler();          // Or just kill the FreeRTOS scheduler. Rely on Timer3 Interrupt for regular output.

  for (;;)
  {
    // See if we can obtain the Serial Semaphore.
    // If the semaphore is not available, wait 5 ticks to see if it becomes free.
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      // We were able to obtain the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.

      Serial.print(F("Audio Stack HighWater @ "));
      Serial.println(uxTaskGetStackHighWaterMark(NULL));

      xSemaphoreGive( xSerialSemaphore ); // Now free the Serial Port for others.
    }
    vTaskDelayUntil( &xLastWakeTime, ( 8192 / portTICK_PERIOD_MS ) );
  }
}

static void TaskReport(void *pvParameters) // report on the status of the device
{
  (void) pvParameters;;
  TickType_t xLastWakeTime;
  /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
    API function. */
  xLastWakeTime = xTaskGetTickCount();

  time_t currentTick;     // set up a location for the current time stamp

  for (;;)
  {
    // See if we can obtain the Serial Semaphore.
    // If the semaphore is not available, wait 5 ticks to see if it becomes free.
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      // We were able to obtain the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.

      Serial.print(F("Report Stack HighWater @ "));
      Serial.print(uxTaskGetStackHighWaterMark(NULL));

      Serial.print(F(" Current Time: "));
      time((time_t *)&currentTick);
      Serial.println(ctime( (time_t *)&currentTick));

      xSemaphoreGive( xSerialSemaphore ); // Now free the Serial Port for others.
    }
    vTaskDelayUntil( &xLastWakeTime, ( 2048 / portTICK_PERIOD_MS ) );
  }
}

/*--------------------------------------------------*/
/*-------------------- Functions -------------------*/
/*--------------------------------------------------*/

void audioCodec_dsp( uint16_t * ch_A,  uint16_t * ch_B)
{
  int16_t xn;
  uint8_t cn;

  if ( SPIRAM_ringBuffer_GetCount(&SRAM_delayBuffer) >= DELAY )
  {
    cn = SPIRAM_ringBuffer_Pop(&SRAM_delayBuffer);
  }
  else
  {
    cn = 0x80 ^ 0x55; // put A-Law nulled signal on the output.
  }

  alaw_expand1(&cn, &xn); // expand the A-Law compression

  *ch_A = *ch_B = (uint16_t)(xn + 0x7fff); // put signal out on A & B channel.

  AudioCodec_ADC(&mod7_value.u16);

  xn = mod7_value.u16 - 0x7fe0; // centre the sample to 0 by subtracting 1/2 10bit range.

  IIRFilter( &tx_filter, &xn);  // filter sample train

  alaw_compress1(&xn, &cn); // compress using A-Law

  if ( SPIRAM_ringBuffer_GetCount(&SRAM_delayBuffer) <= DELAY )
  {
    SPIRAM_ringBuffer_Poke(&SRAM_delayBuffer, cn);
  }
}


/*--------------------------------------------------*/
/*---------------------- Loop ----------------------*/
/*--------------------------------------------------*/

void loop() {
  // Remember that loop() is simply the freeRTOS idle task.
  // It is only something to do, when there's nothing else to do.

  // There are several macros provided in the header file to put
  // the device into sleep mode.
  // SLEEP_MODE_IDLE (0)
  // SLEEP_MODE_ADC _BV(SM0)
  // SLEEP_MODE_PWR_DOWN _BV(SM1)
  // SLEEP_MODE_PWR_SAVE (_BV(SM0) | _BV(SM1))
  // SLEEP_MODE_STANDBY (_BV(SM1) | _BV(SM2))
  // SLEEP_MODE_EXT_STANDBY (_BV(SM0) | _BV(SM1) | _BV(SM2))

  set_sleep_mode( SLEEP_MODE_IDLE );

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    sleep_enable();

#if defined(BODS) && defined(BODSE) // Only if there is support to disable the brown-out detection.
    sleep_bod_disable();
#endif
  }
  sleep_cpu(); // good night.

  // Ugh. I've been woken up. Better disable sleep mode.
  sleep_disable();
}
