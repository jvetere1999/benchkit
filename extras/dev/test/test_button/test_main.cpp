// button: debounce, tap, hold, long hold, edges, double tap, repeat, counters.
#include <unity.h>
#include <Benchkit.h>

void setUp() {}
void tearDown() {}

// Feed a level for a stretch of time, 1 ms steps; returns the events seen (in order) as a string
static const char *NAMES = "-THLD";
static char seen[32];
static uint8_t nSeen;
static uint32_t t;
static void run(Button &b, bool level, uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++, t++) {   // (counts steps, so t can wrap past 0xFFFFFFFF)
    ButtonEvent e = b.update(level, t);
    if (e != BUTTON_NONE && nSeen < sizeof(seen) - 1) { seen[nSeen++] = NAMES[e]; seen[nSeen] = 0; }
  }
}
static void reset() { t = 1000; nSeen = 0; seen[0] = 0; }

void test_tap() {
  Button b; reset();
  run(b, false, 100);
  run(b, true, 200);
  run(b, false, 100);
  TEST_ASSERT_EQUAL_STRING("T", seen);
}

void test_bounces_are_ignored() {
  Button b; reset();
  for (int i = 0; i < 5; i++) { run(b, true, 3); run(b, false, 3); }   // contact chatter
  run(b, false, 100);
  TEST_ASSERT_EQUAL_STRING("", seen);
  TEST_ASSERT_FALSE(b.down());
}

void test_hold_once_and_no_tap_after() {
  Button b; reset();
  run(b, true, 2000);
  run(b, false, 100);
  TEST_ASSERT_EQUAL_STRING("H", seen);
}

void test_hold_fires_at_hold_ms_after_debounce() {
  Button b; reset();
  run(b, true, 30 + 799);
  TEST_ASSERT_EQUAL_STRING("", seen);
  run(b, true, 2);
  TEST_ASSERT_EQUAL_STRING("H", seen);
}

void test_long_hold_only_when_set() {
  Button b; reset();
  b.longHoldMs = 2000;
  run(b, true, 3000);
  run(b, false, 100);
  TEST_ASSERT_EQUAL_STRING("HL", seen);
}

void test_edges_last_one_update() {
  Button b; reset();
  run(b, false, 50);
  bool sawPress = false;
  for (int i = 0; i < 100; i++) { b.update(true, t++); if (b.pressed()) { TEST_ASSERT_FALSE(sawPress); sawPress = true; } }
  TEST_ASSERT_TRUE(sawPress);
  TEST_ASSERT_TRUE(b.down());
  TEST_ASSERT_EQUAL_UINT32(70, b.heldFor(t));
  bool sawRelease = false;
  for (int i = 0; i < 100; i++) { b.update(false, t++); if (b.released()) sawRelease = true; }
  TEST_ASSERT_TRUE(sawRelease);
  TEST_ASSERT_FALSE(b.down());
}

void test_millis_wraparound() {
  Button b;
  t = 0xFFFFFFFFu - 100; nSeen = 0; seen[0] = 0;
  run(b, false, 50);
  run(b, true, 200);    // crosses 0xFFFFFFFF -> 0
  run(b, false, 100);
  TEST_ASSERT_EQUAL_STRING("T", seen);
}

void test_double_tap() {
  Button b; DoubleTap d(300); reset();
  auto go = [&](bool level, uint32_t ms) {
    for (uint32_t end = t + ms; t < end; t++) {
      ButtonEvent e = d.update(b.update(level, t), t);
      if (e != BUTTON_NONE) { seen[nSeen++] = NAMES[e]; seen[nSeen] = 0; }
    }
  };
  go(true, 80); go(false, 100); go(true, 80); go(false, 500);   // two quick taps
  TEST_ASSERT_EQUAL_STRING("D", seen);
  nSeen = 0; seen[0] = 0;
  go(true, 80); go(false, 500);                                 // one tap: reported late
  TEST_ASSERT_EQUAL_STRING("T", seen);
}

void test_repeat() {
  Button b; Repeat r(400, 100); reset();
  int fires = 0;
  for (uint32_t end = t + 1000; t < end; t++) { b.update(true, t); if (r.update(b, t)) fires++; }
  // press (after 30 ms debounce), then 430, 530, ... 930: 1 + 6
  TEST_ASSERT_EQUAL(7, fires);
  for (uint32_t end = t + 500; t < end; t++) { b.update(false, t); TEST_ASSERT_FALSE(r.update(b, t)); }
}

void test_press_count_wraps() {
  PressCount c;
  for (int i = 0; i < 300; i++) c.add(BUTTON_TAP);
  c.add(BUTTON_HOLD);
  TEST_ASSERT_EQUAL_UINT8(300 - 256, c.taps);
  TEST_ASSERT_EQUAL_UINT8(1, c.holds);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_tap);
  RUN_TEST(test_bounces_are_ignored);
  RUN_TEST(test_hold_once_and_no_tap_after);
  RUN_TEST(test_hold_fires_at_hold_ms_after_debounce);
  RUN_TEST(test_long_hold_only_when_set);
  RUN_TEST(test_edges_last_one_update);
  RUN_TEST(test_millis_wraparound);
  RUN_TEST(test_double_tap);
  RUN_TEST(test_repeat);
  RUN_TEST(test_press_count_wraps);
  return UNITY_END();
}
