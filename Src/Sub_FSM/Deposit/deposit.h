#ifndef DEPOSIT_H
#define DEPOSIT_H
#include <stdint.h>
#include "kobuki.h"
#include "ultrasonic.h"

void Start_Deposit(void);
uint8_t Deposit_FSM(Kobuki_Typedef *kobuki, US_Typedef *object);  //returns 1 once deposit complete, otherwise 0

#endif
