// A 2-axis analog joystick (KY-023 style: GND, +5V, VRx, VRy, SW). Pins are passed in:
//
//   Joystick stick;
//   void setup() { stick.begin(JOY_X, JOY_Y); }   // measures the centre: don't touch the stick
//   void loop()  { stick.read(); ... stick.x, stick.y ... }   // -100..100 %, right and up positive
//
// The click (SW) is a button: Button click; click.begin(JOY_SW, Button::PULL_UP);
//
// Optional, only if you make one: StickPresses turns pushes into left/right/up/down "presses"
// (menus, games):   StickPresses dirs;  switch (dirs.update(stick.x, stick.y)) { ... }
#pragma once
#include <stdint.h>

// Raw 0..1023 -> -100..100 %. Each side is scaled on its own, so a stick that rests off-centre
// still reaches 100 % both ways. Within `deadzone` raw units of the centre = 0.
int8_t stickPercent(int16_t raw, int16_t center, uint8_t deadzone, bool invert);

class Joystick {
public:
  int8_t x = 0, y = 0;                 // -100..100 %, right and up positive
  int16_t rawX = 512, rawY = 512;      // latest raw readings, 0..1023
  int16_t centerX = 512, centerY = 512;
  uint8_t deadzone = 12;               // raw units around the centre treated as centred
  bool invertX = false, invertY = true;   // most modules read high when pushed down

#ifdef ARDUINO
  // Set the pins and measure the centre (16 readings, ~32 ms). The stick must be untouched.
  void begin(uint8_t pinX, uint8_t pinY);
  void read();                         // read both axes and update x / y
#endif

  void update(int16_t rawX, int16_t rawY);   // the logic behind read()
  bool centred() const { return x == 0 && y == 0; }

private:
#ifdef ARDUINO
  uint8_t pinX = 255, pinY = 255;
#endif
};

enum StickDir : uint8_t { STICK_NONE, STICK_LEFT, STICK_RIGHT, STICK_UP, STICK_DOWN };

// One "press" each time the stick is pushed past `on` % in a direction; it has to come back
// under `off` % before that direction fires again. The bigger axis wins on diagonals.
class StickPresses {
public:
  explicit StickPresses(uint8_t on = 60, uint8_t off = 30) : on(on), off(off) {}
  StickDir update(int8_t x, int8_t y);   // call after each read
  StickDir held() const { return current; }   // the direction pushed right now, or NONE
private:
  uint8_t on, off;
  StickDir current = STICK_NONE;
};
