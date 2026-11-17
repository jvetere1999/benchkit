// An nRF24L01 / nRF24L01+ 2.4 GHz radio module (the PA+LNA version with the screw-on antenna
// too). Hardware SPI (Uno: MOSI 11, MISO 12, SCK 13; Mega: 51, 50, 52; or the ICSP header) plus
// CE and CSN on any pins. VCC on 3.3 V only: 5 V destroys it. A 10-100 uF capacitor across its
// VCC/GND fixes most flaky links. Needs the RF24 library: `nrf24/RF24` in lib_deps.
//
//   Nrf24 radio;
//   setup():  Nrf24::park(CE, CSN);            // first thing: keeps the radio off SPI meanwhile
//             radio.begin(CE, CSN, 76, 1000, 0, sizeof(Packet));   // channel, kbps, power 0-3, size
//   Sending:  radio.sender("ADDR1");  bool acked = radio.send(&p, sizeof(p));
//             radio.linkPercent()   // % of the last 20 sends the other end acknowledged
//             radio.state()         // NO_CHIP, NO_ANSWER or LINKED
//   Receiving: radio.receiver("ADDR1");  while (radio.receive(&p, sizeof(p))) { ... }
//             radio.sampleSignal() every loop, radio.signalPercent() every few seconds: how
//             much of the time something transmits on the channel (even if it can't be decoded)
//   Scanning: radio.signalOn(channel)   // listens on one channel for ~0.13 ms
//
// Both ends must agree on the channel (0-125), the speed (250 / 1000 / 2000 kbps; 1000 works on
// every chip, many clones can't do 250), the 5-character address and the payload size.
// AckHistory is the part without the radio (tested on the computer).
#pragma once
#include <stdint.h>

// Whether each of the last 20 sends was acknowledged
class AckHistory {
public:
  void add(bool acked) {
    bits = (bits << 1) | (acked ? 1 : 0);
    if (count < 20) count++;
  }
  uint8_t percent() const {
    if (!count) return 0;
    uint8_t hits = 0;
    for (uint8_t i = 0; i < count; i++) hits += (bits >> i) & 1;
    return hits * 100 / count;
  }
  uint8_t sent() const { return count; }

private:
  uint32_t bits = 0;   // newest in bit 0
  uint8_t count = 0;
};

#if defined(ARDUINO) && !defined(NATIVE_TEST) && __has_include(<RF24.h>)
#include <RF24.h>

class Nrf24 {
public:
  enum State : uint8_t {
    NO_CHIP,     // didn't answer on SPI: check its wiring and 3.3 V power
    NO_ANSWER,   // works, but nothing acknowledged the last 20 sends
    LINKED,      // the other end is acknowledging
  };

  // CSN high (not selected) and CE low (idle) until begin(). Call it first thing in setup(),
  // so a half-started radio can't disturb other SPI parts.
  static void park(uint8_t ce, uint8_t csn);

  // Returns false if the chip doesn't answer. kbps: 250, 1000 or 2000; power: 0 (min) .. 3 (max).
  // Keep power 0 on a desk: PA+LNA modules close together drown each other out.
  bool begin(uint8_t ce, uint8_t csn, uint8_t channel, uint16_t kbps = 1000, uint8_t power = 0,
             uint8_t payloadSize = 32);
  bool chipOk() const { return ok; }
  bool rateOk() const { return rate; }   // the chip accepted `kbps` (clones may refuse 250)
  bool isPlus() { return ok && rf.isPVariant(); }   // nRF24L01+ (or a clone that says so)

  // Sending: retries every (retryDelay + 1) * 250 us, up to `retries` times (0-15)
  void sender(const char *address, uint8_t retryDelay = 3, uint8_t retries = 5);
  bool send(const void *data, uint8_t size);   // true if the other end acknowledged it
  uint8_t linkPercent() const { return acks.percent(); }
  State state() const { return !ok ? NO_CHIP : acks.percent() ? LINKED : NO_ANSWER; }

  // Receiving (acknowledged automatically)
  void receiver(const char *address);
  bool receive(void *data, uint8_t size);   // copies the next packet in; false if none waiting

  // Signal on our channel (stronger than -64 dBm), sampled every 2 ms
  void sampleSignal();
  uint8_t signalPercent();   // since the last call; restarts listening (~0.2 ms deaf): only
                             // call it every few seconds, while no packets arrive
  void signalReset();        // just clears the count

  bool signalOn(uint8_t channel);   // scanning: is something transmitting there right now?

  RF24 &lib() { return rf; }   // the RF24 object, for anything not here

private:
  RF24 rf;
  bool ok = false, rate = false;
  AckHistory acks;
  uint16_t hits = 0, samples = 0;
  uint32_t lastSampleUs = 0;
};
#endif
