// joystick: percent scaling, dead zone, inversion, direction presses.
#include <unity.h>
#include <Benchkit.h>

void setUp() {}
void tearDown() {}

void test_centre_and_dead_zone() {
  TEST_ASSERT_EQUAL_INT8(0, stickPercent(512, 512, 12, false));
  TEST_ASSERT_EQUAL_INT8(0, stickPercent(523, 512, 12, false));
  TEST_ASSERT_NOT_EQUAL(0, stickPercent(524, 512, 12, false));
}

void test_full_travel_both_ways_off_centre() {
  // rests at 600: each side is scaled on its own, so both ends still reach 100 %
  TEST_ASSERT_EQUAL_INT8(100, stickPercent(1023, 600, 12, false));
  TEST_ASSERT_EQUAL_INT8(-100, stickPercent(0, 600, 12, false));
  TEST_ASSERT_EQUAL_INT8(50, stickPercent(600 + 423 / 2 + 1, 600, 12, false));
}

void test_invert() {
  TEST_ASSERT_EQUAL_INT8(-100, stickPercent(1023, 512, 12, true));
}

void test_update_uses_settings() {
  Joystick j;          // invertY on by default: pushing up reads low
  j.update(1023, 0);
  TEST_ASSERT_EQUAL_INT8(100, j.x);
  TEST_ASSERT_EQUAL_INT8(100, j.y);
  TEST_ASSERT_EQUAL_INT16(1023, j.rawX);
  j.update(512, 512);
  TEST_ASSERT_TRUE(j.centred());
}

void test_presses_once_per_push() {
  StickPresses p(60, 30);
  TEST_ASSERT_EQUAL(STICK_RIGHT, p.update(70, 0));
  TEST_ASSERT_EQUAL(STICK_NONE, p.update(90, 0));    // still pushed: no repeat
  TEST_ASSERT_EQUAL(STICK_RIGHT, p.held());
  TEST_ASSERT_EQUAL(STICK_NONE, p.update(40, 0));    // between off and on: still held
  TEST_ASSERT_EQUAL(STICK_NONE, p.update(10, 0));    // let go
  TEST_ASSERT_EQUAL(STICK_NONE, p.held());
  TEST_ASSERT_EQUAL(STICK_RIGHT, p.update(70, 0));   // pushed again
}

void test_presses_directions_and_diagonals() {
  StickPresses p;
  TEST_ASSERT_EQUAL(STICK_LEFT, p.update(-80, 0)); p.update(0, 0);
  TEST_ASSERT_EQUAL(STICK_UP, p.update(0, 80));    p.update(0, 0);
  TEST_ASSERT_EQUAL(STICK_DOWN, p.update(0, -80)); p.update(0, 0);
  TEST_ASSERT_EQUAL(STICK_UP, p.update(65, 90));   // bigger axis wins
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_centre_and_dead_zone);
  RUN_TEST(test_full_travel_both_ways_off_centre);
  RUN_TEST(test_invert);
  RUN_TEST(test_update_uses_settings);
  RUN_TEST(test_presses_once_per_push);
  RUN_TEST(test_presses_directions_and_diagonals);
  return UNITY_END();
}
