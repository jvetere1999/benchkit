#include "led.h"
#include <Arduino.h>

void Led::begin(uint8_t p, bool low) {
  pin = p;
  activeLow = low;
  pinMode(pin, OUTPUT);
  pulse.off();
  lit = true;    // force the first write
  apply();
}

void Led::apply() {
  bool want = pulse.isOn();
  if (want == lit) return;
  lit = want;
  digitalWrite(pin, want != activeLow ? HIGH : LOW);
}

void Led::flash(uint16_t ms) { pulse.flash(ms, millis()); lit = !pulse.isOn(); apply(); }
void Led::blink(uint16_t onMs, uint16_t offMs, uint8_t times) {
  pulse.blink(onMs, offMs, times, millis());
  lit = !pulse.isOn();
  apply();
}
void Led::update() { if (pulse.busy()) { pulse.update(millis()); apply(); } }

void Led::brightness(uint8_t level) {
  pulse.on();
  uint8_t v = gamma8(level);
  analogWrite(pin, activeLow ? 255 - v : v);
  lit = level > 0;
  if (!lit) pulse.off();
}

void RgbLed::begin(uint8_t r, uint8_t g, uint8_t b, Kind k) {
  pins[0] = r; pins[1] = g; pins[2] = b;
  kind = k;
  for (uint8_t i = 0; i < 3; i++) pinMode(pins[i], OUTPUT);
  off();
}

void RgbLed::set(uint8_t r, uint8_t g, uint8_t b) {
  const uint8_t v[3] = {r, g, b};
  for (uint8_t i = 0; i < 3; i++) {
    uint8_t out = gamma8(v[i]);
    analogWrite(pins[i], kind == ANODE ? 255 - out : out);
  }
}

uint32_t hueRgb(uint8_t h) {
  uint8_t seg = h / 85, f = (h - seg * 85) * 3, q = 255 - f;
  if (seg == 0) return ((uint32_t)q << 16) | ((uint32_t)f << 8);
  if (seg == 1) return ((uint32_t)q << 8) | f;
  return ((uint32_t)f << 16) | q;
}

void RgbLed::hue(uint8_t h, uint8_t level) {
  uint32_t c = hueRgb(h);
  set(((c >> 16) & 255) * level / 255, ((c >> 8) & 255) * level / 255, (c & 255) * level / 255);
}
