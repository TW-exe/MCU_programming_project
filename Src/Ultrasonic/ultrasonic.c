#include "stm32g474xx.h"
#include <stdint.h>
#define STM32G474xx
#include "stm32g4xx.h"
#include "adc.h"
#include "ultrasonic.h"

#define ULTRASONIC_CHANNEL      ADC_CHANNEL_1
#define ADC_MAX_VALUE           4095UL
#define MB1043_FULL_SCALE_MM    5120UL

void Ultrasonic_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    GPIOA->MODER |= GPIO_MODER_MODE0;
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;
}

uint16_t Ultrasonic_Read(void)
{
    uint16_t adc_value = ADC_Read(ULTRASONIC_CHANNEL);
    uint32_t distance_mm =
        ((uint32_t)adc_value * MB1043_FULL_SCALE_MM +
        (ADC_MAX_VALUE / 2UL)) / ADC_MAX_VALUE;

    if (distance_mm > ULTRASONIC_MAX_RANGE_MM)
        distance_mm = ULTRASONIC_MAX_RANGE_MM;

    return (uint16_t)distance_mm;
}

void Ultrasonic_Detect(US_Typedef *object)
{
    if (object == 0)
        return;

    object->range_mm = Ultrasonic_Read();
    object->detected =
        (object->range_mm < ULTRASONIC_DETECT_RANGE_MM)
        ? 1U : 0U;
}