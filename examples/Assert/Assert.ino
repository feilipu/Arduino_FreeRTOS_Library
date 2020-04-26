/*
 * Example of FreeRTOS configASSERT macro
 * https://www.freertos.org/a00110.html#configASSERT
 */

#include <Arduino_FreeRTOS.h>

const boolean valueToAssert = true;

// The setup function runs once when you press reset or power the board
void setup() {

  // Assert value is true, execution doesn't stop.
  configASSERT(valueToAssert == true);

  // Assert value is false, FreeRTOS execution stops and start to blink main led two times with 4 second cycle.
  configASSERT(valueToAssert == false);
}
    

void loop()
{
  // Empty. Things are done in Tasks.
}
