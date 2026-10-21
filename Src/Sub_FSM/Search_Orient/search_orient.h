//a1923644
#ifndef Search_Orient
#define Search_Orient

#include "stdint.h"
#include "kobuki.h"
#include <stdlib.h>
#include "ultrasonic.h"

typedef enum {
  LayoutA,
  LayoutB
} Layout;

void Reset_Search(void);
uint8_t Search_FSM(Kobuki_Typedef *kobuki, US_Typedef *object);
Layout Search_Layout(void);
void Start_Orient(Layout decision);
uint8_t Orient_FSM(Kobuki_Typedef *kobuki, US_Typedef *object);

#endif