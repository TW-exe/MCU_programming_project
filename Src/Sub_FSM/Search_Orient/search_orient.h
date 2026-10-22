//a1923644
#ifndef SEARCH_ORIENT_H
#define SEARCH_ORIENT_H
#include "stdint.h"
#include "kobuki.h"
#include "ultrasonic.h"

typedef enum {LayoutA, LayoutB} Layout;

extern Layout decision; 

void Reset_Search(void);
uint8_t Search_FSM(Kobuki_Typedef *kobuki, US_Typedef *object);
void Start_Orient(void);

uint8_t Orient_FSM(Kobuki_Typedef *kobuki, US_Typedef *object);
#endif