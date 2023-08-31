/*
 * Code tu use the LGT8F328 16 bit Timer 3 as a scheduler tick source
 * in Arduino_FreeRTOS.
 *
 * gpb01 - Aug 2023
 *
 */

#include <lgtx8p.h>    // lgt8f328p spec
#include <FreeRTOSVariant.h>

/*
 * Formula for the frequency is:
 *      f = F_CPU / (PRESCALER * (1 + COUNTER_TOP)
 *
 */

#define PRESCALER               8UL

#if ( F_CPU == 32000000UL )
    // Assuming the MCU clock of 32MHz, Timer 3 is 16 bits, prescaler 8 and counter top 63998, the resulting tick period is 16 ms (62.5 Hz).
    #define TICK_PERIOD_16MS    63998UL
#elif ( F_CPU == 16000000UL )
    // Assuming the MCU clock of 16MHz, Timer 3 is 16 bits, prescaler 8 and counter top 31999, the resulting tick period is 16 ms (62.5 Hz).
    #define TICK_PERIOD_16MS    31999UL
#elif ( F_CPU == 8000000UL )
    // Assuming the MCU clock of  8MHz, Timer 3 is 16 bits, prescaler 8 and counter top 15999, the resulting tick period is 16 ms (62.5 Hz).
    #define TICK_PERIOD_16MS    15999UL
#else   
    #error "Unsupported MCU frequency."
#endif // F_CPU options

#if (portTICK_PERIOD_MS != (PRESCALER * (1 + TICK_PERIOD_16MS) * 1000 / F_CPU))
    #warning portTICK_PERIOD_MS defined in FreeRTOSVariant.h differs from your timer configuration
#endif

#define MODE_CTC          (1 << WGM32)
#define PRESCALER_8       (1 << CS31)
#define INTERRUPT_OCMA    (1 << OCIE3A)

#ifdef __cplusplus
extern "C" {
#endif

    void prvSetupTimerInterrupt( void )
    {
        cli();
        TCCR3A = 0;
        TCCR3B = 0;
        TCNT3  = 0;
        // Now configure the timer:
        OCR3A = TICK_PERIOD_16MS;
        // CTC
        TCCR3B |= MODE_CTC;
        // Prescaler 8
        TCCR3B |= PRESCALER_8;
        // Output Compare Match A Interrupt Enable
        TIMSK3 |= INTERRUPT_OCMA;
        // Prevent missing the top and going into a possibly long wait until wrapping around:
        TCNT3 = 0;
        // At this point the global interrupt flag is NOT YET enabled,
        // so you're NOT starting to get the ISR calls until FreeRTOS enables it just before launching the scheduler.
    }
	
	
	void vPortEndScheduler( void )
	{
	    cli();              /* disable interrupts */
	    TIMSK3 &= ~( INTERRUPT_OCMA );
	    sei();              /* enable interrupts  */
	}
	
        
#if configUSE_PREEMPTION == 1
    
    ISR(TIMER3_vect, ISR_NAKED) __attribute__ ((hot, flatten))
	{
        if (TIFR3 & (1 << OCF3A)) {
            TIFR3 = 1 << OCF3A;
            /* on OCR3A match */
            vPortYieldFromTick();
            __asm__ __volatile__ ( "reti" );
        }
    }

#else
    
    ISR(TIMER3_vect) __attribute__ ((hot, flatten))
	{
        if (TIFR3 & (1 << OCF3A)) {
            TIFR3 = 1 << OCF3A;
            /* on OCR3A match */
            xTaskIncrementTick();
        }
    }

#endif

#ifdef __cplusplus
}
#endif
