#include "dht11.h"
#include <Arduino.h>

bool Dht11::decode(const uint8_t b[5], int8_t &tempC, uint8_t &humidity) {
  if ((uint8_t)(b[0] + b[1] + b[2] + b[3]) != b[4]) return false;
  humidity = b[0];
  tempC = (b[3] & 0x80) ? -(int8_t)b[2] : (int8_t)b[2];   // newer DHT11s can show below 0 this way
  return true;
}

void Dht11::begin(uint8_t p) {
  pin = p;
  pinMode(pin, INPUT_PULLUP);
  lastMs = millis();             // the chip needs ~1 s after power-up before its first reading
  started = false;
}

// Wait while the pin is at `level`; returns how long it took (µs), or 0 on timeout
static uint16_t waitWhile(uint8_t pin, uint8_t level, uint16_t maxUs) {
  uint32_t t0 = micros();
  while (digitalRead(pin) == level) {
    if (micros() - t0 > maxUs) return 0;
  }
  return micros() - t0 + 1;
}

Dht11::Status Dht11::transfer(uint8_t b[5]) {
  // Start: pull low 20 ms, let go; the chip answers 80 µs low, 80 µs high, then 40 bits:
  // each 50 µs low, then high for ~27 µs (a 0) or ~70 µs (a 1).
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(20);
  pinMode(pin, INPUT_PULLUP);
  delayMicroseconds(30);
  noInterrupts();
  Status s = OK;
  if (!waitWhile(pin, HIGH, 100) || !waitWhile(pin, LOW, 100) || !waitWhile(pin, HIGH, 100)) s = NO_REPLY;
  for (uint8_t i = 0; s == OK && i < 40; i++) {
    if (!waitWhile(pin, LOW, 100)) { s = TIMEOUT; break; }
    uint16_t high = waitWhile(pin, HIGH, 120);
    if (!high) { s = TIMEOUT; break; }
    b[i / 8] = (b[i / 8] << 1) | (high > 45 ? 1 : 0);
  }
  interrupts();
  return s;
}

bool Dht11::read() {
  uint32_t now = millis();
  if (started && now - lastMs < 1000) return good;
  if (!started && now - lastMs < 1000) { last = NOT_YET; return false; }
  started = true;
  lastMs = now;
  uint8_t b[5] = {0, 0, 0, 0, 0};
  last = transfer(b);
  if (last == OK) {
    if (decode(b, t, rh)) good = true;
    else last = BAD_CHECKSUM;
  }
  return good;
}
