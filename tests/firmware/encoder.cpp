#include "../../firmware/GenesisMicro/Encoder.h"
#include <cassert>
#include <initializer_list>
int run(EncoderDecoder& e,std::initializer_list<int> states) {int n=0;for(int s:states)n+=e.update(s);return n;}
int main() {
 EncoderDecoder e;e.reset(0);
 assert(run(e,{2,3})==1);assert(run(e,{1,0})==1);
 assert(run(e,{1,3})==-1);assert(run(e,{2,0})==-1);
 e.reset(0);assert(run(e,{2,0,2,0})==0);assert(run(e,{2,3,2,3})==1);
 assert(run(e,{1,3,1,0,1,0})==1);
 e.reset(0);assert(run(e,{2,0,1,3})==-1);
 e.reset(0);assert(run(e,{3})==0);assert(run(e,{1,0})==1);
 e.reset(2);assert(run(e,{3})==0);assert(run(e,{1,0})==1);
 e.reset(0);for(int n=0;n<100;++n)assert(run(e,{2,3,1,0})==2);
}
