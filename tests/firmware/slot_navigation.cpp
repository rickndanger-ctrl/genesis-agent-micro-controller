#include "../../firmware/GenesisMicro/SlotNavigation.h"
#include <cassert>
int main() {
 SlotNavigation n;assert(!n.jump(0,0));
 n.update(15,100);assert(n.jump(0,101));assert(n.jump(3,101));
 assert(!n.jump(4,101));assert(!n.jump(5,101));assert(!n.jump(6,101));
 assert(!n.jump(0,6100));
 n.update(32,7000);assert(!n.jump(0,7000));assert(n.jump(5,7000));
 n.update(1,0xfffffff0);assert(n.jump(0,20));assert(!n.jump(0,6000));
}
