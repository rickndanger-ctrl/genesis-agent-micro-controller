#include "../../firmware/GenesisMicro/WorkingAnimation.h"
#include <cassert>
int main(){
for(unsigned t=0;t<6000;t+=37) {
 unsigned lit=0;
 for(unsigned y=0;y<5;++y) for(unsigned x=0;x<5;++x) {
  unsigned level=WorkingAnimation::quietLevel(x,y,t);
  if(level) {++lit;assert(y==2 && x%2==0);assert(level>=12 && level<=36);}
 }
 assert(lit==3);
}
assert(WorkingAnimation::quietLevel(2,2,0)==12);
assert(WorkingAnimation::quietLevel(2,2,1500)==36);
assert(WorkingAnimation::quietLevel(2,2,3000)==12);
WorkingAnimation a; a.update(0,true,100);assert(a.spinning(0,30099));assert(!a.spinning(0,30100));a.update(0,true,50000);assert(!a.spinning(0,50000));a.update(1,true,50000);assert(a.spinning(1,50001));a.update(0,false,50001);a.update(0,true,50002);assert(a.spinning(0,50003));WorkingAnimation b;b.update(0,true,0xfffffff0);assert(b.spinning(0,20));assert(!b.spinning(0,30000));}
