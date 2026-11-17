#include "stepper.h"
#include <Arduino.h>

uint8_t Stepper28BYJ::pattern(uint8_t phase) {
  static const uint8_t HALF[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};
  return HALF[phase & 7];
}

void Stepper28BYJ::begin(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  pins[0] = a; pins[1] = b; pins[2] = c; pins[3] = d;
  for (uint8_t i = 0; i < 4; i++) pinMode(pins[i], OUTPUT);
  release();
}

void Stepper28BYJ::rpm(uint8_t r) {
  if (r < 1) r = 1;
  if (r > 15) r = 15;   // faster and it skips steps (or just buzzes)
  intervalUs = 60000000UL / ((uint32_t)TURN * r);
}

void Stepper28BYJ::out(uint8_t bits) {
  for (uint8_t i = 0; i < 4; i++) digitalWrite(pins[i], (bits >> i) & 1);
}

void Stepper28BYJ::release() { out(0); energised = false; }

void Stepper28BYJ::update() {
  uint32_t now = micros();
  if (pos == target) {
    if (energised && !holdWhenStopped && now - lastUs >= intervalUs) release();
    return;
  }
  if (energised && now - lastUs < intervalUs) return;
  if (energised) pos += target > pos ? 1 : -1;   // (the first call only energises the current phase)
  out(pattern((uint8_t)(pos & 7)));
  energised = true;
  lastUs = now;
}
