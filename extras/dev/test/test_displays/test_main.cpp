// gfx (via a fake 32x16 canvas), ssd1306 (buffer + only sending what changed), lcd1602.
#include <unity.h>
#include <Arduino.h>
#include <Wire.h>
#include <Benchkit.h>

// A pretend display: just a pixel array, to check what Gfx draws
struct Canvas : public Gfx<Canvas> {
  uint16_t px[16][32] = {};
  int calls = 0;
  int16_t width() const { return 32; }
  int16_t height() const { return 16; }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) {
    calls++;
    for (int16_t yy = y; yy < y + h; yy++)
      for (int16_t xx = x; xx < x + w; xx++)
        if (xx >= 0 && yy >= 0 && xx < 32 && yy < 16) px[yy][xx] = c;
  }
  int count(uint16_t c) const { int n = 0; for (auto &r : px) for (auto v : r) n += v == c; return n; }
};

void setUp() { fake::reset(); Wire.log.clear(); Wire.bytes = 0; Wire.present = 0x3C; }
void tearDown() {}

void test_gfx_lines_and_rects() {
  Canvas c;
  c.line(0, 0, 9, 9, 1);
  for (int i = 0; i < 10; i++) TEST_ASSERT_EQUAL(1, c.px[i][i]);
  TEST_ASSERT_EQUAL(10, c.count(1));
  Canvas r;
  r.rect(2, 2, 5, 4, 7);
  TEST_ASSERT_EQUAL(14, r.count(7));          // outline only: 5 + 5 + 2 + 2
  TEST_ASSERT_EQUAL(0, r.px[3][3]);
}

void test_gfx_circle_is_symmetric_and_filled() {
  Canvas c;
  c.fillCircle(15, 7, 5, 1);
  TEST_ASSERT_EQUAL(1, c.px[7][10]); TEST_ASSERT_EQUAL(1, c.px[7][20]);
  TEST_ASSERT_EQUAL(1, c.px[2][15]); TEST_ASSERT_EQUAL(1, c.px[12][15]);
  TEST_ASSERT_EQUAL(0, c.px[2][10]);          // corner of the box: outside
  for (int dy = 0; dy <= 5; dy++)              // mirror images above / below and left / right
    for (int dx = 0; dx <= 5; dx++) {
      TEST_ASSERT_EQUAL(c.px[7 + dy][15 + dx], c.px[7 - dy][15 + dx]);
      TEST_ASSERT_EQUAL(c.px[7 + dy][15 + dx], c.px[7 + dy][15 - dx]);
    }
}

void test_gfx_bar_and_text() {
  Canvas c;
  c.bar(0, 0, 12, 4, 5, 10, 2, 3, 9);          // inside is 10 wide: 5 on, 5 off
  TEST_ASSERT_EQUAL(2, c.px[1][1]); TEST_ASSERT_EQUAL(2, c.px[1][5]);
  TEST_ASSERT_EQUAL(3, c.px[1][6]); TEST_ASSERT_EQUAL(9, c.px[0][0]);
  Canvas t;
  int16_t end = t.text(0, 0, "I", 1, 0);
  TEST_ASSERT_EQUAL(6, end);
  TEST_ASSERT_TRUE(t.count(1) > 3);           // 'I' has some lit pixels...
  TEST_ASSERT_EQUAL(0, t.px[0][5]);            // ...and the 6th column is the gap
  TEST_ASSERT_EQUAL(30, t.text(0, 8, "ABCDEFGHIJ", 1, 0));   // stops at the edge (5 fit in 32)
}

void test_colours() {
  TEST_ASSERT_EQUAL_HEX16(0xF800, color565(255, 0, 0));
  TEST_ASSERT_EQUAL_HEX16(0xF800, hue565(0));
  TEST_ASSERT_EQUAL_HEX16(0, dim565(0xFFFF, 0));
}

void test_ssd1306_finds_the_address_and_sends_the_screen_once() {
  Wire.present = 0x3D;
  Ssd1306 oled;
  TEST_ASSERT_TRUE(oled.begin());
  TEST_ASSERT_EQUAL_HEX8(0x3D, oled.address());
  TEST_ASSERT_TRUE(Wire.bytes > 1024);          // the whole screen, plus commands
  Wire.present = 0;
  Ssd1306 none;
  TEST_ASSERT_FALSE(none.begin());
}

void test_ssd1306_only_sends_changes() {
  Ssd1306 oled;
  oled.begin();
  Wire.bytes = 0; Wire.log.clear();
  oled.show();
  TEST_ASSERT_EQUAL(0, Wire.bytes);             // nothing changed
  oled.fillRect(10, 9, 3, 2, OLED_ON);          // inside page 1, columns 10-12
  TEST_ASSERT_TRUE(oled.get(11, 10));
  TEST_ASSERT_FALSE(oled.get(11, 8));
  oled.show();
  TEST_ASSERT_EQUAL_STRING("[3c: 00 21 0a 0c 22 01 01] [3c: 40 06 06 06] ", Wire.log.c_str());
  Wire.log.clear();
  oled.fillRect(10, 9, 3, 2, OLED_ON);          // same again: no change, nothing to send
  oled.show();
  TEST_ASSERT_EQUAL_STRING("", Wire.log.c_str());
}

void test_ssd1306_32_rows() {
  Ssd1306x32 small;
  TEST_ASSERT_EQUAL(32, small.height());
  TEST_ASSERT_TRUE(sizeof(small) < 600);        // a 512-byte buffer, not 1 KB
  small.fillRect(0, 30, 200, 10, OLED_ON);      // clipped to the screen
  TEST_ASSERT_TRUE(small.get(127, 31));
}

void test_lcd_only_writes_what_changed() {
  Lcd1602 lcd;
  lcd.begin(2, 3, 4, 5, 6, 7);
  fake::log.clear();
  lcd.text(0, 0, "Hi");
  size_t first = fake::log.size();
  TEST_ASSERT_TRUE(first > 0);
  TEST_ASSERT_EQUAL('H', lcd.at(0, 0));
  fake::log.clear();
  lcd.text(0, 0, "Hi");                          // same text: nothing sent
  TEST_ASSERT_EQUAL(0, (int)fake::log.size());
  lcd.text(0, 0, "Ho");                          // one character changed: a move + that character
  TEST_ASSERT_EQUAL(first, fake::log.size());    // (the same cost as writing "Hi" from the row start)
  TEST_ASSERT_EQUAL('o', lcd.at(1, 0));
  fake::log.clear();
  lcd.text(2, 0, "!");                           // right after the last character: no move needed
  TEST_ASSERT_EQUAL(first / 2, fake::log.size()); // one character, no position command
  TEST_ASSERT_EQUAL_UINT8(16, lcd.text(10, 1, "too long for it"));   // cut at the edge
  lcd.number(0, 1, 42, 4);
  TEST_ASSERT_EQUAL(' ', lcd.at(1, 1));
  TEST_ASSERT_EQUAL('4', lcd.at(2, 1));
  TEST_ASSERT_EQUAL('2', lcd.at(3, 1));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_gfx_lines_and_rects);
  RUN_TEST(test_gfx_circle_is_symmetric_and_filled);
  RUN_TEST(test_gfx_bar_and_text);
  RUN_TEST(test_colours);
  RUN_TEST(test_ssd1306_finds_the_address_and_sends_the_screen_once);
  RUN_TEST(test_ssd1306_only_sends_changes);
  RUN_TEST(test_ssd1306_32_rows);
  RUN_TEST(test_lcd_only_writes_what_changed);
  return UNITY_END();
}
