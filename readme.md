This is a fork of Richard Barry's FreeRTOS, optimised for the Arduino Microchip ATmega devices.

It has been created to provide access to FreeRTOS capabilities, with full compatibility to the Arduino IDE environment.
It does this by keeping hands off almost everything, and only touching the minimum of hardware to be successful.

If you want to use FreeRTOS on the Renesas family of Arduino like the Arduino UNO R4, it is [already included](https://github.com/arduino/ArduinoCore-renesas/tree/main/libraries/Arduino_FreeRTOS) in the default Arduino IDE. All that is required is to include the header file `Arduino_FreeRTOS.h` provided by the Arduino IDE, and follow the information noted below.

## Usage & Further Reading

Read the short blog post on [Arduino FreeRTOS](https://feilipu.me/2015/11/24/arduino_freertos/) to get started. And there is another much older post on using [FreeRTOS with AVR](https://feilipu.me/2011/09/22/freertos-and-libraries-for-avr-atmega/), which may be useful to read too. There are some further posts I've written on [Hackster.IO](https://www.hackster.io/feilipu), but they're essentially the same content.

The canonical source for information is the [FreeRTOS Web Site](https://www.freertos.org/). Within this site, the [Getting Started](https://www.freertos.org/FreeRTOS-quick-start-guide.html) page is very useful. This is the source for FreeRTOS usage (as distinct from installing and using this Arduino Library).

My other [AVRfreeRTOS Sourceforge Repository](https://sourceforge.net/projects/avrfreertos/) or [AVRfreeRTOS Github](https://github.com/feilipu/avrfreertos) has plenty of examples, ranging from [blink](https://sourceforge.net/projects/avrfreertos/files/MegaBlink/) through to a [synthesiser](https://sourceforge.net/projects/avrfreertos/files/GA_Synth/).

This library was the genesis of [generalised support for the ATmega platform within FreeRTOS](https://github.com/FreeRTOS/FreeRTOS-Kernel/pull/48), and improvement of the [stack depth type management](https://github.com/FreeRTOS/FreeRTOS-Kernel/pull/942).

Over the past few years FreeRTOS development has become increasingly 32-bit orientated, now including symmetric multiprocessing, with little change or improvement for the 8-bit world. As such I'm treating this FreeRTOS V11.1.0 (updated April 22 2024) as my LTS release.

## General

FreeRTOS has a multitude of configuration options, which can be specified from within the FreeRTOSConfig.h file.
To keep commonality with all of the Arduino hardware options, some sensible defaults have been selected. Feel free to change these defaults as you gain experience with FreeRTOS.

Normally, the ATmega Watchdog Timer is used to generate 15ms time slices (Ticks). For applications requiring high precision timing, the Ticks can be sourced from a hardware timer or external clock. See chapter [Scheduler Tick Sources](./doc/tick_sources.md) for the configuration details.

Tasks that suspend or delay before their allocated time slice completes will revert execution back to the Scheduler.

The Arduino `delay()` function has been redefined to automatically use the FreeRTOS `vTaskDelay()` function when the delay required is one Tick or longer, by setting `configUSE_PORT_DELAY` to `1`, so that simple Arduino example sketches and tutorials work as expected. If you would like to measure a short millisecond delay of less than one Tick, then preferably use [`millis()`](https://www.arduino.cc/reference/en/language/functions/time/millis/) (or with greater granularity use [`micros()`](https://www.arduino.cc/reference/en/language/functions/time/micros/)) to achieve this outcome (for example see [BlinkWithoutDelay](https://docs.arduino.cc/built-in-examples/digital/BlinkWithoutDelay)). However, when the delay requested is less than one Tick then the original Arduino `delay()` function will be automatically selected.

The 8-bit ATmega Timer0 has been added as an option for the experienced user. Please examine the Timer0 source code example to figure out how to use it. Reconfiguring Timer0 for the FreeRTOS Tick will break Arduino `millis()` and `micros()` though, as these functions rely on the Arduino IDE configuring Timer0. Example support for the Logic Green hardware using Timer 3 is provided via an open PR.

Stack for the `loop()` function has been set at 192 Bytes. This can be configured by adjusting the `configMINIMAL_STACK_SIZE` parameter. If you have stack overflow issues just increase it (within the SRAM limitations of your hardware). Users should prefer to allocate larger structures, arrays, or buffers on the heap using `pvPortMalloc()`, rather than defining them locally on the stack. Ideally you should __not__ use `loop()` for your sketches, and then the Idle Task stack size can be reduced down to 92 Bytes which will save some valuable memory.

Memory for the heap is allocated by the normal C `malloc()` function, wrapped by the FreeRTOS `pvPortMalloc()` function. This option has been selected because it is automatically adjusted to use the capabilities of each device. Other heap allocation schemes are supported by FreeRTOS, and they can used with some additional configuration.

If you do not need to use FreeRTOS Timer API functions, then they can be disabled. This will remove the need for the Timer Task Stack, saving 92 Bytes of RAM.

## Upgrading

* [Upgrading to FreeRTOS-9](https://www.freertos.org/FreeRTOS-V9.html)
* [Upgrading to FreeRTOS-10](https://www.freertos.org/FreeRTOS-V10.html)
* [Symmetric Multiprocessing with FreeRTOS-11](https://www.freertos.org/2023/12/introducing-freertos-kernel-version-11-0-0-a-major-release-with-symmetric-multiprocessing-smp-support.html)

## Errors

* Stack Overflow: If any stack (for the `loop()` or) for any Task overflows, there will be a slow LED blink, with 4 second cycle.
* Heap Overflow: If any Task tries to allocate memory and that allocation fails, there will be a fast LED blink, with 100 millisecond cycle.

## Errata

Testing with the Software Serial library shows some incompatibilities at low baud rates (9600), due to the extended time this library disables the global interrupt. Use the hardware USARTs.

## Compatibility

  * ATmega328 @ 16MHz : Arduino UNO R3, Arduino Duemilanove, Arduino Diecimila, etc.
  * ATmega328 @ 16MHz : Adafruit Pro Trinket 5V, Adafruit Metro 328, Adafruit Metro Mini
  * ATmega328 @ 16MHz : Seeed Studio Stalker
  * ATmega328 @ 16MHz : Freetronics Eleven
  * ATmega328 @ 12MHz : Adafruit Pro Trinket 3V
  * ATmega32u4 @ 16MHz : Arduino Leonardo, Arduino Micro, Arduino Yun, Teensy 2.0
  * ATmega32u4 @ 8MHz : Adafruit Flora, Bluefruit Micro
  * ATmega1284p @ 16MHz: Sanguino, WickedDevice WildFire
  * ATmega1284p @ 24.576MHz : Seeed Studio Goldilocks Analogue
  * ATmega2560 @ 16MHz : Arduino Mega, Arduino ADK
  * ATmega2560 @ 16MHz : Seeed Studio ADK

The new megaAVR 0-Series devices (eg. ATmega4809) are not fully compatible with this library. Their Timer configuration is substantially different from previous devices, and forms part of a new __avr8x__ architecture. It may be a while until avr-libc is updated to include support for megaAVR devices, but when that happens further work will be added here.

The Arduino IDE supporting the Arduino UNO R4 already includes FreeRTOS as standard.

## Files & Configuration

* `Arduino_FreeRTOS.h` : Must always be `#include` first. It references other configuration files, and sets defaults where necessary.
* `FreeRTOSConfig.h` : Contains a multitude of API and environment configurations.
* `FreeRTOSVariant.h` : Contains the ATmega specific configurations for this port of FreeRTOS.
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
[![All Contributors](https://img.shields.io/badge/all_contributors-10-green.svg?style=flat-square)](#contributors-)
<!-- ALL-CONTRIBUTORS-BADGE:END -->

Thanks goes to these wonderful people ([emoji key](https://allcontributors.org/docs/en/emoji-key)):

<!-- ALL-CONTRIBUTORS-LIST:START - Do not remove or modify this section -->
<!-- prettier-ignore-start -->
<!-- markdownlint-disable -->
<table>
  <tr>
    <td align="center"><a href="https://feilipu.me/"><img src="https://avatars.githubusercontent.com/u/3955592" width="100px;" alt=""/><br /><sub><b>Phillip Stevens</b></sub></a><br /><a title="Maintenance">🚧</a><a title="Code">💻</a><a title="Reviewed Pull Requests">👀</a><a title=Documentation">📖</a></td>
    <td align="center"><a href="https://www.blackleg.es/"><img src="https://avatars.githubusercontent.com/u/4323228" width="100px;" alt=""/><br /><sub><b>Hector Espert</b></sub></a><br /><a title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/Floessie"><img src="https://avatars.githubusercontent.com/u/10133457" width="100px;" alt=""/><br /><sub><b>Floessie</b></sub></a><br /><a title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/Derekduke"><img src="https://avatars.githubusercontent.com/u/30068270" width="100px;" alt=""/><br /><sub><b>Derekduke</b></sub></a><br /><a title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/balaji"><img src="https://avatars.githubusercontent.com/u/29356302" width="100px;" alt=""/><br /><sub><b>Balaji.V</b></sub></a><br /><a title="Code">💻</a><a title=Documentation">📖</a></td>
  </tr>
  <tr>
    <td align="center"><a href="https://github.com/neboskreb"><img src="https://avatars.githubusercontent.com/u/35344069" width="100px;" alt=""/><br /><sub><b>John Y. Pazekha</b></sub></a><br /><a title="Code">💻</a><a title=Documentation">📖</a></td>
    <td align="center"><a href="https://github.com/gpb01"><img src="https://avatars.githubusercontent.com/u/4134059" width="100px;" alt=""/><br /><sub><b>Guglielmo Braguglia</b></sub></a><br /><a title="Code">💻</a><a title=Documentation">📖</a></td>
    <td align="center"><a href="https://github.com/ShortArrow"><img src="https://avatars.githubusercontent.com/u/16986253" width="100px;" alt=""/><br /><sub><b>ShortArrow</b></sub></a><br /><a title=Documentation">📖</a></td>
    <td align="center"><a href="https://github.com/altugbakan"><img src="https://avatars.githubusercontent.com/u/43248015" width="100px;" alt=""/><br /><sub><b>Altuğ Bakan</b></sub></a><br /><a title="Code">💻</a></td>
    <td align="center"><a href="https://github.com/ikatz-drizly"><img src="https://avatars.githubusercontent.com/u/87482555" width="100px;" alt=""/><br /><sub><b>Ian Katz</b></sub></a><br /><a title="Code">💻</a></td>
  </tr>
</table>

<!-- markdownlint-enable -->
<!-- prettier-ignore-end -->
<!-- ALL-CONTRIBUTORS-LIST:END -->

This project follows the [all-contributors](https://github.com/all-contributors/all-contributors) specification. Contributions of any kind welcome!
