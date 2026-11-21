#include "gpsdiag.h"

GpsState gpsState(uint32_t charsPerSecond, uint32_t validSentences, uint32_t ubxFrames,
                  bool locationValid, uint32_t msListening) {
  if (locationValid) return GpsState::Fix;
  if (validSentences > 0) return GpsState::NoFix;
  if (ubxFrames > 0) return GpsState::UbxOnly;
  if (msListening < 5000) return GpsState::Waiting;
  return charsPerSecond >= GPS_SIGNAL_CPS ? GpsState::BadData : GpsState::NoData;
}
