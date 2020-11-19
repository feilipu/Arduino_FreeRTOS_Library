/*
 * Example of a Arduino interruption and RTOS Binary Semaphore
 * https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html
 */


// Include Arduino FreeRTOS library
#include <Arduino_FreeRTOS.h>

// Include semaphore supoport
#include <semphr.h>

/* 
 * Declaring a global variable of type SemaphoreHandle_t 
 * 
 */
SemaphoreHandle_t interruptSemaphore;

void setup() {

  // Configure pin 2 as an input and enable the internal pull-up resistor
  pinMode(2, INPUT_PULLUP);

 // Create task for Arduino led 
  xTaskCreate(TaskLed, // Task function
              "Led", // Task name
              128, // Stack size 
              NULL, 
              0, // Priority
              NULL );

  /**
   * Create a binary semaphore.
   * https://www.freertos.org/xSemaphoreCreateBinary.html
   */
  interruptSemaphore = xSemaphoreCreateBinary();
  if (interruptSemaphore != NULL) {
    // Attach interrupt for Arduino digital pin
    attachInterrupt(digitalPinToInterrupt(2), interruptHandler, LOW);
  }

  
}

void loop() {}


void interruptHandler() {
  /**
   * Give semaphore in the interrupt handler
   * https://www.freertos.org/a00124.html
   */
  
  xSemaphoreGiveFromISR(interruptSemaphore, NULL);
}


/* 
 * Led task. 
 */
void TaskLed(void *pvParameters)
{
  (void) pvParameters;

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) {
    
    /**
     * Take the semaphore.
     * https://www.freertos.org/a00122.html
     */
    if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
    vTaskDelay(10);
  }
}
