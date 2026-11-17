#pragma once
#include <stdint.h>

// Antirrebote de un botón. update() devuelve true una sola vez por pulsación,
// cuando el botón lleva debounceMs pulsado sin rebotar. Mantenerlo no repite.
class Debouncer {
 public:
  explicit Debouncer(uint32_t debounceMs = 30);
  bool update(bool rawPressed, uint32_t nowMs);
  bool isPressed() const;

 private:
  uint32_t debounceMs_;
  bool lastRaw_ = false;
  bool stable_ = false;
  uint32_t changedAt_ = 0;
};
