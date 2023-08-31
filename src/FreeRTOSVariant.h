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
 * Updated on Aug 2023 by gpb01 to add the capability to use the 16 bit Timer 3
 * on LGT8F328 MCU for xTaskIncrementTick.
 *
 */

#ifndef freeRTOSVariant_h
#define freeRTOSVariant_h

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include <avr/wdt.h>

// System Tick - Scheduler timer
// Use the Watchdog timer, and choose the rate at which scheduler interrupts will occur.

/* Watchdog Timer is 128kHz nominal, but 120 kHz at 5V DC and 25 degrees is actually more accurate, from data sheet. */

#ifndef portUSE_WDTO
    #define portUSE_WDTO        WDTO_15MS    // portUSE_WDTO to use the Watchdog Timer for xTaskIncrementTick
#endif

/* Watchdog period options:     WDTO_15MS
                                WDTO_30MS
                                WDTO_60MS
                                WDTO_120MS
                                WDTO_250MS
                                WDTO_500MS
                                WDTO_1S
                                WDTO_2S
*/

   
// #define portUSE_LGT_TIMER3                  // portUSE_LGT_TIMER3 uncomment to use the 16 bit Timer 3 (on LGT8F328 MCU) for xTaskIncrementTick


#if defined( portUSE_LGT_TIMER3 )
   
    #if defined( portUSE_WDTO )
        #undef portUSE_WDTO
    #endif
   
    // Formula for the frequency is: f = F_CPU / (PRESCALER * (1 + COUNTER_TOP))
    #define PRESCALER           8

    #if ( F_CPU == 32000000UL )
        // Assuming the MCU clock of 32MHz, Timer 3 is 16 bits, prescaler 8 and counter top 60004, the resulting tick period is 15 ms (66.66 Hz).
        #define TICK_PERIOD_15MS    60004UL
    #elif ( F_CPU == 16000000UL )
        // Assuming the MCU clock of 16MHz, Timer 3 is 16 bits, prescaler 8 and counter top 30002, the resulting tick period is 15 ms (66.66 Hz).
        #define TICK_PERIOD_15MS    30002UL
    #elif ( F_CPU == 8000000UL )
        // Assuming the MCU clock of  8MHz, Timer 3 is 16 bits, prescaler 8 and counter top 15001, the resulting tick period is 15 ms (66.66 Hz).
        #define TICK_PERIOD_15MS    15001UL
    #else   
        #error "Unsupported MCU frequency."
    #endif // F_CPU options

    #define configTICK_RATE_HZ  ( (TickType_t) ( F_CPU / (uint32_t) ( PRESCALER * ( 1 + TICK_PERIOD_15MS ) ) ) )
    #define portTICK_PERIOD_MS  ( (TickType_t) ( 1000 / configTICK_RATE_HZ ) )   

#elif defined( portUSE_WDTO )

    #define configTICK_RATE_HZ  ( (TickType_t)( (uint32_t)128000 >> (portUSE_WDTO + 11) ) )  // 2^11 = 2048 WDT scaler for 128kHz Timer
    #define portTICK_PERIOD_MS  ( (TickType_t) _BV( portUSE_WDTO + 4 ) )
   
#else
   
    #warning "Variant configuration must define `configTICK_RATE_HZ` and `portTICK_PERIOD_MS` as either a macro or a constant"
    #define configTICK_RATE_HZ  1
    #define portTICK_PERIOD_MS  ( (TickType_t) 1000 / configTICK_RATE_HZ )
   
#endif

/*-----------------------------------------------------------*/

#ifndef INC_TASK_H
#include "Arduino_FreeRTOS.h"
#include "task.h"
#endif

void initVariant(void);

void vApplicationIdleHook( void );

void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName );

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * pulIdleTaskStackSize );
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     configSTACK_DEPTH_TYPE * pulTimerTaskStackSize );

#ifdef __cplusplus
}
#endif

#endif // freeRTOSVariant_h
