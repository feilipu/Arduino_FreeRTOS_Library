# Scheduler tick sources

## Configuration
Tick source is selected by (un)defining values `portUSE_WDTO` and `portUSE_TIMER0` in file `FreeRTOSVariant.h`. Default in Arduino_FreeRTOS is Watchdog timer (WDT), it contains all code needed for this and works out-of-the-box.

For alternative tick source, pieces of code must be provided by the application. Arduino_FreeRTOS expects you to provide function `void prvSetupTimerInterrupt(void)` responsible for the initialisation of your tick source. This function is called after the Arduino's initialisation and before the FreeRTOS scheduler is launched.

NOTE: Reconfiguring Timer0 for FreeRTOS will break Arduino `millis()` and `micros()`, as these functions rely on Timer0. Functions relying on these Arduino features need to be overridden.

## WDT (default)
Time slices can be selected from 15ms up to 500ms. Slower time slicing can allow the Arduino MCU to sleep for longer, without the complexity of a Tickless idle.

Watchdog period options:
* `WDTO_15MS` (default)
* `WDTO_30MS`
* `WDTO_60MS`
* `WDTO_120MS`
* `WDTO_250MS`
* `WDTO_500MS`
* `WDTO_1S`
* `WDTO_2S`

### WDT precision limitations
The frequency of the Watchdog Oscillator is voltage and temperature dependent as shown in “Typical Characteristics” on corresponding figures:

![WDT limitations](https://user-images.githubusercontent.com/35344069/224619444-3c0b634c-f460-40d2-8a73-256bad0d5ba1.png)

Timing consistency may vary as much as 20% between two devices in same setup due to individual device differences, or between a prototype and production device due to setup differences.

## Alternative tick sources
For applications requiring high precision timing, the Ticks can be sourced from one of the hardware timers or an external clock input.

First, you switch it in `FreeRTOSVariant.h` header by removing or undefining `portUSE_WDTO` and defining, here for example, the 8-bit Timer0 `portUSE_TIMER0`.

```cpp
#undef portUSE_WDTO
#define portUSE_TIMER0
```

Next, in your app you provide two pieces of code: the initialisation functions and the ISR hook. Their implementation depends on your tick source.

## Timer specific initialisation and ISR functions

For implementation examples for many different timers, including the RTC Timer2 available on some devices, please refer to `port.c` in the [AVRfreeRTOS Repository](https://github.com/feilipu/avrfreertos/tree/master/freeRTOS10xx/portable).

### Hardware timer Timer0
_NOTE: This code snippet is verified to work on Atmega2560. Full code available [here](./tick_sources_timer0.cpp)._

```cpp
// Formula for the frequency is:
//      f = F_CPU / (PRESCALER * (1 + COUNTER_TOP)
//
// Assuming the MCU clock of 16MHz, prescaler 1024 and counter top 249, the resulting tick period is 16 ms (62.5 Hz).
//
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
```

To switch to an alternative tick source, here for example Timer0, insert this block in `FreeRTOSVariant.h`:

```cpp
#undef portUSE_WDTO
#define portUSE_TIMER0
#define portTICK_PERIOD_MS 16

/*
 * When a tick source other than WDT is used, configuring the tick source becomes the user's responsibility.
 * E.g., when using Timer0 for the tick source, you can use the following snippet:
 */

// Formula for the frequency is:
//      f = F_CPU / (PRESCALER * (1 + COUNTER_TOP)
//
// Assuming the MCU clock of 16MHz, prescaler 1024 and counter top 249, the resulting tick period is 16 ms (62.5 Hz).
//

#define TICK_PERIOD_16MS     249
#define PRESCALER            1024

#if (portTICK_PERIOD_MS != (PRESCALER * (1 + TICK_PERIOD_16MS) * 1000 / F_CPU))
    #warning portTICK_PERIOD_MS defined in FreeRTOSVariant.h differs from your timer configuration
#endif
 *
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
```

Though Timer0 is given as example here, any timer can be used. A 16-bit timer (e.g., Timer1) is needed for time slices longer than ~20 milliseconds.

### Hardware timer ISR hook
For **preemptive** scheduler use `ISR_NAKED` attribute to reduce the call overhead:

```cpp
ISR(TIMER0_COMPA_vect, ISR_NAKED) __attribute__ ((hot, flatten));
ISR(TIMER0_COMPA_vect) {
    vPortYieldFromTick();
    __asm__ __volatile__ ( "reti" );
}
```

The register context is saved at the start of `vPortYieldFromTick()`, then the tick count is incremented, finally the new context is loaded - so no dirtying occurs.


For **cooperative** scheduler, the register context is not saved because no switching is intended; therefore `naked` attribute cannot be applied because cooperative `xTaskIncrementTick()` dirties the context.

```cpp
ISR(TIMER0_COMPA_vect) __attribute__ ((hot, flatten));
ISR(TIMER0_COMPA_vect) {
    xTaskIncrementTick();
}
```

Use `ISR_NOBLOCK` where there is an important timer running, that should preempt the scheduler:
```cpp
ISR(portSCHEDULER_ISR, ISR_NAKED ISR_NOBLOCK) __attribute__ ((hot, flatten));
```

The attributes `hot` and `flatten` help inlining all the code found inside your ISR thus reducing the call overhead.
Note: NO comma before `ISR_NOBLOCK`.


### External clock
_NOTE: This code snippet example has not been verified to work._

Assuming the external clock is connected to data pin 21 (external interrupt `INT0`):

```cpp
// For register EICRA:
#define TICK_ON_RISING_EDGE_D21   (1 << ISC01) | (1 << ISC00)

// For register EIMSK:
#define TICK_INPUT_PIN_D21     (1 << INT0)

extern "C"
void prvSetupTimerInterrupt( void )
{
    EICRA = TICK_ON_RISING_EDGE_D21;

    // At this point the global interrupt flag is NOT YET enabled,
    // so you're NOT starting to get the ISR calls until FreeRTOS enables it just before launching the scheduler.
    EIMSK = TICK_INPUT_PIN_D21;

    // Configure the pin
    pinMode(21, INPUT);
}
```


### External clock ISR hook
Similar to Timer0 ISR, for **preemptive** scheduler:

```cpp
ISR(INT0_vect, ISR_NAKED) __attribute__ ((hot, flatten));
ISR(INT0_vect) {
    vPortYieldFromTick();
    __asm__ __volatile__ ( "reti" );
}
```

For **cooperative** scheduler:
```cpp
ISR(INT0_vect) __attribute__ ((hot, flatten));
ISR(INT0_vect) {
    xTaskIncrementTick();
}
```

## Performance considerations
When selecting the duration for the time slice, the following should be kept in mind.

### Granularity
Note that Timer resolution (or granularity) is affected by integer maths division and the time slice selected. For example, trying to measure 50ms using a 120ms time slice won't work.

### Context switching
In preemptive mode, tasks which are actively executing (i.e., those not waiting for a semaphore or queue) might be switched every time tick, depending on their priority. Switching the context involves pushing all CPU's registers of old task and popping all registers of new task. The shorter your time slice is, the bigger of overhead this becomes.

In cooperative mode, context overhead is not a factor.

### Calculations
On MCUs lacking the hardware division operation like AVR, a special care should be taken to avoid division operations. Where unavoidable, operations with divisor of power of 2 work best because they are performed with bitwise shifting, whereas an arbitrary value results in a software division operation taking ~200 clock cycles (for a `uint16_t` operand).

You might encounter a division when calculating delays, e.g. converting milliseconds to ticks:

```cpp
   TickType_t ticks = delay_millis / portTICK_PERIOD_MS
```

If your application needs to do this sort of conversion a lot, consider making your time slice a power-of-2 value (16 ms, 32 ms, 64 ms etc.).

