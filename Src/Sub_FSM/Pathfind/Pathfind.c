#include "pathfind.h"
#define First_MoveA 1200        //moving to section 4 for layout A
#define Second_MoveA 1350       //moving to section 3 for layout A
#define First_MoveB 1400        //moving to section 2 for layout B
#define Second_MoveB 1150       //moving to section 3 for layout B
#define ATurn (90)              //turning towards section 3 for layout A
#define BTurn (-90)             //turning towards section 3 for layout B


//Pathfind
static enum {
    Start_Path,
    First_Move,
    First_Turn,
    Second_Move,
    Pathfind_Finished
} Pathfind_State;  //pathfind states


void Start_Pathfind(void) {
    Pathfind_State=Start_Path;      //start first step in pathfind
}


uint8_t Pathfind_FSM(Kobuki_Typedef *kobuki){
    switch (Pathfind_State) {

        case Start_Path:            //setting it up 
            Kobuki_Drive_Distance_Setpoint(kobuki->distance, (decision==LayoutA)?First_MoveA:First_MoveB); // understand what the following code does
            kobuki->distance_complete=0;
            Pathfind_State=First_Move;
        break;

        case First_Move:
            if (kobuki->distance_complete){
                Kobuki_Drive(STOP);
                Kobuki_Rotate_Angle_Setpoint(kobuki->angle,(decision==LayoutA)?ATurn:BTurn);
                kobuki->rotation_complete=0;
                Pathfind_State=First_Turn;
            }
        
            else{
                Kobuki_Drive_Distance(kobuki->distance);
            }

        break;

        case First_Turn:
            if (kobuki->rotation_complete){
                Kobuki_Drive(STOP);
                Kobuki_Drive_Distance_Setpoint(kobuki->distance,(decision==LayoutA)?Second_MoveA:Second_MoveB);
                kobuki->distance_complete=0;
                Pathfind_State=Second_Move;
            } 
        
            else{
                Kobuki_Rotate_Angle(kobuki->angle);
            }
        break;

        case Second_Move:
            if (kobuki->distance_complete){
                Kobuki_Drive(STOP);
                Pathfind_State=Pathfind_Finished;
            } else {
                Kobuki_Drive_Distance(kobuki->distance);
            }
        break;

        case Pathfind_Finished:
        return 1;
        break;

        default:
        //null

        break;

    }
return 0;
}

