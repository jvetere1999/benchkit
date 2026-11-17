// Like all/: not a real sketch. The classes that didn't fit in one Uno sketch with all the
// others (radio, NeoPixel ring, Timer1 servo, screen log), so `pio run` still checks every class
// compiles for the Uno and the Mega. Pins are arbitrary.
#include <Arduino.h>
#include <Benchkit.h>

Nrf24 radio; PixelRing<24> ring; Timer1Servo servo; ServoJog jog(1500, 500, 2500, 800);
ScreenLog<26, 9> Log(Serial); Joystick stick; Ssd1306 oled;

void setup() {
  Serial.begin(115200);
  Nrf24::park(7, 6);
  radio.begin(7, 6, 76, 1000, 0, 12);
  radio.sender("ADDR1");
  ring.begin(A0, NEO_GRBW + NEO_KHZ800, 60);
  ring.set(0, PixelRing<24>::hsv(21845));
  ring.show();
#if defined(__AVR_ATmega2560__)
  servo.begin(11);
#else
  servo.begin(9);
#endif
  stick.begin(A4, A5);
  oled.begin(0, 8);
  stamp(Log); Log.println(F("started"));
}

void loop() {
  uint8_t packet[12] = {};
  if (radio.send(packet, sizeof(packet))) Log.println(radio.linkPercent());
  if (radio.receive(packet, sizeof(packet))) Log.println(packet[0]);
  radio.sampleSignal();
  if (radio.state() == Nrf24::NO_CHIP) Serial.println(radio.signalPercent() + radio.signalOn(5));
  stick.read();
  if (jog.update(stick.x, millis())) servo.us256(jog.pos256());
  ring.set(ringShade(ringDistance256(1, 300, 24), 24, 64) % 24, PixelRing<24>::rgbw(0, 0, 0, 20));
  ring.show();
  oled.text(0, 0, Log.row(8), OLED_ON, OLED_OFF);
  oled.show();
}
