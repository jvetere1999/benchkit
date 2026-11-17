#include "ultrasonic.h"
#include <Arduino.h>

void HcSr04::begin(uint8_t t, uint8_t e) {
  trig = t; echo = e;
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trig, LOW);
}

int16_t HcSr04::mm() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);     // a 10 µs pulse starts a measurement
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  uint32_t timeout = (uint32_t)maxCm * 20000 / 343 + 500;   // round trip for maxCm, plus the start-up
  return echoToMm(pulseIn(echo, HIGH, timeout));
}

int16_t HcSr04::median3(int16_t a, int16_t b, int16_t c) {
  int16_t v[3] = {a, b, c}, n = 0;
  for (uint8_t i = 0; i < 3; i++) if (v[i] >= 0) v[n++] = v[i];   // keep the real readings
  if (n == 0) return -1;
  if (n == 1) return v[0];
  if (n == 2) return (v[0] + v[1]) / 2;
  if (v[0] > v[1]) { int16_t t = v[0]; v[0] = v[1]; v[1] = t; }
  if (v[1] > v[2]) { int16_t t = v[1]; v[1] = v[2]; v[2] = t; }
  if (v[0] > v[1]) { int16_t t = v[0]; v[0] = v[1]; v[1] = t; }
  return v[1];
}

int16_t HcSr04::cmMedian() {
  int16_t a = cm(); delay(30);
  int16_t b = cm(); delay(30);
  int16_t c = cm();
  return median3(a, b, c);
}
