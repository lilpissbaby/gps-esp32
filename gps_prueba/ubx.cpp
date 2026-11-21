#include "ubx.h"

static const uint16_t kMaxPayload = 1024;  // más largo que esto es basura, no una trama

bool UbxDetector::feed(uint8_t c) {
  switch (st_) {
    case St::Sync1:
      if (c == 0xB5) st_ = St::Sync2;
      return false;
    case St::Sync2:
      st_ = c == 0x62 ? St::Class : (c == 0xB5 ? St::Sync2 : St::Sync1);
      a_ = b_ = 0;
      return false;
    case St::Class: add(c); st_ = St::Id; return false;
    case St::Id: add(c); st_ = St::Len1; return false;
    case St::Len1: add(c); len_ = c; st_ = St::Len2; return false;
    case St::Len2:
      add(c);
      len_ |= (uint16_t)c << 8;
      got_ = 0;
      st_ = len_ > kMaxPayload ? St::Sync1 : (len_ == 0 ? St::CkA : St::Payload);
      return false;
    case St::Payload:
      add(c);
      if (++got_ == len_) st_ = St::CkA;
      return false;
    case St::CkA: ckA_ = c; st_ = St::CkB; return false;
    case St::CkB:
      st_ = St::Sync1;
      if (ckA_ == a_ && c == b_) { valid_++; return true; }
      return false;
  }
  return false;
}

size_t ubxFactoryReset(uint8_t* out, size_t cap) {
  // clearMask = todo, saveMask = nada, loadMask = todo, deviceMask = BBR|Flash|EEPROM|SPI flash
  static const uint8_t body[] = {0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x17};
  const size_t n = 2 + sizeof(body) + 2;
  if (cap < n) return 0;
  uint8_t a = 0, b = 0;
  out[0] = 0xB5;
  out[1] = 0x62;
  for (size_t i = 0; i < sizeof(body); i++) {
    out[2 + i] = body[i];
    a += body[i];
    b += a;
  }
  out[n - 2] = a;
  out[n - 1] = b;
  return n;
}
