// A DC motor on one side of an L293D or SN754410 (same pinout):
//
//   Motor m;
//   m.begin(EN_PIN, IN1_PIN, IN2_PIN);   // EN on a PWM pin (Uno 3 5 6 9 10 11)
//   m.speed(60);                          // 60 % forwards;  m.speed(-100) full reverse
//   m.brake();  m.coast();
//   m.rampMs = 500;  m.update();          // optional: 0 -> 100 % takes 500 ms; call update() every loop
//
// Chip wiring: pin 16 to 5V (logic), pin 8 to the motor supply (not the board's 5V for anything
// bigger than a tiny motor), 4 5 12 13 to GND; enable 1 (motor A, outputs 3 and 6) or 9 (motor
// B, 11 and 14). Join the motor supply's GND to the board's. The L293D has the flyback diodes
// built in; the SN754410 does not, so put diodes across its motor outputs (1N4004 is fine).
#pragma once
#include <stdint.h>

class Motor {
public:
  uint16_t rampMs = 0;           // time to go 0 -> 100 %; 0 = change at once
  uint8_t minDuty = 0;           // PWM below which the motor just hums: speeds above 0 start here

  void begin(uint8_t en, uint8_t in1, uint8_t in2);
  void speed(int8_t percent);    // -100..100; with rampMs, it gets there over time in update()
  void brake();                  // both outputs to the same level: stops quickly
  void coast();                  // outputs off: spins down freely
  void update();                 // every loop when rampMs is set

  int8_t target() const { return want; }
  int8_t current() const { return now; }

  // The ramp's logic (no pins): moves `current` toward `target` for `elapsedMs`
  static int8_t step(int8_t current, int8_t target, uint32_t elapsedMs, uint16_t rampMs, uint16_t &carryMs);

private:
  uint8_t en = 255, in1 = 255, in2 = 255;
  int8_t want = 0, now = 0;
  uint16_t carry = 0;
  uint32_t last = 0;
  void apply();
};
