// Joystick position in percent, and its click.
// Wiring: VRx A0, VRy A1, SW 3, +5V, GND. Don't touch the stick while it starts (it measures the centre).
#include <Benchkit.h>

Joystick stick;
Button click;

void setup() {
  Serial.begin(115200);
  stick.begin(A0, A1);
  click.begin(3, Button::PULL_UP);
}

void loop() {
  stick.read();
  if (click.read() == BUTTON_TAP) Serial.println(F("click"));
  static uint32_t last;
  if (millis() - last > 200) {
    last = millis();
    Serial.print(stick.x); Serial.print(' '); Serial.println(stick.y);
  }
}
