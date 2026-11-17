#include "shift595.h"
#include <Arduino.h>

void shift595Send(uint8_t data, uint8_t clock, uint8_t latch, const uint8_t *bytes, uint8_t n) {
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(latch, OUTPUT);
  digitalWrite(latch, LOW);
  for (uint8_t i = n; i-- > 0;) shiftOut(data, clock, MSBFIRST, bytes[i]);   // the far chip first
  digitalWrite(latch, HIGH);   // outputs change together, here
}
