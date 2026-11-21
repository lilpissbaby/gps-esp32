#pragma once
#include <stddef.h>
#include <stdint.h>

// Prueba velocidades (baudios) del GPS hasta que llega una frase NMEA válida y se queda
// con esa. Sirve para el NEO-6M (de fábrica a 9600, pero alguien pudo cambiarla) y para
// el M10, que viene a otra velocidad.
class BaudScanner {
 public:
  BaudScanner(const uint32_t* bauds, size_t count, uint32_t dwellMs);
  // Empieza a escuchar en la primera velocidad. validSentences: frases ya contadas antes.
  void start(uint32_t nowMs, uint32_t validSentences = 0);
  // Llamar a menudo con el total de frases válidas. Devuelve true si hay que cambiar de
  // velocidad (entonces baud() ya es la nueva).
  bool update(uint32_t nowMs, uint32_t validSentences);
  uint32_t baud() const { return bauds_[index_]; }
  bool locked() const { return locked_; }

 private:
  const uint32_t* bauds_;
  size_t count_;
  uint32_t dwellMs_;
  size_t index_ = 0;
  uint32_t sinceMs_ = 0;
  uint32_t sentencesAtSwitch_ = 0;
  bool locked_ = false;
};
