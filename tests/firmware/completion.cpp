#include "../../firmware/GenesisMicro/Completion.h"
#include <cassert>
int main() {
 CompletionState s;
 assert(!s.update(0,0xffffff,1,0));
 assert(!s.update(0,0x00ff4c,1,1));
 assert(!s.update(0,0x304ffe,1,2));
 assert(s.update(0,0xffffff,1,3));
 assert(s.active(0,8002)); assert(!s.active(0,8003));
 assert(!s.update(0,0xffffff,1,8004)); assert(!s.active(0,8004));
 s.update(0,0x304ffe,1,9000); assert(s.update(0,0x00ff4c,1,9001));
 s.update(0,0xff6d00,1,9002); assert(!s.active(0,9002));
 s.update(0,0x304ffe,1,9003);s.update(0,0xff0033,1,9004);
 assert(!s.update(0,0xffffff,1,9005));
 s.update(0,0x304ffe,1,9006);s.update(0,0,0,9007);
 assert(!s.update(0,0xffffff,1,9008));
 s.update(1,0x304ffe,1,0xfffffff0);assert(s.update(1,0xffffff,1,0xfffffff1));
 assert(s.active(1,20)); assert(!s.active(0,20));
 s.update(1,0x304ffe,1,21);assert(!s.active(1,21));
 assert(!s.update(6,0xffffff,1,30));
 CompletionState pulses;
 assert(pulses.animationSlot(0)==-1);
 pulses.update(0,0x304ffe,1,0);pulses.update(0,0xffffff,1,1);
 assert(pulses.animationSlot(100)==0);assert(pulses.animationSlot(500)==0);
 pulses.update(2,0x304ffe,1,1);pulses.update(2,0x00ff4c,1,2);
 assert(pulses.animationSlot(2400)==2);assert(pulses.animationSlot(4800)==0);
 assert(pulses.animationSlot(8800)==-1);
 // Each new reply can complete again without treating steady idle as another reply.
 pulses.update(0,0x304ffe,1,9000);assert(pulses.update(0,0xffffff,1,9001));
 assert(!pulses.update(0,0xffffff,1,9002));assert(pulses.animationSlot(9600)==0);
}
