#include <Arduino.h>
#include <Wire.h>

namespace fake {
uint32_t us;
uint8_t level[64], mode[64];
int16_t analog[64], pwm[64];
uint32_t pulse;
std::string log;
void reset() {
  us = 0; pulse = 0; log.clear();
  memset(level, 0, sizeof(level)); memset(mode, 0, sizeof(mode));
  for (int i = 0; i < 64; i++) { analog[i] = 0; pwm[i] = -1; }
}
}

static void add(const char *fmt, int a, int b) {
  char buf[32];
  snprintf(buf, sizeof(buf), fmt, a, b);
  fake::log += buf;
}
void digitalWrite(uint8_t p, uint8_t v) { fake::level[p] = v ? HIGH : LOW; add("w%d=%d ", p, v ? 1 : 0); }
void analogWrite(uint8_t p, int v) { fake::pwm[p] = v; add("a%d=%d ", p, v); }
void tone(uint8_t p, unsigned int hz, unsigned long) { add("t%d=%d ", p, hz); }
void noTone(uint8_t p) { add("n%d%c", p, ' '); }
void shiftOut(uint8_t, uint8_t, uint8_t order, uint8_t value) { add("s%02x%c", order == MSBFIRST ? value : value, ' '); }

TwoWire Wire;
void TwoWire::beginTransmission(uint8_t a) { addr = a; char b[8]; snprintf(b, sizeof(b), "[%02x:", a); log += b; }
size_t TwoWire::write(uint8_t v) { char b[6]; snprintf(b, sizeof(b), " %02x", v); log += b; bytes++; return 1; }
uint8_t TwoWire::endTransmission(bool) { log += "] "; return addr == present ? 0 : 2; }
