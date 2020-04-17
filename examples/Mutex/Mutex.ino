/*
   Example of a FreeRTOS mutex
   https://www.freertos.org/Real-time-embedded-RTOS-mutexes.html
*/

// Include Arduino FreeRTOS library
#include <Arduino_FreeRTOS.h>


// Include mutex support
#include <semphr.h>

/*
   Declaring a global variable of type SemaphoreHandle_t

*/
SemaphoreHandle_t mutex;

int globalCount = 0;

void setup() {

  Serial.begin(9600);

  /**
       Create a mutex.
       https://www.freertos.org/CreateMutex.html
  */
  mutex = xSemaphoreCreateMutex();
  if (mutex != NULL) {
    Serial.println("Mutex created");
  }

  /**
     Create tasks
  */
  xTaskCreate(TaskMutex, // Task function
              "Task1", // Task name for humans
              128, 
              1000, // Task parameter
              1, // Task priority
              NULL);

  xTaskCreate(TaskMutex, "Task2", 128, 1000, 1, NULL);

}

void loop() {}

void TaskMutex(void *pvParameters)
{
  int delay = *((int*)pvParameters); // Use task parameters to define delay

  for (;;)
  {
    /**
       Take mutex
       https://www.freertos.org/a00122.html
    */
    if (xSemaphoreTake(mutex, 10) == pdTRUE)
    {
      Serial.print(pcTaskGetName(NULL)); // Get task name
      Serial.print(", Count readed value: ");
      Serial.print(globalCount);

      globalCount++;

      Serial.print(", Updated value: ");
      Serial.print(globalCount);

      Serial.println();
      /**
         Give mutex
         https://www.freertos.org/a00123.html
      */
      xSemaphoreGive(mutex);
    }

    vTaskDelay(delay / portTICK_PERIOD_MS);
  }
}
