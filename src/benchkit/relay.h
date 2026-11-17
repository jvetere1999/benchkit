// A relay, such as the Songle SRD-05VDC-SL-C.
//
//   Relay r;  r.begin(RELAY_PIN);   // relay modules that switch on with LOW: r.begin(pin, true)
//   r.on();  r.off();  r.onFor(2000);  r.update();   // update() every loop
//
// A bare relay can't go on a pin: its coil needs ~70 mA. Drive it with an NPN transistor
// (PN2222: pin -> 1 kΩ -> base, emitter to GND, collector to one coil end, other coil end to
// 5V) and put a diode (1N4004) across the coil, stripe to 5V, or switching it off kills the
// transistor. Mains on the contacts is out of scope for a breadboard: keep it to low voltage.
//
// Switching too often wears the contacts out (and can be a mistake in the code, like a sensor
// flickering at its threshold), so changes closer together than minSwitchMs wait their turn.
#pragma once
#include <stdint.h>

class Relay {
public:
  uint16_t minSwitchMs = 100;

  void begin(uint8_t pin, bool activeLow = false);
  void set(bool on);
  void on() { set(true); }
  void off() { set(false); }
  void onFor(uint32_t ms);       // then off by itself (needs update())
  void update();                 // every loop
  bool isOn() const { return state; }      // what the contacts are doing now
  bool wanted() const { return want; }     // what was asked for (may still be waiting)
  uint16_t switches = 0;                   // times it actually switched

  // The logic: should it switch now?
  static bool due(bool state, bool want, uint32_t sinceSwitchMs, uint16_t minSwitchMs) {
    return state != want && sinceSwitchMs >= minSwitchMs;
  }

private:
  uint8_t pin = 255;
  bool activeLow = false, state = false, want = false, timed = false;
  uint32_t lastSwitch = 0, offAt = 0;
  void apply(uint32_t now);
};
