#pragma once
#include <stdint.h>
struct KnobGesture {
 enum Event {None,Tap,Back};
 bool pending=false,fired=false;
 uint32_t started=0;
 Event update(int edge,uint32_t now) {
  if(edge==1) {pending=true;fired=false;started=now;}
  if(pending && !fired && uint32_t(now-started)>=650) {
   fired=true;
   if(edge==-1)pending=false;
   return Back;
  }
  if(edge==-1 && pending) {pending=false;return fired?None:Tap;}
  return None;
 }
};
