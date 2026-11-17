// A potentiometer, a light sensor and a TMP36 on the serial monitor.
// Wiring: pot middle leg A0; LDR from 5V to A1 with 10 kOhm A1 to GND; TMP36 out on A2.
#include <Benchkit.h>

Pot pot;
Ldr ldr;
Tmp36 temp;

void setup() {
  Serial.begin(115200);
  pot.begin(A0);
  ldr.begin(A1);
  temp.begin(A2);
}

void loop() {
  pot.read(); ldr.read(); temp.read();
  if (pot.changed()) { Serial.print(F("pot ")); Serial.println(pot.percent()); }
  static uint32_t last;
  if (millis() - last > 1000) {
    last = millis();
    Serial.print(F("light ")); Serial.print(ldr.percent());
    Serial.print(F("  temp ")); Serial.println(temp.tenthsC() / 10.0, 1);
  }
}
