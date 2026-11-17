#include "gpsdiag.h"

GpsState gpsState(uint32_t charsProcessed, bool locationValid, uint32_t msSinceBoot) {
  if (locationValid) return GpsState::Fix;
  if (charsProcessed > 0) return GpsState::NoFix;
  return msSinceBoot >= 5000 ? GpsState::NoData : GpsState::Waiting;
}
