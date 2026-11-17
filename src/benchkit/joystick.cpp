#include "joystick.h"
#ifdef ARDUINO
#include <Arduino.h>

void Joystick::begin(uint8_t px, uint8_t py) {
  pinX = px;
  pinY = py;
  long sx = 0, sy = 0;
  for (uint8_t i = 0; i < 16; i++) {
    sx += analogRead(pinX);
    sy += analogRead(pinY);
    delay(2);
  }
  centerX = sx / 16;
  centerY = sy / 16;
  rawX = centerX;
  rawY = centerY;
}

void Joystick::read() {
  update(analogRead(pinX), analogRead(pinY));
}
#endif

int8_t stickPercent(int16_t raw, int16_t center, uint8_t deadzone, bool invert) {
  int16_t v = raw - center;
  if ((v < 0 ? -v : v) < deadzone) return 0;
  int16_t span = v > 0 ? 1023 - center : center;
  if (span < 1) span = 1;
  int32_t p = (int32_t)v * 100 / span;
  if (p > 100) p = 100;
  if (p < -100) p = -100;
  return invert ? -p : p;
}

void Joystick::update(int16_t rx, int16_t ry) {
  rawX = rx;
  rawY = ry;
  x = stickPercent(rx, centerX, deadzone, invertX);
  y = stickPercent(ry, centerY, deadzone, invertY);
}

StickDir StickPresses::update(int8_t x, int8_t y) {
  int16_t ax = x < 0 ? -x : x, ay = y < 0 ? -y : y;
  if (current != STICK_NONE) {
    // released once the axis it was pushed along comes back under `off`
    int16_t along = (current == STICK_LEFT || current == STICK_RIGHT) ? ax : ay;
    if (along >= off) return STICK_NONE;
    current = STICK_NONE;
  }
  if (ax < on && ay < on) return STICK_NONE;
  current = ax >= ay ? (x > 0 ? STICK_RIGHT : STICK_LEFT) : (y > 0 ? STICK_UP : STICK_DOWN);
  return current;
}
