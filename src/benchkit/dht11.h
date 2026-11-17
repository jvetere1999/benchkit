// DHT11 temperature and humidity (the 3-pin module: S / + / -, pull-up already on the board):
//
//   Dht11 dht;  dht.begin(DHT_PIN);
//   if (dht.read()) { dht.tempC(); dht.humidity(); }   // read() as often as you like
//
// The chip can only measure once a second, so read() talks to it at most that often and
// otherwise returns the last result. A read takes ~25 ms with interrupts off for ~5 ms of it
// (serial input arriving then can be lost). Accuracy: ±2 °C, ±5 % humidity, whole numbers.
#pragma once
#include <stdint.h>

class Dht11 {
public:
  enum Status : uint8_t { OK, NO_REPLY, TIMEOUT, BAD_CHECKSUM, NOT_YET };

  void begin(uint8_t pin);
  bool read();                   // true if tempC() / humidity() hold a good reading
  int8_t tempC() const { return t; }
  uint8_t humidity() const { return rh; }
  Status status() const { return last; }   // what the latest attempt did

  // The 5 bytes the chip sends -> values; false if the checksum is wrong
  static bool decode(const uint8_t b[5], int8_t &tempC, uint8_t &humidity);

private:
  uint8_t pin = 255, rh = 0;
  int8_t t = 0;
  bool good = false;
  Status last = NOT_YET;
  uint32_t lastMs = 0;
  bool started = false;
  Status transfer(uint8_t b[5]);
};
