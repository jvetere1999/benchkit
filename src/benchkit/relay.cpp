#include "relay.h"
#include <Arduino.h>

void Relay::begin(uint8_t p, bool low) {
  pin = p;
  activeLow = low;
  digitalWrite(pin, activeLow ? HIGH : LOW);   // off before it becomes an output: no click at start
  pinMode(pin, OUTPUT);
  state = want = false;
  lastSwitch = millis() - minSwitchMs;
}

void Relay::apply(uint32_t now) {
  if (!due(state, want, now - lastSwitch, minSwitchMs)) return;
  state = want;
  lastSwitch = now;
  switches++;
  digitalWrite(pin, state != activeLow ? HIGH : LOW);
}

void Relay::set(bool on) { want = on; timed = false; apply(millis()); }

void Relay::onFor(uint32_t ms) {
  uint32_t now = millis();
  want = true;
  timed = true;
  offAt = now + ms;
  apply(now);
}

void Relay::update() {
  uint32_t now = millis();
  if (timed && (int32_t)(now - offAt) >= 0) { want = false; timed = false; }
  apply(now);
}
