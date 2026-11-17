// A single LED (through a 220-330 Ω resistor), and an RGB LED.
//
//   Led led;  led.begin(LED_PIN);
//   led.flash(150);            // 150 ms, then off by itself
//   led.blink(200, 800);       // forever;  led.blink(100, 100, 3) = 3 blinks
//   led.update();              // every loop, so flash/blink can turn it off and on
//   led.brightness(64);        // dim (PWM pins only: Uno 3 5 6 9 10 11), steady
//
//   RgbLed rgb;  rgb.begin(R_PIN, G_PIN, B_PIN);   // PWM pins; common cathode (RgbLed::ANODE if the long leg goes to 5V)
//   rgb.set(255, 80, 0);  rgb.set(0xFF5000);  rgb.hue(170);
#pragma once
#include <stdint.h>
#include "pulse.h"

class Led {
public:
  void begin(uint8_t pin, bool activeLow = false);
  void on() { pulse.on(); apply(); }
  void off() { pulse.off(); apply(); }
  void set(bool lit) { lit ? on() : off(); }
  void toggle() { set(!pulse.isOn()); }
  void flash(uint16_t ms);
  void blink(uint16_t onMs, uint16_t offMs, uint8_t times = 0);   // 0 = forever
  void update();                 // every loop (only does something while flashing / blinking)
  bool isOn() const { return pulse.isOn(); }
  bool busy() const { return pulse.busy(); }
  // 0..255, gamma-corrected so 128 looks about half as bright. Stops blinking.
  void brightness(uint8_t level);

private:
  Pulse pulse;
  uint8_t pin = 255;
  bool activeLow = false, lit = false;
  void apply();
};

// Perceived-brightness correction: 0..255 in, 0..255 PWM out (squares it)
inline uint8_t gamma8(uint8_t v) { return ((uint16_t)v * v + 255) >> 8; }

class RgbLed {
public:
  enum Kind : uint8_t { CATHODE, ANODE };   // common cathode: long leg to GND
  void begin(uint8_t r, uint8_t g, uint8_t b, Kind kind = CATHODE);
  void set(uint8_t r, uint8_t g, uint8_t b);      // 0..255 each (gamma-corrected)
  void set(uint32_t rgb) { set(rgb >> 16, rgb >> 8, rgb); }   // 0xRRGGBB
  void hue(uint8_t h, uint8_t level = 255);       // rainbow: 0 red, 85 green, 170 blue
  void off() { set(0, 0, 0); }

private:
  uint8_t pins[3] = {255, 255, 255};
  Kind kind = CATHODE;
};

// Rainbow colour as 0xRRGGBB: 0 red -> 85 green -> 170 blue -> 255 back to red
uint32_t hueRgb(uint8_t h);
