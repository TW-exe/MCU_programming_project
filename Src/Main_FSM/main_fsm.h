//a1887586
//a1923644
#ifndef MAIN_FSM_H
#define MAIN_FSM_H

//Standard C libraries
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

//Application C libraries
#define STM32G474xx

#include "stm32g4xx.h"
#include "adc.h"
#include "kobuki.h"
#include "uart.h"
#include "ultrasonic.h"

// Macros
/* Kobuki button values */
#define BUTTON_0_START  0x01U
#define BUTTON_1_STOP   0x02U
#define BUTTON_2_ROTATE 0x04U

// Logic
#define NULL 0
#define TRUE 1


// States
static enum {
    IDLE,
    SEARCH,
    ORIENT,
    PATHFIND,
    STORAGE,
    DEPOSIT
} state;


typedef enum{
  ORIENT_OPERATIONAL,
  ORIENT_SUCCESS,
  ORIENT_FAIL
}orient_status_t;


typedef struct {
    bool system_start;
    bool system_stop;
    bool wheel_drop;
    bool search_done;
    orient_status_t orient_status;
    bool pathfind_complete;
    bool storage_complete;
    bool deposit_complete;
} State_Machine_Flags_t;


void system_setup(void);

void realtime_operation(void);

void FSM_Functionality(State_Machine_Flags_t *state_machine_flags);

void FSM_Transition(State_Machine_Flags_t *state_machine_flags);


#endif

