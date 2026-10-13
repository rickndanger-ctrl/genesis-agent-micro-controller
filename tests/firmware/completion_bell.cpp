#include "../../firmware/GenesisMicro/CompletionBell.h"
#include <cassert>
int main(){assert(CompletionBell::tone(0)==2093);assert(CompletionBell::tone(110)==0);assert(CompletionBell::tone(145)==1568);assert(CompletionBell::tone(330)==1568);assert(CompletionBell::tone(340)==0);assert(CompletionBell::tone(420)==0);assert(CompletionBell::tone(9999)==0);}
