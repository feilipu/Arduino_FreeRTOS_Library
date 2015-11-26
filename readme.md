This is a fork of Richard Berry's freeRTOS, optimised for the Arduino AVR devices.

It has been created to provide access to FreeRTOS capabilities, with full compatibility to the Arduino environment.
It does this by keeping hands off almost everything, and only touching the minimum of hardware to be successful.

## Further Reading

The canonical source for information is the [FreeRTOS Web Site](http://www.freertos.org/ "FreeRTOS").
Within this site, the [Getting Started](http://www.freertos.org/FreeRTOS-quick-start-guide.html "Quick Start Guide") page is very useful.
It is worth having a view from a user, and [manicbug](https://maniacbug.wordpress.com/2012/01/31/freertos/) has some interesting examples.
My other [AVRfreeRTOS Repository](https://sourceforge.net/projects/avrfreertos/) has plenty of examples,
ranging from [blink](https://sourceforge.net/projects/avrfreertos/files/MegaBlink/) through to a [synthesiser](https://sourceforge.net/projects/avrfreertos/files/GA_Synth/).

## General

FreeRTOS has a multitude of configuration options, which can be specified from within the FreeRTOSConfig.h file.
To keep commonality with all of the Arduino hardware options, some sensible defaults have been selected.

The AVR Watchdog Timer is used with to generate 30ms time slices, but Tasks that finish before their allocated time will hand execution back to the Scheduler.
This does not affect the use of any of the normal Timer functions in Arduino.

Time slices can be selected from 15ms up to 500ms. Slower time slicing can allow the Arduino MCU to sleep for longer, without the complexity of a Tickless idle.

Watchdog period options:
* WDTO_15MS
* WDTO_30MS
* WDTO_60MS
* WDTO_120MS
* WDTO_250MS
* WDTO_500MS
							
Note that Timer resolution is affected by integer math division and the time slice selected. Trying to measure 100ms, using a 60ms time slice for example, won't work.

Stack for the loop() function has been set at 128 bytes. This can be configured by adjusting the configIDLE_STACK_SIZE parameter.
It should not be less than the configMINIMAL_STACK_SIZE. If you have stack overflow issues, just increase it.
Users should prefer to allocate larger structures, arrays, or buffers using pvPortMalloc(), rather than defining them locally on the stack.

Memory for the heap is allocated by the normal malloc() function, wrapped by pvPortMalloc().
This option has been selected because it is automatically adjusted to use the capabilities of each device.
Other heap allocation schemes are supported by FreeRTOS, and they can used with additional configuration.

## Errors

* Stack Overflow: If any stack (for the loop() or) for any Task overflows, there will be a slow LED blink, with 4 second cycle.
* Heap Overflow: If any Task tries to allocate memory and that allocation fails, there will be a fast LED blink, with 100 millisecond cycle.

## Compatibility

  * ATmega328 @ 16MHz : Arduino UNO, Arduino Duemilanove, Arduino Diecimila, etc.
  * ATmega328 @ 16MHz : Adafruit Pro Trinket 5V, Adafruit Metro 328, Adafruit Metro Mini
  * ATmega328 @ 16MHz : Seeed Studio Stalker
  * ATmega328 @ 16MHz : Freetronics Eleven
  * ATmega328 @ 12MHz : Adafruit Pro Trinket 3V
  * ATmega32u4 @ 16MHz : Arduino Leonardo, Arduino Micro, Arduino Yun, Teensy 2.0
  * ATmega32u4 @ 8MHz : Adafruit Flora, Bluefruit Micro
  * ATmega1284p @ 24.576MHz : Seeed Studio Goldilocks, Seeed Studio Goldilocks Analogue
  * ATmega2560 @ 16MHz : Arduino Mega, Arduino ADK
  * ATmega2560 @ 16MHz : Seeed Studio ADK
  * ATmegaXXXX @ XXMHz : Anything with an ATmega MCU, really.

## Files & Configuration

* Arduino_FreeRTOS.h : Must always be #include first. It references other configuration files, and sets defaults where necessary.
* FreeRTOSConfig.h : Contains a multitude of API and environment configurations.
* FreeRTOSVariant.h : Contains the AVR specific configurations for this port of freeRTOS.
* heap_3.c : Contains the heap allocation scheme based on malloc(). Other schemes are available, but depend on user configuration for specific MCU choice.


