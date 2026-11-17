// pulse, led, sound, relay, shift595, seg7
#include <unity.h>
#include <Arduino.h>
#include <Benchkit.h>

void setUp() { fake::reset(); }
void tearDown() {}
static void ms(uint32_t n) { fake::us += n * 1000; }

void test_pulse_blink_count() {
  Pulse p;
  p.blink(100, 100, 3, 0);
  int ons = 0; bool was = false;
  for (uint32_t t = 0; t < 2000; t++) { bool on = p.update(t); if (on && !was) ons++; was = on; }
  TEST_ASSERT_EQUAL(3, ons);
  TEST_ASSERT_FALSE(p.busy());
  TEST_ASSERT_FALSE(p.isOn());
}

void test_pulse_flash_and_forever() {
  Pulse p;
  p.flash(150, 1000);
  TEST_ASSERT_TRUE(p.update(1149));
  TEST_ASSERT_FALSE(p.update(1150));
  p.blink(10, 10, 0, 0);
  TEST_ASSERT_TRUE(p.update(100005));          // still going much later
  TEST_ASSERT_TRUE(p.busy());
  p.off();
  TEST_ASSERT_FALSE(p.update(200000));
}

void test_led_flash_turns_itself_off() {
  Led led; led.begin(13);
  led.flash(100);
  TEST_ASSERT_EQUAL(HIGH, fake::level[13]);
  ms(99); led.update();
  TEST_ASSERT_EQUAL(HIGH, fake::level[13]);
  ms(2); led.update();
  TEST_ASSERT_EQUAL(LOW, fake::level[13]);
  Led inv; inv.begin(12, true);                // active low: on = LOW
  inv.on();
  TEST_ASSERT_EQUAL(LOW, fake::level[12]);
}

void test_led_brightness_and_rgb() {
  Led led; led.begin(9);
  led.brightness(255); TEST_ASSERT_EQUAL(255, fake::pwm[9]);
  led.brightness(128); TEST_ASSERT_INT_WITHIN(2, 64, fake::pwm[9]);   // gamma: half looks half
  RgbLed rgb; rgb.begin(3, 5, 6, RgbLed::ANODE);
  rgb.set(0xFF0000);
  TEST_ASSERT_EQUAL(0, fake::pwm[3]);          // anode: full on = LOW
  TEST_ASSERT_EQUAL(255, fake::pwm[5]);
  TEST_ASSERT_EQUAL_HEX32(0xFF0000, hueRgb(0));
  TEST_ASSERT_EQUAL_HEX32(0x00FF00, hueRgb(85));
}

void test_buzzer_plays_a_tune() {
  static const Note TUNE[] = {{440, 100}, {0, 50}, {880, 100}};
  Buzzer bz; bz.begin(8);
  bz.play(TUNE, 3);
  TEST_ASSERT_TRUE(bz.playing());
  for (int i = 0; i < 300; i++) { ms(1); bz.update(); }
  TEST_ASSERT_FALSE(bz.playing());
  TEST_ASSERT_EQUAL_STRING("t8=440 n8 t8=880 n8 ", fake::log.c_str());
}

void test_active_buzzer_beeps() {
  ActiveBuzzer ab; ab.begin(7);
  fake::log.clear();
  ab.beeps(2, 50, 50);
  for (int i = 0; i < 300; i++) { ms(1); ab.update(); }
  TEST_ASSERT_EQUAL_STRING("w7=1 w7=0 w7=1 w7=0 ", fake::log.c_str());
}

void test_relay_min_time_between_switches() {
  Relay r; r.begin(4);
  ms(500);
  r.on();
  TEST_ASSERT_TRUE(r.isOn());
  r.off();                                     // too soon: waits
  TEST_ASSERT_TRUE(r.isOn());
  TEST_ASSERT_FALSE(r.wanted());
  ms(100); r.update();
  TEST_ASSERT_FALSE(r.isOn());
  TEST_ASSERT_EQUAL(2, r.switches);
}

void test_relay_on_for() {
  Relay r; r.begin(4, true);                   // active low module
  TEST_ASSERT_EQUAL(HIGH, fake::level[4]);     // off
  ms(200);
  r.onFor(300);
  TEST_ASSERT_EQUAL(LOW, fake::level[4]);
  ms(299); r.update(); TEST_ASSERT_TRUE(r.isOn());
  ms(2); r.update(); TEST_ASSERT_FALSE(r.isOn());
}

void test_shift595_sends_far_chip_first_and_only_on_change() {
  Shift595<2> sr; sr.begin(2, 3, 4);
  fake::log.clear();
  sr.set(0, true);                             // chip 0, Q0
  sr.set(15, true);                            // chip 1, Q7
  TEST_ASSERT_TRUE(sr.write());
  TEST_ASSERT_EQUAL_STRING("w4=0 s80 s01 w4=1 ", fake::log.c_str());
  fake::log.clear();
  TEST_ASSERT_FALSE(sr.write());               // nothing changed
  TEST_ASSERT_EQUAL_STRING("", fake::log.c_str());
  TEST_ASSERT_TRUE(sr.get(15));
  sr.set(99, true);                            // out of range: ignored
}

void test_seg7_characters() {
  TEST_ASSERT_EQUAL_HEX8(0x3F, segmentsFor('0'));
  TEST_ASSERT_EQUAL_HEX8(0x7F, segmentsFor('8'));
  TEST_ASSERT_EQUAL_HEX8(0x77, segmentsFor('A'));
  TEST_ASSERT_EQUAL_HEX8(0x00, segmentsFor('W'));   // can't be shown: blank
}

static void expect4(Seg7x4 &f, const char *chars, uint8_t dots) {
  for (uint8_t i = 0; i < 4; i++)
    TEST_ASSERT_EQUAL_HEX8(segmentsFor(chars[i]) | ((dots >> (3 - i)) & 1 ? 0x80 : 0), f.at(i));
}

void test_seg7x4_numbers() {
  const uint8_t S[8] = {2, 3, 4, 5, 6, 7, 8, 9}, D[4] = {10, 11, 12, 13};
  Seg7x4 f; f.begin(S, D);
  f.number(42);     expect4(f, "  42", 0);
  f.number(-7);     expect4(f, "  -7", 0);
  f.number(12345);  expect4(f, "----", 0);
  f.fixed(215, 1);  expect4(f, " 215", 0b0010);    // " 21.5"
  f.fixed(5, 1);    expect4(f, "  05", 0b0010);    // "  0.5"
  f.fixed(-55, 1);  expect4(f, " -55", 0b0010);    // " -5.5"
  f.text("HI.");    expect4(f, "HI  ", 0b0100);
}

void test_seg7x4_lights_one_digit_at_a_time() {
  const uint8_t S[8] = {2, 3, 4, 5, 6, 7, 8, 9}, D[4] = {10, 11, 12, 13};
  Seg7x4 f; f.begin(S, D);
  f.number(1234);
  for (int i = 0; i < 4; i++) {
    fake::us += 2000; f.update();
    int lit = 0;
    for (int d = 10; d <= 13; d++) lit += fake::level[d] == LOW;   // cathode: LOW = this digit on
    TEST_ASSERT_EQUAL(1, lit);
  }
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_pulse_blink_count);
  RUN_TEST(test_pulse_flash_and_forever);
  RUN_TEST(test_led_flash_turns_itself_off);
  RUN_TEST(test_led_brightness_and_rgb);
  RUN_TEST(test_buzzer_plays_a_tune);
  RUN_TEST(test_active_buzzer_beeps);
  RUN_TEST(test_relay_min_time_between_switches);
  RUN_TEST(test_relay_on_for);
  RUN_TEST(test_shift595_sends_far_chip_first_and_only_on_change);
  RUN_TEST(test_seg7_characters);
  RUN_TEST(test_seg7x4_numbers);
  RUN_TEST(test_seg7x4_lights_one_digit_at_a_time);
  return UNITY_END();
}
