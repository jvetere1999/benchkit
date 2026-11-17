// Sound without delay().
//
// Buzzer: a passive buzzer, piezo or 8 Ω speaker, played with tone() (any pin):
//   Buzzer bz;  bz.begin(BUZZER_PIN);
//   bz.beep();                     // 2 kHz for 80 ms
//   bz.beep(NOTE_A5, 200);
//   bz.play(TUNE, TUNE_LEN);       // notes in PROGMEM: const Note TUNE[] PROGMEM = {{NOTE_C5, 150}, {0, 50}, ...}
//   bz.update();                   // every loop, while a tune plays
// The speaker: one leg through a 100 Ω resistor to the pin, the other to GND (or drive it
// through a transistor for more volume); never straight onto a pin.
// tone() uses Timer2 on an Uno / Mega: while a sound plays, PWM on pins 3 and 11 (Uno) or
// 9 and 10 (Mega) stops working.
//
// ActiveBuzzer: has its own oscillator (sealed, a sticker on top, louder): on/off only.
//   ActiveBuzzer ab;  ab.begin(PIN);  ab.beep(100);  ab.beeps(3);  ab.update();
#pragma once
#include <stdint.h>
#include "pulse.h"

struct Note { uint16_t hz; uint16_t ms; };   // hz 0 = silence

// A few notes (Hz). Each octave up doubles.
enum : uint16_t {
  NOTE_C4 = 262, NOTE_D4 = 294, NOTE_E4 = 330, NOTE_F4 = 349, NOTE_G4 = 392, NOTE_A4 = 440, NOTE_B4 = 494,
  NOTE_C5 = 523, NOTE_D5 = 587, NOTE_E5 = 659, NOTE_F5 = 698, NOTE_G5 = 784, NOTE_A5 = 880, NOTE_B5 = 988,
  NOTE_C6 = 1047, NOTE_E6 = 1319, NOTE_G6 = 1568,
};

class Buzzer {
public:
  void begin(uint8_t pin);
  void beep(uint16_t hz = 2000, uint16_t ms = 80);
  void play(const Note *notes, uint8_t count);   // notes in PROGMEM
  void stop();
  void update();                                 // every loop
  bool playing() const { return tune && index < count; }
private:
  uint8_t pin = 255, index = 0, count = 0;
  const Note *tune = nullptr;
  uint32_t noteStart = 0;
  uint16_t noteMs = 0;
  void start(uint16_t hz, uint16_t ms);
};

class ActiveBuzzer {
public:
  void begin(uint8_t pin, bool activeLow = false);
  void on();
  void off();
  void beep(uint16_t ms = 80);
  void beeps(uint8_t times, uint16_t onMs = 80, uint16_t offMs = 80);
  void pattern(uint16_t onMs, uint16_t offMs);   // forever, until off()
  void update();
  bool busy() const { return pulse.busy(); }
private:
  Pulse pulse;
  uint8_t pin = 255;
  bool activeLow = false;
  void apply();
};
