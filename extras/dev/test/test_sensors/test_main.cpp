// sensors (pot, LDR, TMP36, thermistor, Sharp IR, tilt, vibration), ultrasonic, dht11
#include <unity.h>
#include <Arduino.h>
#include <Benchkit.h>

void setUp() { fake::reset(); }
void tearDown() {}

void test_smooth_follows_without_jumping() {
  Smooth s;
  TEST_ASSERT_EQUAL(500, s.add(500));          // the first reading is taken as is
  s.add(1000);
  TEST_ASSERT_TRUE(s.value() > 500 && s.value() < 600);   // 1/8 of the way
  for (int i = 0; i < 100; i++) s.add(1000);
  TEST_ASSERT_INT_WITHIN(1, 1000, s.value());
}

void test_pot_percent_map_changed() {
  Pot p;
  fake::analog[A0] = 1023;
  p.begin(A0);
  TEST_ASSERT_EQUAL(100, p.percent());
  TEST_ASSERT_EQUAL(255, p.map(0, 255));
  TEST_ASSERT_TRUE(p.changed());               // first time
  TEST_ASSERT_FALSE(p.changed());
  fake::analog[A0] = 1010;                     // a little jitter
  for (int i = 0; i < 50; i++) p.read();
  TEST_ASSERT_FALSE(p.changed());
  fake::analog[A0] = 0;
  for (int i = 0; i < 50; i++) p.read();
  TEST_ASSERT_TRUE(p.changed());
}

void test_ldr_dark_with_hysteresis() {
  Ldr l;
  l.update(200);                               // ~20 %
  TEST_ASSERT_TRUE(l.dark(30));
  Ldr m;
  m.update(320);                               // ~31 %: light
  TEST_ASSERT_FALSE(m.dark(30));
  for (int i = 0; i < 60; i++) m.update(290);  // ~28 %: dark
  TEST_ASSERT_TRUE(m.dark(30));
  for (int i = 0; i < 60; i++) m.update(330);  // ~32 %: still dark (needs > 35 %)
  TEST_ASSERT_TRUE(m.dark(30));
  for (int i = 0; i < 60; i++) m.update(400);
  TEST_ASSERT_FALSE(m.dark(30));
}

void test_tmp36() {
  TEST_ASSERT_INT_WITHIN(3, 0, tmp36TenthsC(102));      // ~0.5 V = 0 °C
  TEST_ASSERT_INT_WITHIN(3, 250, tmp36TenthsC(153));    // 0.75 V = 25 °C
  TEST_ASSERT_INT_WITHIN(3, -100, tmp36TenthsC(82));    // 0.4 V = -10 °C
  TEST_ASSERT_INT_WITHIN(3, 250, tmp36TenthsC(232, 3300));   // on a 3.3 V board
  Tmp36 t; t.update(153);
  TEST_ASSERT_EQUAL(t.tenthsC() * 9 / 5 + 320, t.tenthsF());
  TEST_ASSERT_INT_WITHIN(10, 770, t.tenthsF());   // ~25 °C = ~77 °F
}

void test_thermistor() {
  TEST_ASSERT_INT_WITHIN(2, 250, thermistorTenthsC(512));          // equal resistors: 25 °C
  // thermistor to GND (the default): a higher reading = more ohms = colder
  TEST_ASSERT_TRUE(thermistorTenthsC(600) < 250);
  TEST_ASSERT_TRUE(thermistorTenthsC(600, true) > 250);            // thermistor to 5V: the other way
  TEST_ASSERT_INT_WITHIN(10, 0, thermistorTenthsC(1023 * 32650 / (32650 + 10000)));   // 32.65 kΩ = 0 °C
  TEST_ASSERT_EQUAL(-999, thermistorTenthsC(0));                   // disconnected
}

void test_sharp_ir() {
  TEST_ASSERT_EQUAL(0, sharpIrCm(50));         // nothing there
  TEST_ASSERT_INT_WITHIN(1, 20, sharpIrCm(260));
  TEST_ASSERT_EQUAL(0, sharpIrCm(700));        // closer than 10 cm: unreliable
}

void test_tilt_settles() {
  TiltSwitch t;
  TEST_ASSERT_FALSE(t.update(true, 0));        // upright (contacts closed)
  TEST_ASSERT_FALSE(t.tilted());
  for (uint32_t ms = 0; ms < 40; ms += 5) t.update(ms % 10 == 0, ms);   // rattling
  TEST_ASSERT_FALSE(t.tilted());
  bool changed = false;
  for (uint32_t ms = 40; ms < 200; ms++) changed |= t.update(false, ms);
  TEST_ASSERT_TRUE(changed);
  TEST_ASSERT_TRUE(t.tilted());
}

void test_vibration_counts_once_per_shake() {
  VibrationSensor v;
  int n = 0;
  for (uint32_t ms = 0; ms < 100; ms++) n += v.update(ms % 3 == 0, ms);   // a burst of pulses
  for (uint32_t ms = 100; ms < 400; ms++) n += v.update(false, ms);       // quiet
  for (uint32_t ms = 400; ms < 450; ms++) n += v.update(true, ms);        // another shake
  TEST_ASSERT_EQUAL(2, n);
  TEST_ASSERT_EQUAL(2, v.shakes);
}

void test_ultrasonic() {
  TEST_ASSERT_EQUAL(-1, echoToMm(0));
  TEST_ASSERT_INT_WITHIN(2, 100, echoToMm(583));   // 10 cm
  HcSr04 s; s.begin(7, 8);
  fake::pulse = 5831;                          // 1 m
  TEST_ASSERT_INT_WITHIN(1, 100, s.cm());
  fake::pulse = 0;
  TEST_ASSERT_EQUAL(-1, s.cm());
  TEST_ASSERT_EQUAL(20, HcSr04::median3(20, 300, 19));
  TEST_ASSERT_EQUAL(20, HcSr04::median3(-1, 20, -1));
  TEST_ASSERT_EQUAL(-1, HcSr04::median3(-1, -1, -1));
}

void test_dht11_decode() {
  int8_t t; uint8_t h;
  const uint8_t good[5] = {45, 0, 23, 0, 68};
  TEST_ASSERT_TRUE(Dht11::decode(good, t, h));
  TEST_ASSERT_EQUAL(23, t); TEST_ASSERT_EQUAL(45, h);
  const uint8_t bad[5] = {45, 0, 23, 0, 69};
  TEST_ASSERT_FALSE(Dht11::decode(bad, t, h));
  const uint8_t cold[5] = {80, 0, 3, 0x80, (uint8_t)(80 + 3 + 0x80)};
  TEST_ASSERT_TRUE(Dht11::decode(cold, t, h));
  TEST_ASSERT_EQUAL(-3, t);
}

void test_dht11_waits_after_power_up() {
  Dht11 d; d.begin(4);
  TEST_ASSERT_FALSE(d.read());
  TEST_ASSERT_EQUAL(Dht11::NOT_YET, d.status());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_smooth_follows_without_jumping);
  RUN_TEST(test_pot_percent_map_changed);
  RUN_TEST(test_ldr_dark_with_hysteresis);
  RUN_TEST(test_tmp36);
  RUN_TEST(test_thermistor);
  RUN_TEST(test_sharp_ir);
  RUN_TEST(test_tilt_settles);
  RUN_TEST(test_vibration_counts_once_per_shake);
  RUN_TEST(test_ultrasonic);
  RUN_TEST(test_dht11_decode);
  RUN_TEST(test_dht11_waits_after_power_up);
  return UNITY_END();
}
