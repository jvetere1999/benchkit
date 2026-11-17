#include "sound.h"
#include <Arduino.h>

void Buzzer::begin(uint8_t p) { pin = p; pinMode(pin, OUTPUT); }

void Buzzer::start(uint16_t hz, uint16_t ms) {
  if (hz) tone(pin, hz); else noTone(pin);
  noteStart = millis();
  noteMs = ms;
}

void Buzzer::beep(uint16_t hz, uint16_t ms) {
  tune = nullptr;
  start(hz, ms);
}

void Buzzer::play(const Note *notes, uint8_t n) {
  tune = notes;
  count = n;
  index = 0;
  if (n) start(pgm_read_word(&notes[0].hz), pgm_read_word(&notes[0].ms));
}

void Buzzer::stop() { tune = nullptr; noteMs = 0; noTone(pin); }

void Buzzer::update() {
  if (!noteMs || millis() - noteStart < noteMs) return;
  if (tune && ++index < count) {
    start(pgm_read_word(&tune[index].hz), pgm_read_word(&tune[index].ms));
    return;
  }
  noteMs = 0;
  noTone(pin);
}

void ActiveBuzzer::begin(uint8_t p, bool low) { pin = p; activeLow = low; pinMode(pin, OUTPUT); off(); }
void ActiveBuzzer::apply() { digitalWrite(pin, pulse.isOn() != activeLow ? HIGH : LOW); }
void ActiveBuzzer::on() { pulse.on(); apply(); }
void ActiveBuzzer::off() { pulse.off(); apply(); }
void ActiveBuzzer::beep(uint16_t ms) { pulse.flash(ms, millis()); apply(); }
void ActiveBuzzer::beeps(uint8_t times, uint16_t onMs, uint16_t offMs) { pulse.blink(onMs, offMs, times, millis()); apply(); }
void ActiveBuzzer::pattern(uint16_t onMs, uint16_t offMs) { pulse.blink(onMs, offMs, 0, millis()); apply(); }
void ActiveBuzzer::update() {
  if (!pulse.busy()) return;
  bool was = pulse.isOn();
  pulse.update(millis());
  if (pulse.isOn() != was) apply();
}
