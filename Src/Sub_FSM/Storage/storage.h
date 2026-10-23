#ifndef STORAGE_H
#define STORAGE_H
#include <stdint.h>
#include "kobuki.h"

void Start_Storage(void);
uint8_t Storage_FSM(Kobuki_Typedef *kobuki);

#endif
