// A hobby servo on Timer1's hardware PWM, and moving one with a joystick.
//
//   Timer1Servo arm;   arm.begin(11, 1500);   arm.us(1800);   // pulse width in microseconds
//
// The pulse comes from the timer itself, so it keeps its exact width while interrupts are off
// (NeoPixel show(), long serial bursts): the servo never twitches, which the Servo library
// can't promise. Only Timer1's two pins work: Uno / Nano / Leonardo 9 or 10, Mega 11 or 12
// (begin() returns false on another pin). Timer1 is then busy: no Servo library and no
// analogWrite on those two pins. A big servo (e.g. DS3235, ~2.3 A at stall) needs its own
// 5-7.4 V supply, with that supply's GND joined to the board's GND; only the signal wire goes to
// the board.
//
//   ServoJog jog(1500, 500, 2500, 800);   // start, min, max (us), speed at full stick (us/s)
//   loop(): if (jog.update(stick.x, millis())) arm.us256(jog.pos256());
//
// ServoJog moves faster the further the stick is pushed (nothing inside the dead zone), in
// steps of 1/256 us, so slow moves are smooth. It's the part without the timer (tested on the
// computer).
#pragma once
#include <stdint.h>

// Move a position (1/256 us) for `dtMs` with the stick at `joy` (-100..100, + = towards maxUs).
// Keep dtMs <= 100 and usPerS <= 10000 (no overflow).
inline int32_t servoJog256(int32_t pos, int8_t joy, uint16_t dtMs, uint8_t dead, uint16_t usPerS,
                           uint16_t minUs, uint16_t maxUs) {
  int16_t j = joy;
  int16_t mag = j < 0 ? -j : j;
  if (mag > 100) mag = 100;
  if (mag <= dead) return pos;
  int32_t speed = (int32_t)(mag - dead) * usPerS * 256 / (100 - dead);   // 1/256 us per second
  int32_t step = speed * dtMs / 1000;
  pos += j > 0 ? step : -step;
  if (pos < (int32_t)minUs * 256) pos = (int32_t)minUs * 256;
  if (pos > (int32_t)maxUs * 256) pos = (int32_t)maxUs * 256;
  return pos;
}

class ServoJog {
public:
  ServoJog(uint16_t startUs, uint16_t minUs, uint16_t maxUs, uint16_t usPerS, uint8_t deadZone = 10)
      : pos((int32_t)startUs * 256), minUs(minUs), maxUs(maxUs), usPerS(usPerS), dead(deadZone) {}

  uint8_t tickMs = 5;   // move this often (a servo takes a new position every 20 ms anyway)

  // Call every loop. Returns true when the position changed. After a pause (more than 100 ms
  // since the last move) it moves 100 ms' worth, not a leap.
  bool update(int8_t joy, uint32_t nowMs) {
    uint32_t dt = nowMs - lastMs;
    if (dt < tickMs) return false;
    lastMs = nowMs;
    if (dt > 100) dt = 100;
    int32_t was = pos;
    pos = servoJog256(pos, joy, (uint16_t)dt, dead, usPerS, minUs, maxUs);
    return pos != was;
  }

  uint16_t us() const { return (uint16_t)((pos + 128) >> 8); }
  int32_t pos256() const { return pos; }
  void set(uint16_t us) { pos = (int32_t)us * 256; }

private:
  int32_t pos;
  uint16_t minUs, maxUs, usPerS;
  uint8_t dead;
  uint32_t lastMs = 0;
};

#if defined(__AVR__) && !defined(NATIVE_TEST)
class Timer1Servo {
public:
  bool begin(uint8_t pin, uint16_t startUs = 1500);   // false: not a Timer1 pin on this board
  void us(uint16_t us) { us256((int32_t)us * 256); }
  void us256(int32_t pos256);   // 1/256 us, as ServoJog keeps it (the timer has 0.5 us steps)
  uint16_t us() const { return current; }

private:
  uint8_t channel = 0;   // 1 = OC1A, 2 = OC1B
  uint16_t current = 0;
};
#endif
