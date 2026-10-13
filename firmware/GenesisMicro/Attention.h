#pragma once
#include <stdint.h>

// Interoperability facts from this installed Codex build; no semantic status
// field is supplied over USB. Unknown colors are displayed without alerting.
inline bool needsAttention(uint32_t rgb) {
  return rgb == 0xff6d00 || rgb == 0xff0033;
}
struct AttentionState {
  uint32_t lastVisible[6] = {};
  bool update(unsigned slot, uint32_t rgb, float brightness) {
    if (slot >= 6 || rgb == 0 || brightness <= 0) return false;
    bool fresh = needsAttention(rgb) && lastVisible[slot] != rgb;
    lastVisible[slot] = rgb;
    return fresh;
  }
};
