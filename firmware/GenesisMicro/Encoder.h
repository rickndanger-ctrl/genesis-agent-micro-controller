#pragma once
#include <stdint.h>
// Emit at stable quadrature endpoints, never in the middle of a detent.
struct EncoderDecoder {
  uint8_t previous=0, origin=0;
  int8_t travel=0;
  bool anchored=false;
  void reset(uint8_t state) {previous=state;origin=state;travel=0;anchored=(state==0 || state==3);}
  int update(uint8_t state) {
    static const int8_t delta[16]={0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    if(state==previous)return 0;
    if((state^previous)==3) {reset(state);return 0;}
    int step=delta[(previous<<2)|state];previous=state;
    if(!anchored) {if(state==0 || state==3)reset(state);return 0;}
    travel+=step;
    if(state!=0 && state!=3)return 0;
    int result=(state!=origin && travel==2)?1:(state!=origin && travel==-2)?-1:0;
    origin=state;travel=0;
    return result;
  }
};
