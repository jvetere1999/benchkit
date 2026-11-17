#include "sensors.h"
#include <math.h>
#ifdef ARDUINO
#include <Arduino.h>

void AnalogSensor::begin(uint8_t p) { pin = p; pinMode(pin, INPUT); update(analogRead(pin)); }
uint16_t AnalogSensor::read() { return update(analogRead(pin)); }

void TiltSwitch::begin(uint8_t p) {
  pin = p;
  pinMode(pin, INPUT_PULLUP);
  raw = state = digitalRead(pin) == HIGH;   // start settled on whatever it is now
}
bool TiltSwitch::read() { return update(digitalRead(pin) == LOW, millis()); }

void VibrationSensor::begin(uint8_t p) { pin = p; pinMode(pin, INPUT); }
bool VibrationSensor::read() { return update(digitalRead(pin) == HIGH, millis()); }
#endif

int16_t tmp36TenthsC(uint16_t raw, uint16_t vrefMv) {
  int32_t mv10 = (int32_t)raw * vrefMv * 10 / 1024;   // tenths of a millivolt
  return (mv10 - 5000) / 10;                          // 10 mV per °C: tenths of °C = (mV - 500)
}

int16_t thermistorTenthsC(uint16_t raw, bool onTop, uint32_t series, uint16_t beta, uint32_t nominal) {
  if (raw == 0 || raw >= 1023) return -999;
  // the divider: reading = 1023 * Rbottom / (Rtop + Rbottom)
  float ratio = (float)raw / (1023 - raw);   // Rbottom / Rtop
  float r = onTop ? series / ratio : series * ratio;
  float kelvin = 1.0f / (1.0f / 298.15f + logf(r / nominal) / beta);
  return (int16_t)lroundf((kelvin - 273.15f) * 10);
}

uint8_t sharpIrCm(uint16_t raw) {
  // The sensor's curve (distance against voltage) is close to cm = 4800 / (raw - 20) on 5 V.
  // Under ~0.4 V it's past 80 cm; the curve folds back under 10 cm, so very close reads as far.
  if (raw <= 80) return 0;
  uint16_t cm = 4800 / (raw - 20);
  return cm < 10 || cm > 80 ? 0 : cm;
}

bool TiltSwitch::update(bool closed, uint32_t now) {
  bool tilt = !closed;
  if (tilt != raw) { raw = tilt; since = now; }
  if (raw != state && now - since >= settleMs) { state = raw; return true; }
  return false;
}

bool VibrationSensor::update(bool level, uint32_t now) {
  bool shaking = level == activeHigh;
  if (shaking && armed) { armed = false; last = now; shakes++; return true; }
  if (shaking) last = now;                         // still shaking: keep waiting
  else if (!armed && now - last >= quietMs) armed = true;
  return false;
}
