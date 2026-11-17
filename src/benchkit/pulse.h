// On/off timing without delay(), for anything that's just on or off (an LED, an active buzzer,
// a relay). The part's class calls update(now) every loop and sets its pin to what it returns.
//
//   flash(150)            on for 150 ms, then off
//   blink(200, 800)       200 ms on, 800 ms off, forever
//   blink(100, 100, 3)    3 quick blinks, then off
//   on() / off()          steady (stops any blinking)
#pragma once
#include <stdint.h>

class Pulse {
public:
  void on() { onMs = 0; count = 0; level = true; mode = STEADY; }
  void off() { onMs = 0; count = 0; level = false; mode = STEADY; }
  void flash(uint16_t ms, uint32_t now) { blink(ms, 0, 1, now); }
  // times = 0: forever
  void blink(uint16_t onFor, uint16_t offFor, uint8_t times, uint32_t now) {
    onMs = onFor; offMs = offFor; count = times; level = true; since = now; mode = BLINK;
  }
  bool update(uint32_t now) {        // call every loop; returns whether it should be on now
    if (mode != BLINK) return level;
    uint32_t span = level ? onMs : offMs;
    while (now - since >= span) {
      since += span;
      if (level) {
        level = false;
        if (count && --count == 0) { mode = STEADY; return false; }   // that was the last one
      } else {
        level = true;
      }
      span = level ? onMs : offMs;
      if (span == 0) { since = now; break; }
    }
    return level;
  }
  bool busy() const { return mode == BLINK; }   // flashing / blinking right now
  bool isOn() const { return level; }

private:
  enum : uint8_t { STEADY, BLINK };
  uint8_t mode = STEADY, count = 0;
  bool level = false;
  uint16_t onMs = 0, offMs = 0;
  uint32_t since = 0;
};
