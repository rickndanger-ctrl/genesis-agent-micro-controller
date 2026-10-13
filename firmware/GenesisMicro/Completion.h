#pragma once
#include <stdint.h>

// The host supplies colors, not completion events. Require an observed
// working -> idle/unread transition; initial idle/unread is not a completion.
struct CompletionState {
  uint32_t previous[6] = {};
  uint32_t started[6] = {};
  bool flashing[6] = {};
  bool update(unsigned slot, uint32_t rgb, float brightness, uint32_t now) {
    if(slot >= 6) return false;
    if(brightness <= 0 || rgb == 0) {
      previous[slot]=0; flashing[slot]=false; return false;
    }
    bool finished=previous[slot]==0x304ffe && (rgb==0xffffff || rgb==0x00ff4c);
    if(finished) {started[slot]=now;flashing[slot]=true;}
    else if(rgb!=0xffffff && rgb!=0x00ff4c) flashing[slot]=false;
    previous[slot]=rgb;
    return finished;
  }
  // Rotate concurrent completed slots; all frames retain the slot hue.
  int animationSlot(uint32_t now) const {
    unsigned count=0;
    for(unsigned i=0;i<6;++i) if(active(i,now)) ++count;
    if(!count) return -1;
    unsigned target=(now/2400)%count;
    for(unsigned i=0;i<6;++i) if(active(i,now) && target--==0) return int(i);
    return -1;
  }
  bool active(unsigned slot, uint32_t now) const {
    return slot<6 && flashing[slot] && uint32_t(now-started[slot])<8000;
  }
};
