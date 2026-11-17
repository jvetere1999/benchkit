// Benchkit: one small class per hobby-electronics part, pins passed in at begin(), no delay().
//
//   #include <Benchkit.h>
//
//   Button btn;
//   Led led;
//   void setup() { btn.begin(2); led.begin(13); }
//   void loop()  { if (btn.read() == BUTTON_TAP) led.flash(150); led.update(); }
//
// Including everything costs nothing extra: only what the sketch uses reaches the board.
// With PlatformIO you can also include single parts: #include <benchkit/button.h>.
// Every header in src/benchkit/ starts with an example and the wiring.
#pragma once

#define BENCHKIT_VERSION "1.1.0"

#include "benchkit/button.h"
#include "benchkit/joystick.h"
#include "benchkit/pulse.h"
#include "benchkit/led.h"
#include "benchkit/sound.h"
#include "benchkit/sensors.h"
#include "benchkit/ultrasonic.h"
#include "benchkit/dht11.h"
#include "benchkit/motor.h"
#include "benchkit/stepper.h"
#include "benchkit/relay.h"
#include "benchkit/shift595.h"
#include "benchkit/seg7.h"
#include "benchkit/gfx.h"
#include "benchkit/ssd1306.h"
#include "benchkit/lcd1602.h"
#include "benchkit/screenlog.h"
#include "benchkit/servo_t1.h"
#include "benchkit/pixelring.h"   // PixelRing needs adafruit/Adafruit NeoPixel; the ring maths don't
#include "benchkit/nrf24.h"       // Nrf24 needs nrf24/RF24; AckHistory doesn't

#ifndef NATIVE_TEST
#include "benchkit/stamp.h"
#endif

// The colour TFT drives the panel through Adafruit's ST7735 library (installed with Benchkit).
#if __has_include(<Adafruit_ST7735.h>)
#include "benchkit/st7735.h"
#endif
