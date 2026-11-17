#include "servo_t1.h"
#if defined(__AVR__) && !defined(NATIVE_TEST)
#include <Arduino.h>

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
static const uint8_t PIN_A = 11, PIN_B = 12;   // OC1A, OC1B on a Mega
#else
static const uint8_t PIN_A = 9, PIN_B = 10;    // Uno, Nano, Leonardo
#endif
static const uint32_t TICKS_PER_US = F_CPU / 8 / 1000000;   // clock / 8: 2 ticks per us at 16 MHz

bool Timer1Servo::begin(uint8_t pin, uint16_t startUs) {
  channel = pin == PIN_A ? 1 : pin == PIN_B ? 2 : 0;
  if (!channel) return false;
  pinMode(pin, OUTPUT);
  static bool running = false;   // the other channel may already be in use: keep it
  if (!running) {
    TCCR1B = 0;                                     // stopped while it's set up
    TCCR1A = _BV(WGM11);
    ICR1 = 20000 * TICKS_PER_US - 1;                // 20 ms period
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11);   // fast PWM (TOP = ICR1), clock / 8: running
    running = true;
  }
  us(startUs);
  TCCR1A |= (channel == 1 ? _BV(COM1A1) : _BV(COM1B1)) | _BV(WGM11);   // pin high for OCR1x ticks
  return true;
}

void Timer1Servo::us256(int32_t pos256) {
  if (!channel) return;
  uint16_t ticks = (uint16_t)((pos256 * (int32_t)TICKS_PER_US) >> 8);
  if (channel == 1) OCR1A = ticks;   // takes effect at the next 20 ms pulse
  else OCR1B = ticks;
  current = (uint16_t)((pos256 + 128) >> 8);
}
#endif
