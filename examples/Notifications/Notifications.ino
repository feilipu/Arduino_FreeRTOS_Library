/**
   Example of a Arduino interruption and RTOS Task Notification.
   https://www.freertos.org/RTOS_Task_Notification_As_Binary_Semaphore.html
*/

// Include Arduino FreeRTOS library
#include <Arduino_FreeRTOS.h>

/**
   Declaring a global TaskHandle for the led task.
*/
TaskHandle_t taskNotificationHandler;

void setup() {

  // Configure pin 2 as an input and enable the internal pull-up resistor.
  pinMode(2, INPUT_PULLUP);

  // Create task for FreeRTOS notification
  xTaskCreate(TaskNotification, // Task function
              "Notification", // Task name
              128, // Stack size
              NULL,
              3, // Priority
              &taskNotificationHandler ); // TaskHandle
}

void loop() {

}

/*
   Notification task.
*/
void TaskNotification(void *pvParameters)
{
  (void) pvParameters;

  int digitalPin = 2;

  Serial.begin(115200);

  attachInterrupt(digitalPinToInterrupt(digitalPin), digitalPinInterruptHandler, LOW);

  for (;;) {

    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
      Serial.println("Notification received");
    }

  }
}


void digitalPinInterruptHandler() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(taskNotificationHandler, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    taskYIELD();
  }
}
