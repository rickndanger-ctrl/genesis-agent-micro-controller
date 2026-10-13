#include "../../firmware/GenesisMicro/KnobGesture.h"
#include <cassert>
int main() {
 KnobGesture g;
 assert(g.update(1,100)==g.None);assert(g.update(-1,200)==g.Tap);
 assert(g.update(1,300)==g.None);assert(g.update(0,949)==g.None);
 assert(g.update(0,950)==g.Back);assert(g.update(0,1100)==g.None);
 assert(g.update(-1,1200)==g.None);assert(!g.pending);
 assert(g.update(1,1300)==g.None);assert(g.update(-1,1950)==g.Back);
 assert(!g.pending);assert(g.update(-1,1960)==g.None);
 assert(g.update(1,2000)==g.None);assert(g.update(-1,2100)==g.Tap);
 assert(g.update(1,UINT32_MAX-100)==g.None);assert(g.update(0,550)==g.Back);
 assert(g.update(-1,600)==g.None);
}
