/*
 * Copyright (C) 2023 Phillip Stevens  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * This file is NOT part of the FreeRTOS distribution.
 *
 */
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <util/delay.h>

#include <Arduino.h>

/* FreeRTOS includes. */
#include "Arduino_FreeRTOS.h"
#include "task.h"
#include "timers.h"

extern void setup(void);
extern void loop(void);

/*-----------------------------------------------------------*/

void initVariant(void) __attribute__ ((OS_main));

void initVariant(void)
{
    // As the Task stacks are on heap before Task allocated heap variables,
    // the library default __malloc_heap_end = 0 doesn't work.
    __malloc_heap_end = (char *)(RAMEND - __malloc_margin);

#if defined(USBCON)
    USBDevice.attach();
#endif

    setup();                    // the normal Arduino setup() function is run here.
    vTaskStartScheduler();      // initialise and run the freeRTOS scheduler. Execution should never return here.
}


/*-----------------------------------------------------------*/
#if ( configUSE_IDLE_HOOK == 1 )
/*
 * Call the user defined loop() function from within the idle task.
 * This allows the application designer to add background functionality
 * without the overhead of a separate task.
 *
 * NOTE: vApplicationIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK.
 *
 */
void vApplicationIdleHook( void ) __attribute__ ((weak));

void vApplicationIdleHook( void )
{
    loop();                     // the normal Arduino loop() function is run here.
    if (serialEventRun) serialEventRun();
}

#else
    void loop() {} //Empty loop function
#endif /* configUSE_IDLE_HOOK == 1 */
/*-----------------------------------------------------------*/

#if ( configUSE_MALLOC_FAILED_HOOK == 1 || configCHECK_FOR_STACK_OVERFLOW >= 1 || configDEFAULT_ASSERT == 1 )

/**
 * Private function to enable board led to use it in application hooks
 */
void prvSetMainLedOn( void ) 
{

#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) // Arduino Mega with 2560
    DDRB  |= _BV(DDB7);
    PORTB |= _BV(PORTB7);       // Main (red PB7) LED on. Main LED on.

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284PA__) // Seeed Goldilocks with 1284p
    DDRB  |= _BV(DDB7);
    PORTB |= _BV(PORTB7);       // Main (red PB7) LED on. Main LED on.

#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) // assume we're using an Arduino Uno with 328p
    DDRB  |= _BV(DDB5);
    PORTB |= _BV(PORTB5);       // Main (red PB5) LED on. Main LED on.

#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) // assume we're using an Arduino Leonardo with 32u4
    DDRC  |= _BV(DDC7);
    PORTC |= _BV(PORTC7);       // Main (red PC7) LED on. Main LED on.

#endif

}

/**
 * Private function to blink board led to use it in application hooks
 */
void prvBlinkMainLed( void ) 
{

#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)  // Mega with 2560
        PINB  |= _BV(PINB7);       // Main (red PB7) LED toggle.

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284PA__) // Seeed Goldilocks with 1284p
        PINB  |= _BV(PINB7);       // Main (red PB7) LED toggle.

#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) // assume we're using an Arduino Uno with 328p
        PINB  |= _BV(PINB5);       // Main (red PB5) LED toggle.

#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) // assume we're using an Arduino Leonardo with 32u4
        PINC  |= _BV(PINC7);       // Main (red PC7) LED toggle.

#endif

}

#endif

#if ( configUSE_MALLOC_FAILED_HOOK == 1 )
/*---------------------------------------------------------------------------*\
Usage:
    called by task system when a malloc failure is noticed
Description:
    Malloc failure handler -- Shut down all interrupts, send serious complaint
    to command port. FAST Blink on main LED.
Arguments:
    pxTask - pointer to task handle
    pcTaskName - pointer to task name
Results:
    <none>
Notes:
    This routine will never return.
    This routine is referenced in the task.c file of FreeRTOS as an extern.
\*---------------------------------------------------------------------------*/
void vApplicationMallocFailedHook( void ) __attribute__ ((weak));

void vApplicationMallocFailedHook( void )
{
    prvSetMainLedOn(); // Main LED on.
    
    for(;;)
    {
        _delay_ms(50);
        prvBlinkMainLed(); // Main LED fast blink.
    }
}

#endif /* configUSE_MALLOC_FAILED_HOOK == 1 */
/*-----------------------------------------------------------*/


#if ( configCHECK_FOR_STACK_OVERFLOW >= 1 )

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    char * pcTaskName ) __attribute__ ((weak));

void vApplicationStackOverflowHook( TaskHandle_t xTask __attribute__ ((unused)),
                                    char * pcTaskName __attribute__ ((unused)) )
{

    prvSetMainLedOn(); // Main LED on.

    for(;;)
    {
        _delay_ms(2000);
        prvBlinkMainLed();  // Main LED slow blink.
    }
}

#endif /* configCHECK_FOR_STACK_OVERFLOW >= 1 */
/*-----------------------------------------------------------*/

#if ( configSUPPORT_STATIC_ALLOCATION >= 1 )

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * puxIdleTaskStackSize ) __attribute__ ((weak));

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * puxIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if ( configUSE_TIMERS >= 1 )

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     configSTACK_DEPTH_TYPE * puxTimerTaskStackSize ) __attribute__ ((weak));

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     configSTACK_DEPTH_TYPE * puxTimerTaskStackSize )
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *puxTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#endif /* configUSE_TIMERS >= 1 */

#endif /* configSUPPORT_STATIC_ALLOCATION >= 1 */

/**
 * configASSERT default implementation
 */
#if configDEFAULT_ASSERT == 1

void vApplicationAssertHook() {

    taskDISABLE_INTERRUPTS(); // Disable task interrupts

    prvSetMainLedOn(); // Main LED on.
    for(;;)
    {
        _delay_ms(100);
        prvBlinkMainLed(); // Led off.

        _delay_ms(2000);
        prvBlinkMainLed(); // Led on.

        _delay_ms(100);
        prvBlinkMainLed(); // Led off

        _delay_ms(100);
        prvBlinkMainLed(); // Led on.
    }
}
#endif
