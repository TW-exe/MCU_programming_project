#include "storage.h"
#define Storage_TurnA (135)         //turning towards storage area for layout A
#define Storage_TurnB (-135)        //turning towards storage area for layout B
#define Storage_Move_Dist 1000      //moving into storage area (mm)


//Storage
static enum {Start_Store,
    Store_Turn,
    Store_Move,
    Storage_Finished
} Storage_State;  //storage states



void Start_Storage(void) {
    Storage_State=Start_Store;      //start first step in storage
}



uint8_t Storage_FSM(Kobuki_Typedef *kobuki){
    switch (Storage_State) {

        case Start_Store:           //setting up rotation from where pathfind finished
            Kobuki_Drive(STOP);
            Kobuki_Rotate_Angle_Setpoint(kobuki->angle,(decision==LayoutA)?Storage_TurnA:Storage_TurnB);
            kobuki->rotation_complete=0;
            Storage_State=Store_Turn;
        break;

        case Store_Turn:            //rotate +135 (layout A) or -135 (layout B)
            if (kobuki->rotation_complete){
                Kobuki_Drive(STOP);
                Kobuki_Drive_Distance_Setpoint(kobuki->distance,Storage_Move_Dist);
                kobuki->distance_complete=0;
                Storage_State=Store_Move;
            }

            else{
                Kobuki_Rotate_Angle(kobuki->angle);
            }
        break;

        case Store_Move:            //drive forward 1000mm
            if (kobuki->distance_complete){
                Kobuki_Drive(STOP);
                Storage_State=Storage_Finished;
            } else {
                Kobuki_Drive_Distance(kobuki->distance);
            }
        break;

        case Storage_Finished:      //storage finished, stay stopped
        return 1;
        break;

        default:
        //null

        break;

    }
return 0;
}
