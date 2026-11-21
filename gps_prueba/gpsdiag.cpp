#include "gpsdiag.h"

GpsState gpsState(uint32_t charsProcessed, uint32_t validSentences, bool locationValid,
                  uint32_t msListening) {
  if (locationValid) return GpsState::Fix;
  if (validSentences > 0) return GpsState::NoFix;
  if (msListening < 5000) return GpsState::Waiting;
  return charsProcessed >= GPS_GARBAGE_CHARS ? GpsState::BadData : GpsState::NoData;
}
