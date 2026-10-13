#pragma once
#include <stdint.h>

struct MatrixAnimation {
  uint32_t lastActivity=0;
  void wake(uint32_t now) {lastActivity=now;}
  bool asleep(uint32_t now) const {return uint32_t(now-lastActivity)>=60000;}
  static uint32_t dim(uint32_t rgb, unsigned level) {
    return ((((rgb>>16)&255)*level/255)<<16) |
           ((((rgb>>8)&255)*level/255)<<8) | ((rgb&255)*level/255);
  }
  // Three outward diamond ripples per 8-second completion, in one identity hue.
  static unsigned replyLevel(unsigned x,unsigned y,uint32_t elapsed) {
    int dx=int(x)-2,dy=int(y)-2;
    unsigned distance=unsigned(dx<0?-dx:dx)+unsigned(dy<0?-dy:dy);
    unsigned ring=(elapsed%2400)/400;
    // Sparkles fill the gaps between expanding rings without introducing other hues.
    unsigned sparkle=((elapsed/160)*7)%25;
    if(y*5+x==sparkle) return 220;
    return distance==ring ? 255 : (ring>0 && distance==ring-1 ? 120 : 14);
  }
  // A quiet comet orbits the edge, slowing visually without changing hue.
  static unsigned idleLevel(unsigned x,unsigned y,uint32_t elapsed) {
    int position=-1;
    if(y==0) position=int(x);
    else if(x==4) position=4+int(y);
    else if(y==4) position=12-int(x);
    else if(x==0) position=16-int(y);
    if(position<0) {
      if(x!=2 || y!=2) return 0;
      unsigned phase=(elapsed%2400)/12;
      return 18+(phase<100?phase:200-phase)/2;
    }
    unsigned age=((elapsed/180)%16+16-unsigned(position))%16;
    return age==0?160:age==1?95:age==2?45:age==3?18:0;
  }
};
