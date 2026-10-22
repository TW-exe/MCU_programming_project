//a1923644
#ifndef Pathfind
#define Pathfind
#include <stdint.h>
#include "kobuki.h"
#include "search_orient.h"
void Start_Pathfind(void);
uint8_t Pathfind_FSM(Kobuki_Typedef *kobuki);
#endif