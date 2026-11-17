#include "button.h"
#ifdef ARDUINO
#include <Arduino.h>

void Button::begin(uint8_t p, Wiring wiring) {
  pin = p;
  flags = wiring == PULL_UP ? ACTIVE_LOW : 0;
  pinMode(pin, wiring == PULL_UP ? INPUT_PULLUP : INPUT);
}

ButtonEvent Button::read() {
  bool level = digitalRead(pin) == HIGH;
  return update(flags & ACTIVE_LOW ? !level : level, millis());
}
#endif

ButtonEvent Button::update(bool pressedRaw, uint32_t now) {
  flags &= ~(PRESSED_NOW | RELEASED_NOW);
  if (pressedRaw != bool(flags & RAW)) {
    flags ^= RAW;
    changeMs = now;
  }
  if (now - changeMs < debounceMs) return BUTTON_NONE;   // still bouncing

  if (bool(flags & RAW) != bool(flags & STABLE)) {
    flags ^= STABLE;
    if (flags & STABLE) {
      pressStartMs = now;
      flags = (flags & ~(HOLD_FIRED | LONG_FIRED)) | PRESSED_NOW;
    } else {
      flags |= RELEASED_NOW;
      if (!(flags & HOLD_FIRED)) return BUTTON_TAP;
    }
  }

  if (flags & STABLE) {
    uint32_t held = now - pressStartMs;
    if (!(flags & HOLD_FIRED) && held >= holdMs) {
      flags |= HOLD_FIRED;
      return BUTTON_HOLD;
    }
    if (longHoldMs && !(flags & LONG_FIRED) && held >= longHoldMs) {
      flags |= LONG_FIRED;
      return BUTTON_LONG_HOLD;
    }
  }
  return BUTTON_NONE;
}

ButtonEvent DoubleTap::update(ButtonEvent e, uint32_t now) {
  if (e == BUTTON_TAP) {
    if (waiting && now - firstMs <= gapMs) {
      waiting = false;
      return BUTTON_DOUBLE_TAP;
    }
    waiting = true;
    firstMs = now;
    return BUTTON_NONE;
  }
  if (waiting && now - firstMs > gapMs) {
    waiting = false;
    if (e == BUTTON_NONE) return BUTTON_TAP;   // the late single tap
  }
  return e;
}

bool Repeat::update(const Button &b, uint32_t now) {
  if (b.pressed()) { nextMs = now + delayMs; return true; }
  if (b.down() && (int32_t)(now - nextMs) >= 0) { nextMs = now + everyMs; return true; }
  return false;
}
