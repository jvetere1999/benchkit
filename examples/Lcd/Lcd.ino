// Uptime on a 16x2 LCD.
// Wiring: RS 12, E 11, D4-D7 5, 4, 3, 2; see lcd1602.h for contrast and backlight.
#include <Benchkit.h>

Lcd1602 lcd;

void setup() {
  lcd.begin(12, 11, 5, 4, 3, 2);
}

void loop() {
  lcd.text(0, 0, F("Up for"));
  lcd.number(0, 1, millis() / 1000, 6);   // rewriting every loop is free: only changes are sent
  lcd.text(7, 1, F("s"));
}
