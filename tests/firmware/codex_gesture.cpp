#include "../../firmware/GenesisMicro/CodexGesture.h"
#include <cassert>
int main(){
 CodexGesture g;
 assert(g.update(1,0)==g.None);assert(g.update(-1,80)==g.None);
 assert(g.update(0,380)==g.None);assert(g.update(0,381)==g.Tap);assert(!g.busy());
 assert(g.update(1,500)==g.None);assert(g.update(-1,580)==g.None);
 assert(g.update(1,700)==g.Back);assert(g.update(-1,780)==g.None);assert(!g.busy());
 assert(g.update(1,1000)==g.None);assert(g.update(0,1299)==g.None);
 assert(g.update(0,1300)==g.Down);assert(g.update(0,1500)==g.None);
 assert(g.update(-1,1700)==g.Up);assert(!g.busy());
 assert(g.update(1,2000)==g.None);assert(g.update(-1,2050)==g.None);
 assert(g.update(1,2400)==g.Tap);assert(g.update(-1,2450)==g.None);
 assert(g.update(0,2751)==g.Tap);
 assert(g.update(1,UINT32_MAX-100)==g.None);assert(g.update(0,200)==g.Down);
 assert(g.update(-1,300)==g.Up);
 // Stop dictation with a click, then immediately turn toward Send.
 assert(g.update(1,4000)==g.None);assert(g.update(-1,4050)==g.None);
 assert(g.turn()==g.Tap);assert(!g.busy());assert(g.update(0,4500)==g.None);
 // Return to chats then rotate before second release: no latched lock.
 assert(g.update(1,5000)==g.None);assert(g.update(-1,5080)==g.None);
 assert(g.update(1,5200)==g.Back);assert(g.turn()==g.None);assert(!g.busy());
 assert(g.update(-1,5300)==g.None);
 // Rotation can always recover even if a release was missed.
 assert(g.update(1,6000)==g.None);assert(g.update(0,6300)==g.Down);
 assert(g.turn()==g.Up);assert(!g.busy());assert(g.update(-1,6400)==g.None);
 assert(g.update(1,7000)==g.None);assert(g.turn()==g.None);assert(!g.busy());
}
