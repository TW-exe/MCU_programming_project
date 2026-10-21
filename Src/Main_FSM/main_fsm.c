#include "main_fsm.h"



void system_setup(void){
  // // Both of these only for testing purposes. Not required 
  //   volatile uint16_t ultrasonic_read = 0; // Zeroed variables to ensure defined behaviour
  //   volatile uint16_t adc_read = 0; // Zeroed variables to ensure defined behaviour
    
  //   Kobuki_Typedef kobuki = {0}; // Zeroed structs to ensure defined behaviour 
  //   US_Typedef object = {0}; // Zeroed structs to ensure defined behaviour
  //   Kobuki_LED_Enum led_1, led_2;
    
  //   UART_Init();
  //   ADC_Init(); // Has to be executed before Ultrasonic Init, because Ultrasonic subroutine uses ADC subroutine
  //   Ultrasonic_Init();
    
  //   //Initial assigning of all ENUMS
  //   state = IDLE;
  //   led_1 = OFF;
  //   led_2 = OFF;

}

void realtime_operation(void){
  // Kobuki_Read(&kobuki);
  // Ultrasonic_Detect(&object);
  
  // // Both of these only for testing purposes. Not required
  // ultrasonic_read = Ultrasonic_Read();
  // adc_read = ADC_Read(ADC_CHANNEL_1);

  // Kobuki_Update_LEDs(led_1, led_2);

  // //Flag Configuration
}

void FSM_Functionality(State_Machine_Flags_t *state_machine_flags){
  // Functionality FSM
        switch (state) {
            case IDLE:
            break;
                
            case SEARCH:
            break;
                
            case ORIENT:
            break;
                
            case PATHFIND:
            break;
                
            case STORAGE:
            break;

            case DEPOSIT:
            break;
            
            default:
            break;
        }
}

void FSM_Transition(State_Machine_Flags_t *state_machine_flags){
  // Transition FSM
        switch (state) {
            case IDLE:

            break;
                
            case SEARCH:
            break;
                
            case ORIENT:
            break;
                
            case PATHFIND:
            break;
                
            case STORAGE:
            break;
            
            case DEPOSIT:
            break;
            
            default:
            break;
        }
}