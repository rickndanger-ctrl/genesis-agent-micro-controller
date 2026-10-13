#pragma once
#include <stdint.h>
struct SlotNavigation {
  uint8_t chats=0;
  uint32_t received=0;
  bool known=false;
  void update(uint8_t mask,uint32_t now) {chats=mask&63;received=now;known=true;}
  bool jump(unsigned slot,uint32_t now) const {
    return slot<6 && known && uint32_t(now-received)<6000 && (chats&(1u<<slot));
  }
};
