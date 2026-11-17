// A counter and a bar on a 128x64 SSD1306 OLED.
// Wiring: SDA A4, SCL A5 (Mega: 20, 21), VCC 5V, GND.
#include <Benchkit.h>

Ssd1306 oled;

void setup() {
  Serial.begin(115200);
  if (!oled.begin()) Serial.println(F("no OLED at 0x3C or 0x3D"));
}

void loop() {
  uint8_t n = (millis() / 100) % 100;
  oled.fill(OLED_OFF);
  oled.text(0, 0, F("Benchkit"), OLED_ON, OLED_OFF);
  oled.bar(0, 20, 128, 10, n, 100, OLED_ON, OLED_OFF, OLED_ON);
  oled.show();               // sends only what changed
}
