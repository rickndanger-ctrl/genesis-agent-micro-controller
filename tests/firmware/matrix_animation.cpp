#include "../../firmware/GenesisMicro/MatrixAnimation.h"
#include <cassert>
int main() {
 MatrixAnimation s;
 assert(!s.asleep(59999));assert(s.asleep(60000));
 s.wake(62000);assert(!s.asleep(62000));assert(s.asleep(122000));
 s.wake(0xfffffff0);assert(!s.asleep(20));assert(s.asleep(60000));
 assert(MatrixAnimation::dim(0x3478f6,255)==0x3478f6);
 assert(MatrixAnimation::dim(0x3478f6,0)==0);
 assert(MatrixAnimation::replyLevel(2,2,0)==255);
 assert(MatrixAnimation::replyLevel(0,0,1600)==255);
 assert(MatrixAnimation::replyLevel(2,2,400)==120);
 for(unsigned phase=0;phase<16;++phase) {
   unsigned heads=0;
   for(unsigned y=0;y<5;++y) for(unsigned x=0;x<5;++x)
     if(MatrixAnimation::idleLevel(x,y,phase*180)==160) ++heads;
   assert(heads==1);
 }
 assert(MatrixAnimation::idleLevel(2,2,0)==18);
 assert(MatrixAnimation::idleLevel(2,2,1200)==68);
}
