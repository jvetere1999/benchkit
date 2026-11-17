// Shapes and text on a 1.8" ST7735 colour screen, on any pins.
// Wiring: SDA/MOSI 4, SCK 5, DC 9, CS 10, RST 8, VCC 5V, GND, LED 3.3V.
#include <Benchkit.h>

St7735 tft;

void setup() {
  tft.begin(4, 5, 9, 10, 8);  // landscape 160 x 128, cleared
  tft.text(4, 4, F("Benchkit"), WHITE, BLACK);
  tft.circle(80, 70, 30, color565(60, 140, 255));
}

void loop() {
  tft.bar(10, 112, 140, 8, (millis() / 50) % 100, 100, hue565(millis() / 30), BLACK, WHITE);
}
