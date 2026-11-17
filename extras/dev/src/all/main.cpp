// Not a real sketch: uses every Benchkit class once so `pio run` checks they all compile for
// the Uno and the Mega, and shows what they cost together. Pins are arbitrary.
#include <Arduino.h>
#include <Benchkit.h>

St7735 tft; Ssd1306 oled; Lcd1602 lcd;
Button btn; DoubleTap dbl; Repeat rep; PressCount count;
Joystick stick; StickPresses dirs;
Led led; RgbLed rgb; Buzzer bz; ActiveBuzzer ab;
Pot pot; Ldr ldr; Tmp36 tmp; Thermistor th; SharpIr ir; TiltSwitch tilt; VibrationSensor vib;
Motor motor; Stepper28BYJ stepper; Relay relay; Shift595<2> sr; Seg7 one; Seg7x4 four;
HcSr04 sonar; Dht11 dht;

const uint8_t SEGS[8] = {22, 23, 24, 25, 26, 27, 28, 29}, DIGS[4] = {30, 31, 32, 33};
const Note TUNE[] PROGMEM = {{NOTE_C5, 100}, {0, 50}, {NOTE_G5, 200}};
const uint16_t ARROW[3] PROGMEM = {0x8000, 0xC000, 0x8000};

void setup() {
  Serial.begin(115200);
  tft.begin(4, 5, 9, 10, 8); tft.text(0, 0, F("hi"), WHITE, BLACK); tft.circle(20, 20, 5, WHITE); tft.sprite(0, 0, ARROW, 2, 3, WHITE, BLACK);
  oled.begin(); oled.line(0, 0, 127, 63, OLED_ON); oled.text(0, 0, "x", OLED_ON, OLED_OFF); oled.show();
  lcd.begin(A0, A1, A2, A3, 6, 7); lcd.text(0, 0, F("ok")); lcd.number(0, 1, 42, 4);
  btn.begin(2); stick.begin(A4, A5);
  led.begin(13); led.blink(100, 100, 3); rgb.begin(3, 5, 6); rgb.hue(100);
  bz.begin(12); bz.play(TUNE, 3); ab.begin(11); ab.beeps(2);
  pot.begin(A4); ldr.begin(A5); tmp.begin(A3); th.begin(A1); ir.begin(A2); tilt.begin(34); vib.begin(35);
  motor.begin(44, 36, 37); motor.rampMs = 300; motor.speed(50);
  stepper.begin(38, 39, 40, 41); stepper.move(Stepper28BYJ::TURN);
  relay.begin(42); relay.onFor(500);
  sr.begin(43, 45, 46); sr.set(9, true); sr.write();
  one.begin(SEGS); one.show('7'); four.begin(SEGS, DIGS); four.fixed(-55, 1);
  sonar.begin(47, 48); dht.begin(49);
}

void loop() {
  ButtonEvent e = dbl.update(btn.read(), millis());
  count.add(e);
  if (rep.update(btn, millis())) stamp();
  stick.read(); dirs.update(stick.x, stick.y);
  led.update(); bz.update(); ab.update(); motor.update(); stepper.update(); relay.update(); four.update();
  pot.read(); ldr.read(); tmp.read(); th.read(); ir.read(); tilt.read(); vib.read();
  if (pot.changed()) Serial.println(pot.percent());
  Serial.println(tmp.tenthsC() + th.tenthsC() + ir.cm() + ldr.dark() + sonar.cm() + dht.read() + count.taps);
}
