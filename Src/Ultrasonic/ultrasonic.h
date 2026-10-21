#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

#define ULTRASONIC_DETECT_RANGE_MM ((uint16_t)700U)
#define ULTRASONIC_MAX_RANGE_MM    ((uint16_t)5000U)

typedef struct {
    uint8_t detected;
    uint16_t range_mm;
} US_Typedef;

void Ultrasonic_Init(void);
uint16_t Ultrasonic_Read(void);
void Ultrasonic_Detect(US_Typedef *object);

#endif