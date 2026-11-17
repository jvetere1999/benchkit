// DHT11 temperature and humidity.
// Wiring: the 3-pin module: S to pin 2, + to 5V, - to GND.
#include <Benchkit.h>

Dht11 dht;

void setup() {
  Serial.begin(115200);
  dht.begin(2);
}

void loop() {
  if (dht.read()) {
    Serial.print(dht.tempC()); Serial.print(F(" C  "));
    Serial.print(dht.humidity()); Serial.println(F(" %"));
  }
  delay(1000);   // the chip only measures once a second anyway
}
