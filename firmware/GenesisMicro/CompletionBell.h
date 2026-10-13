#pragma once
#include <stdint.h>
struct CompletionBell {
  static uint16_t tone(uint32_t elapsed) {
    if(elapsed<110) return 2093;
    if(elapsed<145) return 0;
    if(elapsed<330) return 1568;
    // Short separated tail approximates a fading bell on a piezo buzzer.
    if(elapsed<420 && (elapsed-330)%30<10) return 1568;
    return 0;
  }
};
