#include "../../firmware/GenesisMicro/Attention.h"
#include <cassert>
int main() {
  AttentionState s;
  assert(!s.update(0,0x304ffe,1));
  assert(s.update(0,0xff6d00,1));
  assert(!s.update(0,0xff6d00,1));
  assert(!s.update(0,0xff6d00,0.3f));
  assert(!s.update(0,0,0)); // App sleep must not retrigger the same alert.
  assert(!s.update(0,0xff6d00,1));
  assert(s.update(1,0xff6d00,1)); // Other chat still independently alerts.
  assert(s.update(0,0xff0033,1));
  assert(!s.update(0,0xffffff,1));
  assert(s.update(0,0xff6d00,1)); // A genuinely new wait alerts again.
  assert(!s.update(6,0xff6d00,1));
  assert(!s.update(2,0x123456,1));
  assert(!s.update(3,0xff6d00,0));
  assert(s.update(3,0xff6d00,1));
}
