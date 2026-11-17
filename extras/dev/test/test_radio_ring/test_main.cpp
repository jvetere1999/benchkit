// screenlog (LogRows), nrf24 (AckHistory), servo_t1 (ServoJog), pixelring (ring maths),
// and the SSD1306 reset pin
#include <unity.h>
#include <Arduino.h>
#include <Benchkit.h>

void setUp() { fake::reset(); }
void tearDown() {}

void test_log_rows_wrap_and_keep_the_newest() {
  LogRows<5, 3> log;
  TEST_ASSERT_EQUAL_STRING("     ", log.row(0));
  const char *text = "one\ntwo\nthree\nfour\n";
  for (const char *c = text; *c; c++) log.add(*c);
  TEST_ASSERT_EQUAL_STRING("two  ", log.row(0));   // the oldest kept
  TEST_ASSERT_EQUAL_STRING("three", log.row(1));
  TEST_ASSERT_EQUAL_STRING("four ", log.row(2));   // the newest
  uint16_t v = log.version();
  for (const char *c = "toolong\r\n"; *c; c++) log.add(*c);   // wraps; \r ignored
  TEST_ASSERT_EQUAL_STRING("toolo", log.row(1));
  TEST_ASSERT_EQUAL_STRING("ng   ", log.row(2));
  TEST_ASSERT_TRUE(log.version() != v);
}

void test_ack_history_counts_the_last_twenty() {
  AckHistory a;
  TEST_ASSERT_EQUAL(0, a.percent());
  a.add(true); a.add(false);
  TEST_ASSERT_EQUAL(50, a.percent());
  for (int i = 0; i < 20; i++) a.add(true);
  TEST_ASSERT_EQUAL(100, a.percent());
  TEST_ASSERT_EQUAL(20, a.sent());
  for (int i = 0; i < 5; i++) a.add(false);
  TEST_ASSERT_EQUAL(75, a.percent());
}

void test_jog_dead_zone_direction_and_limits() {
  const int32_t U = 256;   // 1 us
  TEST_ASSERT_EQUAL(1500 * U, servoJog256(1500 * U, 8, 100, 10, 800, 500, 2500));      // resting stick
  TEST_ASSERT_EQUAL(1580 * U, servoJog256(1500 * U, 100, 100, 10, 800, 500, 2500));    // full: 80 us in 100 ms
  TEST_ASSERT_EQUAL(1420 * U, servoJog256(1500 * U, -100, 100, 10, 800, 500, 2500));
  TEST_ASSERT_EQUAL(1540 * U, servoJog256(1500 * U, 55, 100, 10, 800, 500, 2500));     // half over: half speed
  TEST_ASSERT_EQUAL(2500 * U, servoJog256(2490 * U, 100, 100, 10, 800, 500, 2500));
  TEST_ASSERT_EQUAL(500 * U, servoJog256(510 * U, -128, 100, 10, 800, 500, 2500));
  TEST_ASSERT_TRUE(servoJog256(1500 * U, 11, 5, 10, 800, 500, 2500) > 1500 * U);       // one tick still moves
}

void test_servo_jog_ticks_and_caps_a_pause() {
  ServoJog jog(1500, 500, 2500, 800);
  TEST_ASSERT_TRUE(jog.update(100, 1000));     // first call: 100 ms' worth, not 1 s
  TEST_ASSERT_EQUAL(1580, jog.us());
  TEST_ASSERT_FALSE(jog.update(100, 1003));    // under tickMs: not yet
  TEST_ASSERT_TRUE(jog.update(100, 1005));
  TEST_ASSERT_EQUAL(1584, jog.us());
  TEST_ASSERT_FALSE(jog.update(0, 1010));      // centred: doesn't move
  jog.set(2500);
  TEST_ASSERT_FALSE(jog.update(100, 1020));    // at the end already
}

void test_ring_step_takes_the_short_way() {
  TEST_ASSERT_EQUAL(6, ringStep(0, 6, 24));
  TEST_ASSERT_EQUAL(-6, ringStep(6, 0, 24));
  TEST_ASSERT_EQUAL(3, ringStep(22, 1, 24));    // across the 23 -> 0 seam
  TEST_ASSERT_EQUAL(-3, ringStep(1, 22, 24));
}

void test_ring_distance_the_short_way_round() {
  TEST_ASSERT_EQUAL(0, ringDistance256(5, 5 * 256, 24));
  TEST_ASSERT_EQUAL(2 * 256, ringDistance256(23, 1 * 256, 24));   // across the seam
  TEST_ASSERT_EQUAL(12 * 256, ringDistance256(0, 12 * 256, 24));  // the far side
  TEST_ASSERT_EQUAL(128, ringDistance256(3, 3 * 256 + 128, 24));
}

void test_ring_shades() {
  TEST_ASSERT_EQUAL(0, ringShade(0, 24, 64));
  TEST_ASSERT_EQUAL(63, ringShade(12 * 256, 24, 64));   // far side: last shade
  TEST_ASSERT_EQUAL(0, ringShadeDistance(0, 24, 64));
  TEST_ASSERT_EQUAL(12 * 256, ringShadeDistance(63, 24, 64));
  for (uint8_t k = 0; k < 64; k++)                      // round trip
    TEST_ASSERT_EQUAL(k, ringShade(ringShadeDistance(k, 24, 64), 24, 64));
}

void test_oled_reset_pin_is_pulsed_then_left_high() {
  Ssd1306 oled;
  oled.begin(0, 8);
  TEST_ASSERT_EQUAL(OUTPUT, fake::mode[8]);
  TEST_ASSERT_EQUAL(HIGH, fake::level[8]);
  TEST_ASSERT_TRUE(fake::log.find("w8=0") != std::string::npos);   // it went low once
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_log_rows_wrap_and_keep_the_newest);
  RUN_TEST(test_ack_history_counts_the_last_twenty);
  RUN_TEST(test_jog_dead_zone_direction_and_limits);
  RUN_TEST(test_servo_jog_ticks_and_caps_a_pause);
  RUN_TEST(test_ring_step_takes_the_short_way);
  RUN_TEST(test_ring_distance_the_short_way_round);
  RUN_TEST(test_ring_shades);
  RUN_TEST(test_oled_reset_pin_is_pulsed_then_left_high);
  return UNITY_END();
}
