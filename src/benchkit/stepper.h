// The 28BYJ-48 5 V stepper on its ULN2003 driver board:
//
//   Stepper28BYJ st;
//   st.begin(IN1, IN2, IN3, IN4);    // the board's IN1-IN4, in that order
//   st.rpm(10);                      // up to ~15
//   st.move(Stepper28BYJ::TURN / 4);  // a quarter turn (negative = the other way)
//   st.update();                     // every loop: takes a step when it's time
//   if (!st.busy()) ...
//
// Power the driver board from 5V (the motor draws ~240 mA: fine from an Uno's USB 5V for one,
// better from its own supply). Coils are switched off once it stops (it'd get warm otherwise);
// set holdWhenStopped if something pushes against it.
#pragma once
#include <stdint.h>

class Stepper28BYJ {
public:
  static const int32_t TURN = 4096;   // half-steps per output-shaft turn (64 x the 63.68:1 gearbox, rounded)
  bool holdWhenStopped = false;

  void begin(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4);
  void rpm(uint8_t r);                // 1..15
  void move(int32_t steps) { target += steps; }
  void moveTo(int32_t position) { target = position; }
  void stop() { target = pos; }
  void setPosition(int32_t p) { pos = target = p; }   // e.g. 0 at a limit switch
  void update();
  void release();                     // coils off now
  bool busy() const { return pos != target; }
  int32_t position() const { return pos; }

  // The coil pattern for a half-step phase 0..7, bit 0 = IN1 .. bit 3 = IN4
  static uint8_t pattern(uint8_t phase);

private:
  uint8_t pins[4] = {255, 255, 255, 255};
  int32_t pos = 0, target = 0;
  uint32_t intervalUs = 1465, lastUs = 0;   // 10 rpm
  bool energised = false;
  void out(uint8_t bits);
};
