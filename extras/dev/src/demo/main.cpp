// Demo on an Uno:
//   button 2 (pull-down), joystick VRx A5 / VRy A4 / SW 3, ST7735 MOSI 4, SCK 5, DC 9, CS 10, RST 8.
// Tap / hold / double-tap the button, push and click the stick; the screen and serial show it.
#include <Arduino.h>
#include <Benchkit.h>

St7735 screen;
Button btn, click;
DoubleTap dbl;
Joystick stick;
StickPresses dirs;
PressCount count;

const uint16_t GREY = color565(90, 90, 90), ACCENT = color565(60, 140, 255);

void show(const __FlashStringHelper *what) {
  stamp();
  Serial.println(what);
  screen.fillRect(4, 100, 152, 8, BLACK);
  screen.text(4, 100, what, WHITE, BLACK);
}

void setup() {
  Serial.begin(115200);
  btn.begin(2);
  click.begin(3, Button::PULL_UP);
  stick.begin(A5, A4);
  unsigned long t0 = micros();
  screen.begin(4, 5, 9, 10, 8);   // MOSI, SCK, DC, CS, RST
  stamp(); Serial.print(F("Screen ready, clear took ")); Serial.print((micros() - t0) / 1000); Serial.println(F(" ms"));
  screen.text(4, 4, F("lib demo"), ACCENT, BLACK, 2);
  screen.rect(100, 30, 52, 52, GREY);
}

void loop() {
  static unsigned long lastDraw = 0;
  static int16_t dotX = 126, dotY = 56;
  unsigned long now = millis();

  ButtonEvent e = dbl.update(btn.read(), now);
  count.add(e);
  if (e == BUTTON_TAP) show(F("tap"));
  if (e == BUTTON_DOUBLE_TAP) show(F("double tap"));
  if (e == BUTTON_HOLD) show(F("hold"));
  if (click.read() == BUTTON_TAP) show(F("stick click"));

  stick.read();
  switch (dirs.update(stick.x, stick.y)) {
    case STICK_LEFT:  show(F("left"));  break;
    case STICK_RIGHT: show(F("right")); break;
    case STICK_UP:    show(F("up"));    break;
    case STICK_DOWN:  show(F("down"));  break;
    default: break;
  }

  if (now - lastDraw >= 33) {
    lastDraw = now;
    unsigned long t0 = micros();
    char buf[24];
    snprintf_P(buf, sizeof(buf), PSTR("x %4d%%  y %4d%%"), stick.x, stick.y);
    screen.text(4, 30, buf, WHITE, BLACK);
    snprintf_P(buf, sizeof(buf), PSTR("taps %3u dbl %3u"), count.taps, count.doubleTaps);
    screen.text(4, 42, buf, WHITE, BLACK);
    screen.bar(4, 56, 90, 10, stick.x + 100, 200, ACCENT, BLACK, GREY);
    screen.bar(4, 70, 90, 10, stick.y + 100, 200, ACCENT, BLACK, GREY);
    int16_t nx = 126 + stick.x / 4, ny = 56 - stick.y / 4;
    if (nx != dotX || ny != dotY) {
      screen.fillCircle(dotX, dotY, 3, BLACK);
      dotX = nx; dotY = ny;
    }
    screen.fillCircle(dotX, dotY, 3, btn.down() ? WHITE : ACCENT);
    snprintf_P(buf, sizeof(buf), PSTR("frame %3lu ms"), (micros() - t0) / 1000);
    screen.text(4, 116, buf, GREY, BLACK);
  }
}
