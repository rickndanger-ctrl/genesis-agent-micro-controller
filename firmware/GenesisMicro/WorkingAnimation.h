#pragma once
#include <stdint.h>
struct WorkingAnimation {
  bool active[6]={};
  uint32_t started[6]={};
  void update(unsigned slot,bool working,uint32_t now) {
    if(slot>=6) return;
    if(working && !active[slot]) started[slot]=now;
    active[slot]=working;
  }
  // Three separated dots across the middle row; a gentle 3-second breath.
  static unsigned quietLevel(unsigned x,unsigned y,uint32_t now) {
    if(y!=2 || (x!=0 && x!=2 && x!=4)) return 0;
    unsigned phase=now%3000;
    unsigned ramp=phase<1500?phase:3000-phase;
    return 12+24*ramp/1500;
  }
  bool spinning(unsigned slot,uint32_t now) const {
    return slot<6 && active[slot] && uint32_t(now-started[slot])<30000;
  }
};
