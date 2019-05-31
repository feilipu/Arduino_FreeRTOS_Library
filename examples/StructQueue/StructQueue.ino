/*
 * Example of a basic FreeRTOS queue
 * https://www.freertos.org/Embedded-RTOS-Queues.html
 */

// Include Arduino FreeRTOS library
#include <Arduino_FreeRTOS.h>

// Include queue support
#include <queue.h>

// Define a struct
struct pinRead {
  int pin;
  int value;
};

/* 
 * Declaring a global variable of type QueueHandle_t 
 * 
 */
QueueHandle_t structQueue;

void setup() {

  /**
   * Create a queue.
   * https://www.freertos.org/a00116.html
   */
  structQueue = xQueueCreate(10, // Queue length
                              sizeof(struct pinRead) // Queue item size
                              );
  
  if (structQueue != NULL) {
    
    // Create task that consumes the queue if it was created.
    xTaskCreate(TaskSerial, // Task function
                "Serial", // A name just for humans
                128,  // This stack size can be checked & adjusted by reading the Stack Highwater
                NULL, 
                2, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                NULL);


    // Create task that publish data in the queue if it was created.
    xTaskCreate(TaskAnalogReadPin0, // Task function
                "AnalogReadPin0", // Task name
                128,  // Stack size
                NULL, 
                1, // Priority
                NULL);

    // Create other task that publish data in the queue if it was created.
    xTaskCreate(TaskAnalogReadPin1, // Task function
                "AnalogReadPin1", // Task name
                128,  // Stack size
                NULL, 
                1, // Priority
                NULL);
    
  }


  xTaskCreate(TaskBlink, // Task function
              "Blink", // Task name
              128, // Stack size 
              NULL, 
              0, // Priority
              NULL );

}

void loop() {}


/**
 * Analog read task for Pin A0
 * Reads an analog input on pin 0 and send the readed value through the queue.
 * See Blink_AnalogRead example.
 */
void TaskAnalogReadPin0(void *pvParameters)
{
  (void) pvParameters;
  
  for (;;)
  {
    // Read the input on analog pin 0:
    struct pinRead currentPinRead;
    currentPinRead.pin = 0;
    currentPinRead.value = analogRead(A0);

    /**
     * Post an item on a queue.
     * https://www.freertos.org/a00117.html
     */
    xQueueSend(structQueue, &currentPinRead, portMAX_DELAY);

    // One tick delay (15ms) in between reads for stability
    vTaskDelay(1);
  }
}


/**
 * Analog read task for Pin A1
 * Reads an analog input on pin 1 and send the readed value through the queue.
 * See Blink_AnalogRead example.
 */
void TaskAnalogReadPin1(void *pvParameters)
{
  (void) pvParameters;
  
  for (;;)
  {
    // Read the input on analog pin 1:
    struct pinRead currentPinRead;
    currentPinRead.pin = 1;
    currentPinRead.value = analogRead(A1);

    /**
     * Post an item on a queue.
     * https://www.freertos.org/a00117.html
     */
    xQueueSend(structQueue, &currentPinRead, portMAX_DELAY);

    // One tick delay (15ms) in between reads for stability
    vTaskDelay(1);
  }
}

/**
 * Serial task.
 * Prints the received items from the queue to the serial monitor.
 */
void TaskSerial(void * pvParameters) {
  (void) pvParameters;

  // Init Arduino serial
  Serial.begin(9600);

  // Wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  while (!Serial) {
    vTaskDelay(1);
  }
  
  for (;;) 
  {

    struct pinRead currentPinRead;

    /**
     * Read an item from a queue.
     * https://www.freertos.org/a00118.html
     */
    if (xQueueReceive(structQueue, &currentPinRead, portMAX_DELAY) == pdPASS) {
      Serial.print("Pin: ");
      Serial.print(currentPinRead.pin);
      Serial.print(" Value: ");
      Serial.println(currentPinRead.value);
    }
  }
}

/* 
 * Blink task. 
 * See Blink_AnalogRead example. 
 */
void TaskBlink(void *pvParameters)
{
  (void) pvParameters;

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay( 250 / portTICK_PERIOD_MS );
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay( 250 / portTICK_PERIOD_MS );
  }
}
