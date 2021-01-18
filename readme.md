This is a fork of Richard Barry's freeRTOS, optimised for the Arduino AVR devices.

It has been created to provide access to FreeRTOS capabilities, with full compatibility to the Arduino environment.
It does this by keeping hands off almost everything, and only touching the minimum of hardware to be successful.

[![Arduino_CI](https://github.com/feilipu/Arduino_FreeRTOS_Library/workflows/Arduino_CI/badge.svg)](https://github.com/marketplace/actions/arduino_ci)

## Usage & Further Reading

Read the short blog post on [Arduino FreeRTOS](https://feilipu.me/2015/11/24/arduino_freertos/) to get started. And there is another much older post on using [FreeRTOS with AVR](https://feilipu.me/2011/09/22/freertos-and-libraries-for-avr-atmega/), which may be useful to read too. There are some further posts I've written on [Hackster.IO](https://www.hackster.io/feilipu), but they're essentially the same content.

The canonical source for information is the [FreeRTOS Web Site](https://www.freertos.org/). Within this site, the [Getting Started](https://www.freertos.org/FreeRTOS-quick-start-guide.html) page is very useful. This is the source for FreeRTOS usage (as distinct from installing and using this Arduino Library).

My other [AVRfreeRTOS Sourceforge Repository](https://sourceforge.net/projects/avrfreertos/) or [AVRfreeRTOS Github](https://github.com/feilipu/avrfreertos) has plenty of examples, ranging from [blink](https://sourceforge.net/projects/avrfreertos/files/MegaBlink/) through to a [synthesiser](https://sourceforge.net/projects/avrfreertos/files/GA_Synth/).

This library was the genesis of [generalised support for the ATmega platform within FreeRTOS](https://github.com/FreeRTOS/FreeRTOS-Kernel/pull/48).

## General

FreeRTOS has a multitude of configuration options, which can be specified from within the FreeRTOSConfig.h file.
To keep commonality with all of the Arduino hardware options, some sensible defaults have been selected.

The AVR Watchdog Timer is used to generate 15ms time slices, but Tasks that finish before their allocated time will hand execution back to the Scheduler. This does not affect the use of any of the normal Timer functions in Arduino.

Time slices can be selected from 15ms up to 500ms. Slower time slicing can allow the Arduino MCU to sleep for longer, without the complexity of a Tickless idle.

Watchdog period options:
* `WDTO_15MS`
* `WDTO_30MS`
* `WDTO_60MS`
* `WDTO_120MS`
* `WDTO_250MS`
* `WDTO_500MS`
* `WDTO_1S`
* `WDTO_2S`

Note that Timer resolution is affected by integer math division and the time slice selected. Trying to measure 50ms, using a 120ms time slice for example, won't work.

Stack for the `loop()` function has been set at 192 bytes. This can be configured by adjusting the `configMINIMAL_STACK_SIZE` parameter. If you have stack overflow issues, just increase it.
Users should prefer to allocate larger structures, arrays, or buffers using `pvPortMalloc()`, rather than defining them locally on the stack.

Memory for the heap is allocated by the normal `malloc()` function, wrapped by `pvPortMalloc()`.
This option has been selected because it is automatically adjusted to use the capabilities of each device.
Other heap allocation schemes are supported by FreeRTOS, and they can used with additional configuration.

## Upgrading

* [Upgrading to FreeRTOS-9](https://www.freertos.org/FreeRTOS-V9.html)
* [Upgrading to FreeRTOS-10](https://www.freertos.org/FreeRTOS-V10.html)

## Errors

* Stack Overflow: If any stack (for the `loop()` or) for any Task overflows, there will be a slow LED blink, with 4 second cycle.
* Heap Overflow: If any Task tries to allocate memory and that allocation fails, there will be a fast LED blink, with 100 millisecond cycle.

## Errata

Testing with the Software Serial library shows some incompatibilities at low baud rates (9600), due to the extended time this library disables the global interrupt. Use the hardware USARTs.

## Compatibility

  * ATmega328 @ 16MHz : Arduino UNO, Arduino Duemilanove, Arduino Diecimila, etc.
  * ATmega328 @ 16MHz : Adafruit Pro Trinket 5V, Adafruit Metro 328, Adafruit Metro Mini
  * ATmega328 @ 16MHz : Seeed Studio Stalker
  * ATmega328 @ 16MHz : Freetronics Eleven
  * ATmega328 @ 12MHz : Adafruit Pro Trinket 3V
  * ATmega32u4 @ 16MHz : Arduino Leonardo, Arduino Micro, Arduino Yun, Teensy 2.0
  * ATmega32u4 @ 8MHz : Adafruit Flora, Bluefruit Micro
  * ATmega1284p @ 16MHz: Sanguino, WickedDevice WildFire
  * ATmega1284p @ 24.576MHz : Seeed Studio Goldilocks, Seeed Studio Goldilocks Analogue
  * ATmega2560 @ 16MHz : Arduino Mega, Arduino ADK
  * ATmega2560 @ 16MHz : Seeed Studio ADK
  
The new megaAVR 0-Series devices (eg. ATmega4809) are not fully compatible with this library. Their Timer configuration is substantially different from previous devices, and forms part of a new __avr8x__ architecture. It may be a while until avr-libc is updated to include support for megaAVR devices, but when that happens further work will be added here.

## Files & Configuration

* `Arduino_FreeRTOS.h` : Must always be `#include` first. It references other configuration files, and sets defaults where necessary.
* `FreeRTOSConfig.h` : Contains a multitude of API and environment configurations.
* `FreeRTOSVariant.h` : Contains the AVR specific configurations for this port of freeRTOS.
* `heap_3.c` : Contains the heap allocation scheme based on `malloc()`. Other schemes are available, but depend on user configuration for specific MCU choice.

### PlatformIO

[Arduino FreeRTOS](https://platformio.org/lib/show/507/FreeRTOS) is available in the [PlatformIO library manager](https://docs.platformio.org/en/latest/librarymanager/index.html) for use in a [PlatformIO project](https://docs.platformio.org/en/latest/projectconf/index.html).

Watchdog period is configurable using build-flags:

```python
build_flags =
  -DportUSE_WDTO=WDTO_15MS
```

### Code of conduct

See the [Code of conduct](https://github.com/feilipu/Arduino_FreeRTOS_Library/blob/master/CODE_OF_CONDUCT.md).
## Contributors ✨

<!-- ALL-CONTRIBUTORS-BADGE:START - Do not remove or modify this section -->
[![All Contributors](https://img.shields.io/badge/all_contributors-5-orange.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->

Thanks goes to these wonderful people ([emoji key](https://allcontributors.org/docs/en/emoji-key)):

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tr>
    <td align="center"><a href="https://feilipu.me/"><img src="https://avatars2.githubusercontent.com/u/3955592" width="100px;" alt=""/><br /><sub><b>Phillip Stevens</b></sub></a><br /><a title="Maintenance">🚧</a><a title="Code">💻</a><a title="Reviewed Pull Requests">👀</a></td>
    <td align="center"><a href="https://www.blackleg.es/"><img src="https://avatars1.githubusercontent.com/u/4323228" width="100px;" alt=""/><br /><sub><b>Hector Espert</b></sub></a><br /><a title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/Floessie"><img src="https://avatars1.githubusercontent.com/u/10133457" width="100px;" alt=""/><br /><sub><b>Floessie</b></sub></a><br /><a title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/Derekduke"><img src="https://avatars2.githubusercontent.com/u/30068270" width="100px;" alt=""/><br /><sub><b>Derekduke</b></sub></a><br /><a title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/balaji"><img src="https://avatars2.githubusercontent.com/u/29356302" width="100px;" alt=""/><br /><sub><b>Balaji.V</b></sub></a><br /><a title="Code">💻</a></td>
  </tr>
</table>

<!-- markdownlint-enable -->
<!-- prettier-ignore-end -->
<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome!
