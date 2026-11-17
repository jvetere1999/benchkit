// HC-SR04 distance in cm.
// Wiring: VCC 5V, Trig 7, Echo 6, GND.
#include <Benchkit.h>

HcSr04 sonar;

void setup() {
  Serial.begin(115200);
  sonar.begin(7, 6);
}

void loop() {
  static uint32_t last;
  if (millis() - last >= 100) {            // >= 60 ms between readings
    last = millis();
    int16_t cm = sonar.cmMedian();
    if (cm < 0) Serial.println(F("nothing")); else Serial.println(cm);
  }
}
