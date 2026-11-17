#include "debouncer.h"

Debouncer::Debouncer(uint32_t debounceMs) : debounceMs_(debounceMs) {}

bool Debouncer::update(bool rawPressed, uint32_t nowMs) {
  if (rawPressed != lastRaw_) {   // cambio: empieza a contar de nuevo
    lastRaw_ = rawPressed;
    changedAt_ = nowMs;
    return false;
  }
  if (rawPressed == stable_ || nowMs - changedAt_ < debounceMs_) return false;
  stable_ = rawPressed;           // lleva debounceMs_ sin cambiar: es estable
  return stable_;                 // evento solo al quedar pulsado
}

bool Debouncer::isPressed() const { return stable_; }
