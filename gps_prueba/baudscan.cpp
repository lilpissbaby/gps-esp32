#include "baudscan.h"

BaudScanner::BaudScanner(const uint32_t* bauds, size_t count, uint32_t dwellMs)
    : bauds_(bauds), count_(count), dwellMs_(dwellMs) {}

void BaudScanner::start(uint32_t nowMs, uint32_t validSentences) {
  index_ = 0;
  sinceMs_ = nowMs;
  sentencesAtSwitch_ = validSentences;
  locked_ = false;
}

bool BaudScanner::update(uint32_t nowMs, uint32_t validSentences) {
  if (locked_) return false;
  if (validSentences > sentencesAtSwitch_) {
    locked_ = true;
    return false;
  }
  if (nowMs - sinceMs_ < dwellMs_) return false;
  index_ = (index_ + 1) % count_;
  sinceMs_ = nowMs;
  sentencesAtSwitch_ = validSentences;
  return true;
}
