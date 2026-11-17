// A debounced push button. The pin is passed in at begin(), so your sketch's pin list stays the
// one place for its pins:
//
//   Button btn;
//   void setup() { btn.begin(BUTTON_PIN); }                 // external pull-down: pressed = HIGH
//   void loop()  { if (btn.read() == BUTTON_TAP) ... }       // call every loop
//
//   Button click;  click.begin(JOY_SW, Button::PULL_UP);     // joystick SW: pressed = LOW
//
// Extras are separate objects, so a project that doesn't make one pays nothing for it:
//
//   DoubleTap dbl;      e = dbl.update(btn.read(), millis());   // TAP / DOUBLE_TAP
//   Repeat rep;         if (rep.update(btn, millis())) ...      // menu-style auto-repeat
//   PressCount count;   count.add(e);                           // taps/holds for a radio packet
//
// The timing logic (update()) has no Arduino calls, so it's tested on the computer:
//   cd extras/dev && pio test -e native
#pragma once
#include <stdint.h>

enum ButtonEvent : uint8_t {
  BUTTON_NONE,
  BUTTON_TAP,         // pressed and let go before holdMs (reported on release)
  BUTTON_HOLD,        // held for holdMs (reported once, while still held)
  BUTTON_LONG_HOLD,   // held for longHoldMs (once, after HOLD; only if longHoldMs is set)
  BUTTON_DOUBLE_TAP,  // two taps close together (only from DoubleTap)
};

class Button {
public:
  enum Wiring : uint8_t {
    PULL_DOWN,   // 10 kΩ from the pin to GND, button to 5V: pressed reads HIGH
    PULL_UP,     // the board's internal pull-up, button to GND: pressed reads LOW
  };

  uint16_t debounceMs = 30;
  uint16_t holdMs = 800;
  uint16_t longHoldMs = 0;   // 0 = no LONG_HOLD event

#ifdef ARDUINO
  void begin(uint8_t pin, Wiring wiring = PULL_DOWN);
  ButtonEvent read();        // reads the pin; call every loop
#endif

  // The logic behind read(): `pressed` is the raw reading (true = pressed), `now` in ms.
  ButtonEvent update(bool pressed, uint32_t now);

  bool down() const { return flags & STABLE; }             // held right now (debounced)
  bool pressed() const { return flags & PRESSED_NOW; }     // went down in the last update
  bool released() const { return flags & RELEASED_NOW; }   // came up in the last update
  uint32_t heldFor(uint32_t now) const { return down() ? now - pressStartMs : 0; }

private:
  enum : uint8_t { RAW = 1, STABLE = 2, HOLD_FIRED = 4, LONG_FIRED = 8,
                   PRESSED_NOW = 16, RELEASED_NOW = 32, ACTIVE_LOW = 64 };
#ifdef ARDUINO
  uint8_t pin = 255;
#endif
  uint8_t flags = 0;
  uint32_t changeMs = 0, pressStartMs = 0;
};

// Turns taps into single and double taps. A single TAP is reported once gapMs has passed
// with no second tap, so it arrives gapMs late: only use this when you need double taps.
// Other events pass straight through.
class DoubleTap {
public:
  explicit DoubleTap(uint16_t gapMs = 300) : gapMs(gapMs) {}
  ButtonEvent update(ButtonEvent e, uint32_t now);   // call every loop, even with BUTTON_NONE
private:
  uint16_t gapMs;
  bool waiting = false;
  uint32_t firstMs = 0;
};

// Auto-repeat while held, like a keyboard: fires on the press, then after delayMs, then
// every everyMs until released.
class Repeat {
public:
  explicit Repeat(uint16_t delayMs = 400, uint16_t everyMs = 100) : delayMs(delayMs), everyMs(everyMs) {}
  bool update(const Button &b, uint32_t now);        // call every loop, after b.read()
private:
  uint16_t delayMs, everyMs;
  uint32_t nextMs = 0;
};

// Counts presses. Counters wrap at 255: send them in every radio packet and the receiver
// works out how many happened from the difference, so a lost packet can't lose a press.
struct PressCount {
  uint8_t taps = 0, holds = 0, longHolds = 0, doubleTaps = 0;
  void add(ButtonEvent e) {
    if (e == BUTTON_TAP) taps++;
    else if (e == BUTTON_HOLD) holds++;
    else if (e == BUTTON_LONG_HOLD) longHolds++;
    else if (e == BUTTON_DOUBLE_TAP) doubleTaps++;
  }
};
