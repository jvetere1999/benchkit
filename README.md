# Benchkit

One small class per hobby-electronics part, for Arduino. Pins are passed in at `begin()`, nothing
uses `delay()`, and only what your sketch uses ends up on the board.

```cpp
#include <Benchkit.h>

Button btn;
Led led;

void setup() {
  btn.begin(2);          // button to 5V, 10 kΩ pin to GND: pressed = HIGH
  led.begin(13);
}

void loop() {
  if (btn.read() == BUTTON_TAP) led.flash(150);
  led.update();
}
```

> **Status:** every class is tested on a computer against a pretend Arduino, and every example
> compiles for an Uno, but most parts haven't been tried on real hardware yet. Issues welcome.

## Install

**PlatformIO** (`platformio.ini`):

```ini
lib_deps = https://github.com/jvetere1999/benchkit.git#v1.0.0
```

**Arduino IDE:** download the ZIP from the [releases](https://github.com/jvetere1999/benchkit/releases)
(or *Code → Download ZIP*), then *Sketch → Include Library → Add .ZIP Library*. Install
*Adafruit ST7735 and ST7789 Library* too (it brings Adafruit GFX and BusIO); only `St7735` uses
it, and without it everything else still works.

## Parts

| Header | Classes | Parts |
|---|---|---|
| `button` | `Button`, `DoubleTap`, `Repeat`, `PressCount` | push buttons, a joystick's click |
| `joystick` | `Joystick`, `StickPresses` | 2-axis joystick module (KY-023) |
| `st7735` | `St7735` | 1.8" colour TFT (software SPI, any pins, fast) |
| `ssd1306` | `Ssd1306`, `Ssd1306x32` | 128x64 / 128x32 OLED (I2C); only sends what changed |
| `lcd1602` | `Lcd1602` | 16x2 character LCD (HD44780, 4-bit); only sends what changed |
| `gfx` | `Gfx<…>`, `color565`, `hue565`, `dim565` | drawing shared by `St7735` and `Ssd1306` |
| `led` | `Led`, `RgbLed` | LEDs, RGB LED (cathode or anode) |
| `sound` | `Buzzer`, `ActiveBuzzer`, `Note` | passive / piezo buzzer, 8 Ω speaker; active buzzer |
| `sensors` | `Pot`, `Ldr`, `Tmp36`, `Thermistor`, `SharpIr`, `TiltSwitch`, `VibrationSensor` | potentiometer, photoresistor, TMP36, 10k NTC, Sharp IR distance, ball tilt switch, SW-420 vibration module |
| `ultrasonic` | `HcSr04` | HC-SR04 |
| `dht11` | `Dht11` | DHT11 temperature + humidity |
| `motor` | `Motor` | DC motor on an L293D / SN754410 |
| `stepper` | `Stepper28BYJ` | 28BYJ-48 + ULN2003 board |
| `relay` | `Relay` | 5 V relay (on a transistor) or a relay module |
| `shift595` | `Shift595<chips>` | 74HC595, one or chained |
| `seg7` | `Seg7`, `Seg7x4`, `segmentsFor()` | 1-digit and 4-digit 7-segment displays |
| `pulse` | `Pulse` | on/off timing used by `led`, `sound` |
| `stamp` | `stamp()` | `[m:ss.mmm]` at the start of serial lines |

Each header in [`src/benchkit/`](src/benchkit) starts with an example and the wiring, and
[`examples/`](examples) has a sketch per part. With PlatformIO you can include one part on its
own: `#include <benchkit/button.h>`.

Things that take time (flash, blink, tunes, ramps, steps, multiplexing) have an `update()` to
call every loop. Two exceptions, said in their headers: `HcSr04` waits for its echo (~12 ms) and
`Dht11` takes ~25 ms once a second.

For a NeoPixel ring, IR remote or nRF24 radio, use Adafruit_NeoPixel, IRremote and RF24.

## Costs

Unused functions are dropped when linking, nothing is `virtual` (the displays share drawing
through a template), and extras such as `DoubleTap` are separate objects you only pay for if you
make one. RAM is per object: an `Ssd1306` is a 1 KB screen buffer (`Ssd1306x32`: 512 B), a
`Shift595<2>` 4 bytes, most others under 20 bytes. Fonts, tunes and sprites stay in flash.

## Developing

```sh
cd extras/dev
pio test -e native     # 54 tests on the computer, against a fake Arduino (extras/dev/fakelib)
pio run                # a demo, and every class built for an Uno and a Mega
```

CI runs both and compiles every example. To release: bump the version in `library.json`,
`library.properties` and `src/Benchkit.h` (CI checks they agree), then tag `vX.Y.Z`.

## Licence

MIT ([LICENSE](LICENSE)). The 5x7 font is Adafruit's, under BSD ([LICENSE-font.txt](LICENSE-font.txt)).
