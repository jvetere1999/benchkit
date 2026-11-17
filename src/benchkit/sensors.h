// Simple sensors, one class each. Every one: begin(pin) in setup(), read() in loop().
//
//   Pot pot;            pot.begin(A0);       pot.read();  pot.percent()  0..100, changed() when it moved
//   Ldr ldr;            ldr.begin(A1);       ldr.read();  ldr.percent()  0 dark .. 100 bright, dark()
//   Tmp36 temp;         temp.begin(A2);      temp.read(); temp.tenthsC() 215 = 21.5 °C
//   Thermistor th;      th.begin(A3);        th.read();   th.tenthsC()
//   SharpIr ir;         ir.begin(A4);        ir.read();   ir.cm()       10..80, 0 = nothing in range
//   TiltSwitch tilt;    tilt.begin(4);       if (tilt.read()) ... tilted() changed
//   VibrationSensor v;  v.begin(5);          if (v.read()) ... it shook
//
// Analog readings are smoothed (a running average that follows changes in ~8 readings), so
// numbers don't flicker. The conversions are plain functions too (tmp36TenthsC() etc.), tested
// on the computer: cd extras/dev && pio test -e native
#pragma once
#include <stdint.h>

// Running average of 0..1023 readings: each new reading moves it 1/8 of the way.
// Kept x16 so it's smooth without floating point.
class Smooth {
public:
  uint16_t add(uint16_t raw) {
    if (!started) { acc = raw << 4; started = true; }
    else acc += ((int32_t)(raw << 4) - (int32_t)acc) >> 3;
    return value();
  }
  uint16_t value() const { return (acc + 8) >> 4; }
private:
  uint16_t acc = 0;
  bool started = false;
};

// --- Conversions (pure, tested natively) ---------------------------------------------------------

// TMP36: 10 mV per °C, 500 mV at 0 °C. vrefMv = the board's analog reference (5000 on an Uno).
int16_t tmp36TenthsC(uint16_t raw, uint16_t vrefMv = 5000);
// 10 kΩ NTC thermistor in a divider with `seriesOhms`. thermistorOnTop: thermistor between 5V
// and the pin (series resistor to GND); false: series resistor to 5V, thermistor to GND.
// beta 3950 for most 10k NTCs. Returns tenths of °C; -999 if the reading is 0 or 1023 (open / short).
int16_t thermistorTenthsC(uint16_t raw, bool thermistorOnTop = false, uint32_t seriesOhms = 10000,
                          uint16_t beta = 3950, uint32_t nominalOhms = 10000);
// Sharp GP2Y0A21YK0F (10-80 cm) on a 5 V board: cm, 0 = out of range (too far or too close)
uint8_t sharpIrCm(uint16_t raw);

// --- Analog sensors -----------------------------------------------------------------------------

class AnalogSensor {   // the shared part (not virtual: each sensor just adds its conversion)
public:
#ifdef ARDUINO
  void begin(uint8_t pin);
  uint16_t read();               // reads the pin, returns the smoothed 0..1023
#endif
  uint16_t update(uint16_t raw) { return smooth.add(raw); }   // what read() does with a reading
  uint16_t raw() const { return smooth.value(); }
protected:
  Smooth smooth;
  uint8_t pin = 255;
};

class Pot : public AnalogSensor {
public:
  uint8_t percent() const { return ((uint32_t)raw() * 100 + 511) / 1023; }   // rounded
  // Scaled to any range, e.g. map(0, 255) for a brightness
  long map(long lo, long hi) const { return lo + (long)raw() * (hi - lo) / 1023; }
  // True when percent() moved by `step` or more since the last time it said true (no jitter)
  bool changed(uint8_t step = 2) {
    int16_t p = percent();
    if (p - last >= step || last - p >= step || (last < 0)) { last = p; return true; }
    return false;
  }
private:
  int16_t last = -1;
};

// Photoresistor in a divider: LDR from 5V to the pin, 10 kΩ from the pin to GND (brighter = higher).
class Ldr : public AnalogSensor {
public:
  uint8_t percent() const { return ((uint32_t)raw() * 100 + 511) / 1023; }   // rounded
  // Dark below `darkAt` %, light again above darkAt + 5 % (so it doesn't flicker at the edge)
  bool dark(uint8_t darkAt = 30) {
    uint8_t p = percent();
    if (isDark && p > darkAt + 5) isDark = false;
    else if (!isDark && p < darkAt) isDark = true;
    return isDark;
  }
private:
  bool isDark = false;
};

// TMP36: flat side facing you, pins left to right: 5V, Vout (to the analog pin), GND.
class Tmp36 : public AnalogSensor {
public:
  uint16_t vrefMv = 5000;
  int16_t tenthsC() const { return tmp36TenthsC(raw(), vrefMv); }
  int16_t tenthsF() const { return tenthsC() * 9 / 5 + 320; }
};

class Thermistor : public AnalogSensor {
public:
  bool onTop = false;            // see thermistorTenthsC()
  uint32_t seriesOhms = 10000;
  uint16_t beta = 3950;
  int16_t tenthsC() const { return thermistorTenthsC(raw(), onTop, seriesOhms, beta); }
};

// Sharp GP2Y0A21: red 5V, black GND, yellow to the analog pin. Put a 10 µF capacitor across
// 5V/GND near it: it pulls current in bursts that make its own reading jump.
class SharpIr : public AnalogSensor {
public:
  uint8_t cm() const { return sharpIrCm(raw()); }
};

// --- Digital sensors -----------------------------------------------------------------------------

// Ball tilt switch (SW-200D): one leg to the pin, the other to GND (uses the internal pull-up).
// The ball rattles, so a change only counts once it has held for `settleMs`.
class TiltSwitch {
public:
  uint16_t settleMs = 50;
#ifdef ARDUINO
  void begin(uint8_t pin);
  bool read();                   // true when tilted() just changed
#endif
  bool update(bool closed, uint32_t now);
  bool tilted() const { return state; }   // switch open = tilted (ball rolled off the contacts)
private:
#ifdef ARDUINO
  uint8_t pin = 255;
#endif
  bool raw = false, state = false;
  uint32_t since = 0;
};

// Vibration module (SW-420 type: VCC, GND, DO). DO changes when it shakes; the blue pot sets how
// hard. activeHigh: true if DO goes HIGH on a shake (most SW-420 boards; flip it if yours reads
// shaking all the time). A shake counts once, then it waits `quietMs` before counting another.
class VibrationSensor {
public:
  uint16_t quietMs = 200;
  bool activeHigh = true;
#ifdef ARDUINO
  void begin(uint8_t pin);
  bool read();                   // true once per shake
#endif
  bool update(bool level, uint32_t now);
  uint16_t shakes = 0;           // shakes so far
private:
#ifdef ARDUINO
  uint8_t pin = 255;
#endif
  bool armed = true;
  uint32_t last = 0;
};
