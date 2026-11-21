#pragma once
#include <stddef.h>
#include <stdint.h>

// UBX es el protocolo binario de u-blox (NEO-6M, M10...). Las controladoras de dron suelen
// dejar el GPS hablando solo en UBX; este detector reconoce sus tramas en lo que llega.
class UbxDetector {
 public:
  // Mete un byte. Devuelve true si con él se completa una trama con la suma de control bien.
  bool feed(uint8_t c);
  uint32_t validFrames() const { return valid_; }

 private:
  enum class St { Sync1, Sync2, Class, Id, Len1, Len2, Payload, CkA, CkB };
  St st_ = St::Sync1;
  uint16_t len_ = 0, got_ = 0;
  uint8_t a_ = 0, b_ = 0, ckA_ = 0;
  uint32_t valid_ = 0;
  void add(uint8_t c) { a_ += c; b_ += a_; }
};

// Escribe en out la orden UBX-CFG-CFG que borra la configuración guardada en el GPS y carga
// la de fábrica (9600 baudios, frases NMEA). Devuelve los bytes escritos, o 0 si no caben.
size_t ubxFactoryReset(uint8_t* out, size_t cap);
