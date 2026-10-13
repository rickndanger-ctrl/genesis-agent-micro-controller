#pragma once
#include <stdint.h>
// Reserve two quick clicks for navigation before dispatching any host action.
struct CodexGesture {
 enum Event {None,Down,Up,Tap,Back};
 enum State {Idle,Pressed,Waiting,Held,SwallowRelease};
 State state=Idle;
 uint32_t changed=0;
 bool busy() const {return state!=Idle;}
 // Turning resolves a released click before changing its target, or cancels
 // a held/back gesture. A gesture must never latch rotation off.
 Event turn() {
  Event event=state==Waiting?Tap:state==Held?Up:None;
  state=Idle;return event;
 }
 Event update(int edge,uint32_t now) {
  switch(state) {
   case Idle: if(edge==1){state=Pressed;changed=now;} break;
   case Pressed:
    if(edge==-1){state=Waiting;changed=now;}
    else if(uint32_t(now-changed)>=300){state=Held;return Down;}
    break;
   case Waiting:
    if(uint32_t(now-changed)>300) {
     state=edge==1?Pressed:Idle;changed=now;return Tap;
    }
    if(edge==1){state=SwallowRelease;return Back;}
    break;
   case Held: if(edge==-1){state=Idle;return Up;} break;
   case SwallowRelease: if(edge==-1)state=Idle;break;
  }
  return None;
 }
};
