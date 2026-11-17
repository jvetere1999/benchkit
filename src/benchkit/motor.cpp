#include "motor.h"
#include <Arduino.h>

void Motor::begin(uint8_t e, uint8_t a, uint8_t b) {
  en = e; in1 = a; in2 = b;
  pinMode(en, OUTPUT); pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  coast();
}

void Motor::apply() {
  if (now == 0) { analogWrite(en, 0); return; }
  digitalWrite(in1, now > 0 ? HIGH : LOW);
  digitalWrite(in2, now > 0 ? LOW : HIGH);
  uint8_t pct = now > 0 ? now : -now;
  analogWrite(en, minDuty + (uint16_t)(255 - minDuty) * pct / 100);
}

void Motor::speed(int8_t p) {
  if (p > 100) p = 100;
  if (p < -100) p = -100;
  want = p;
  last = millis();
  if (!rampMs) { now = want; apply(); }
}

void Motor::brake() {
  want = now = 0;
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(en, 255);   // enabled with both sides low: the motor's ends are shorted together
}

void Motor::coast() {
  want = now = 0;
  analogWrite(en, 0);
}

void Motor::update() {
  if (now == want) return;
  uint32_t t = millis();
  int8_t next = step(now, want, t - last, rampMs, carry);
  last = t;
  if (next != now) { now = next; apply(); }
}

int8_t Motor::step(int8_t current, int8_t target, uint32_t elapsed, uint16_t ramp, uint16_t &carry) {
  if (!ramp) return target;
  uint32_t ms = elapsed + carry;
  uint32_t pct = ms * 100 / ramp;               // percent points we may move
  carry = ms - pct * ramp / 100;                // leftover time for next call
  if (pct == 0) return current;
  int16_t diff = target - current;
  if ((uint32_t)(diff < 0 ? -diff : diff) <= pct) { carry = 0; return target; }
  return current + (diff > 0 ? (int16_t)pct : -(int16_t)pct);
}
