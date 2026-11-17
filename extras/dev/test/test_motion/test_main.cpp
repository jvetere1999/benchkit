// motor (H-bridge), stepper (28BYJ-48 + ULN2003)
#include <unity.h>
#include <Arduino.h>
#include <Benchkit.h>

void setUp() { fake::reset(); }
void tearDown() {}

void test_motor_directions_and_stops() {
  Motor m; m.begin(9, 7, 8);
  m.speed(100);
  TEST_ASSERT_EQUAL(HIGH, fake::level[7]); TEST_ASSERT_EQUAL(LOW, fake::level[8]); TEST_ASSERT_EQUAL(255, fake::pwm[9]);
  m.speed(-50);
  TEST_ASSERT_EQUAL(LOW, fake::level[7]); TEST_ASSERT_EQUAL(HIGH, fake::level[8]); TEST_ASSERT_INT_WITHIN(1, 127, fake::pwm[9]);
  m.brake();
  TEST_ASSERT_EQUAL(fake::level[7], fake::level[8]); TEST_ASSERT_EQUAL(255, fake::pwm[9]);   // shorted: fast stop
  m.coast();
  TEST_ASSERT_EQUAL(0, fake::pwm[9]);
  m.speed(120);
  TEST_ASSERT_EQUAL(100, m.current());         // clamped
}

void test_motor_min_duty() {
  Motor m; m.begin(9, 7, 8);
  m.minDuty = 100;
  m.speed(1);
  TEST_ASSERT_INT_WITHIN(2, 101, fake::pwm[9]);
}

void test_motor_ramp() {
  Motor m; m.begin(9, 7, 8);
  m.rampMs = 500;                              // 0 -> 100 % in 500 ms
  m.speed(100);
  TEST_ASSERT_EQUAL(0, m.current());
  for (int i = 0; i < 250; i++) { fake::us += 1000; m.update(); }
  TEST_ASSERT_INT_WITHIN(1, 50, m.current());
  for (int i = 0; i < 300; i++) { fake::us += 1000; m.update(); }
  TEST_ASSERT_EQUAL(100, m.current());
  m.speed(-100);                               // reversing goes through 0
  for (int i = 0; i < 500; i++) { fake::us += 1000; m.update(); }
  TEST_ASSERT_INT_WITHIN(1, 0, m.current());
}

void test_ramp_step_carries_leftover_time() {
  uint16_t carry = 0;
  int8_t s = 0;
  for (int i = 0; i < 100; i++) s = Motor::step(s, 100, 3, 1000, carry);   // 3 ms at a time, 1000 ms ramp
  TEST_ASSERT_INT_WITHIN(1, 30, s);            // 300 ms -> 30 %, not 0 from rounding each call down
}

void test_stepper_pattern() {
  // each half-step changes exactly one coil
  for (uint8_t p = 0; p < 8; p++) {
    uint8_t a = Stepper28BYJ::pattern(p), b = Stepper28BYJ::pattern(p + 1), diff = a ^ b;
    TEST_ASSERT_TRUE(diff && !(diff & (diff - 1)));
  }
}

void test_stepper_moves_at_speed_and_releases() {
  Stepper28BYJ s; s.begin(2, 3, 4, 5);
  s.rpm(10);                                   // 4096 steps a turn: ~1465 us a step
  s.move(100);
  for (int i = 0; i < 1465 * 50; i += 10) { fake::us += 10; s.update(); }
  TEST_ASSERT_INT_WITHIN(2, 49, s.position()); // first call only energises
  for (int i = 0; i < 1465 * 60; i += 10) { fake::us += 10; s.update(); }
  TEST_ASSERT_EQUAL(100, s.position());
  TEST_ASSERT_FALSE(s.busy());
  TEST_ASSERT_EQUAL(0, fake::level[2] + fake::level[3] + fake::level[4] + fake::level[5]);   // coils off
  s.moveTo(-8);
  for (int i = 0; i < 1465 * 200; i += 10) { fake::us += 10; s.update(); }
  TEST_ASSERT_EQUAL(-8, s.position());
}

void test_stepper_holds_when_asked() {
  Stepper28BYJ s; s.begin(2, 3, 4, 5);
  s.holdWhenStopped = true;
  s.move(3);
  for (int i = 0; i < 100000; i += 10) { fake::us += 10; s.update(); }
  TEST_ASSERT_TRUE(fake::level[2] + fake::level[3] + fake::level[4] + fake::level[5] > 0);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_motor_directions_and_stops);
  RUN_TEST(test_motor_min_duty);
  RUN_TEST(test_motor_ramp);
  RUN_TEST(test_ramp_step_carries_leftover_time);
  RUN_TEST(test_stepper_pattern);
  RUN_TEST(test_stepper_moves_at_speed_and_releases);
  RUN_TEST(test_stepper_holds_when_asked);
  return UNITY_END();
}
