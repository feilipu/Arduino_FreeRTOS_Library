/*
 * Copyright (C) 2022 Phillip Stevens  All Rights Reserved.
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

#ifndef freeRTOSVariant_h
#define freeRTOSVariant_h

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include <avr/wdt.h>

// System Tick - Scheduler timer
// Use the Watchdog timer, and choose the rate at which scheduler interrupts will occur.

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

/* Watchdog Timer is 128kHz nominal, but 120 kHz at 5V DC and 25 degrees is actually more accurate, from data sheet. */
#define configTICK_RATE_HZ  ( (TickType_t)( (uint32_t)128000 >> (portUSE_WDTO + 11) ) )  // 2^11 = 2048 WDT scaler for 128kHz Timer

/*-----------------------------------------------------------*/
// To switch to an alternative tick source, uncomment this block:
// #undef portUSE_WDTO
// #define portUSE_TIMER0
// #define portTICK_PERIOD_MS 16

/*
 * When a tick source other than WDT is used, configuring the tick source becomes the user's responsibility.
 * E.g., when using Timer0 for the tick source, you can use the following snippet:
 *
 *      // Formula for the frequency is:
 *      //      f = F_CPU / (PRESCALER * (1 + COUNTER_TOP)
 *      //
 *      // Assuming the MCU clock of 16MHz, prescaler 1024 and counter top 249, the resulting tick period is 16 ms (62.5 Hz).
 *      //
 *      #define TICK_PERIOD_16MS     249
 *      #define PRESCALER            1024
 *      #if (portTICK_PERIOD_MS != (PRESCALER * (1 + TICK_PERIOD_16MS) * 1000 / F_CPU))
 *          #warning portTICK_PERIOD_MS defined in FreeRTOSVariant.h differs from your timer configuration
 *      #endif
 *
 *      // For register TCCR0A:
 *      #define NO_PWM              (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0)
 *      #define MODE_CTC_TCCR0A     (1 << WGM01) | (0 << WGM00)
 *
 *      // For register TCCR0B:
 *      #define MODE_CTC_TCCR0B     (0 << WGM02)
 *      #define PRESCALER_1024      (1 << CS02) | (0 << CS01) | (1 << CS00)
 *
 *      // For register TIMSK0:
 *      #define INTERRUPT_AT_TOP    (1 << OCIE0A)
 *
 *      extern "C"
 *      void prvSetupTimerInterrupt( void )
 *      {
 *          // In case Arduino platform has pre-configured the timer,
 *          // disable it before re-configuring here to avoid unpredicted results:
 *          TIMSK0 = 0;
 *
 *          // Now configure the timer:
 *          TCCR0A = NO_PWM | MODE_CTC_TCCR0A;
 *          TCCR0B = MODE_CTC_TCCR0B | PRESCALER_1024;
 *          OCR0A = TICK_PERIOD_16MS;
 *
 *          // Prevent missing the top and going into a possibly long wait until wrapping around:
 *          TCNT0 = 0;
 *
 *          // At this point the global interrupt flag is NOT YET enabled,
 *          // so you're NOT starting to get the ISR calls until FreeRTOS enables it just before launching the scheduler.
 *          TIMSK0 = INTERRUPT_AT_TOP;
 *      }
 */
void prvSetupTimerInterrupt( void );

/*
 * When a tick source other than WDT is used, calling the scheduler becomes the user's responsibility.
 * E.g., when using Timer0 for the tick source, you'll have a timer compare IRS wherein you call portSCHEDULER_ISR().
 *
 * Implement your ISR handler efficiently:
 *
 * For PREEMPTIVE scheduler use a `naked` attribute to reduce the call overhead:
 *
 *      ISR(portSCHEDULER_ISR, ISR_NAKED) __attribute__ ((hot, flatten)) {
 *          portSchedulerTick();
 *          __asm__ __volatile__ ( "reti" );
 *      }
 *
 *      The context is saved at the start of `portSchedulerTick()`, then the tick count is incremented, finally
 *      the new context is loaded - so no dirtying occurs.
 *
 *
 * For COOPERATIVE scheduler, the context is not saved because no switching is intended; therefore `naked` attribute
 *      cannot be applied because cooperative `portSchedulerTick()` dirties the context.
 *
 *      ISR(portSCHEDULER_ISR) __attribute__ ((hot, flatten)) {
 *          portSCHEDULER_ISR();
 *      }
 *
 *      Use ISR_NOBLOCK where there is an important timer running, that should preempt the scheduler:
 *
 *      ISR(portSCHEDULER_ISR, ISR_NAKED ISR_NOBLOCK) __attribute__ ((hot, flatten));
 *
 * Attributes `hot` and `flatten` help inlining all the code found inside your ISR thus reducing the call overhead.
 */
void portSchedulerTick( void ) __attribute__ ((hot, flatten));

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
