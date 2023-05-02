#include <FreeRTOSVariant.h>

/*
 * Formula for the frequency is:
 *      f = F_CPU / (PRESCALER * (1 + COUNTER_TOP)
 *
 * Assuming the MCU clock of 16MHz, prescaler 1024 and counter top 249, the resulting tick period is 16 ms (62.5 Hz).
 */
#define TICK_PERIOD_16MS     249
#define PRESCALER            1024
#if (portTICK_PERIOD_MS != (PRESCALER * (1 + TICK_PERIOD_16MS) * 1000 / F_CPU))
    #warning portTICK_PERIOD_MS defined in FreeRTOSVariant.h differs from your timer configuration
#endif

// For register TCCR0A:
#define NO_PWM              (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0)
#define MODE_CTC_TCCR0A     (1 << WGM01) | (0 << WGM00)

// For register TCCR0B:
#define MODE_CTC_TCCR0B     (0 << WGM02)
#define PRESCALER_1024      (1 << CS02) | (0 << CS01) | (1 << CS00)

// For register TIMSK0:
#define INTERRUPT_AT_TOP    (1 << OCIE0A)

extern "C"
void prvSetupTimerInterrupt( void )
{
    // In case Arduino platform has pre-configured the timer,
    // disable it before re-configuring here to avoid unpredicted results:
    TIMSK0 = 0;

    // Now configure the timer:
    TCCR0A = NO_PWM | MODE_CTC_TCCR0A;
    TCCR0B = MODE_CTC_TCCR0B | PRESCALER_1024;
    OCR0A = TICK_PERIOD_16MS;

    // Prevent missing the top and going into a possibly long wait until wrapping around:
    TCNT0 = 0;

    // At this point the global interrupt flag is NOT YET enabled,
    // so you're NOT starting to get the ISR calls until FreeRTOS enables it just before launching the scheduler.
    TIMSK0 = INTERRUPT_AT_TOP;
}


ISR(TIMER0_COMPA_vect, ISR_NAKED) __attribute__ ((hot, flatten));
ISR(TIMER0_COMPA_vect)
{
    vPortYieldFromTick();
    __asm__ __volatile__ ( "reti" );
}
