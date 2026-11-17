// Fade an RGB LED through the colour wheel.
// Wiring: R 9, G 10, B 11 (each through 220 Ohm), common cathode to GND.
#include <Benchkit.h>

RgbLed rgb;

void setup() {
  rgb.begin(9, 10, 11);      // RgbLed::ANODE as a 4th argument if the long leg goes to 5V
}

void loop() {
  rgb.hue(millis() / 20);
}
