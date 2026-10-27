#include <stdlib.h>
#include "deposit.h"

#define Deposit_Move_Dist 50                   //distance to drive forward once aligned (mm)
#define Object_Dist 300                         //expected distance of deposit object (mm)
#define Dist_Error 50                           //error for each side
#define Object_Sensed (Object_Dist+Dist_Error)  //closer than this object sensed (500mm)
#define N_Object_Sensed (Object_Sensed+100)     //further than this object not sensed (600mm)
#define Full_Rotation 36000                     //360 degrees in kobuki angle ticks


//Deposit
static enum {Start_Dep, Dep_Detect, Dep_Left, Dep_Right, Dep_Center, Dep_Move, Deposit_Finished} Deposit_State;  //deposit states
static int32_t theta_Left, theta_Right;     //object edge angles
static uint8_t seen;                        //object is seen

void Start_Deposit(void) {
    Deposit_State=Start_Dep;                //start first step in deposit
}

uint8_t Deposit_FSM(Kobuki_Typedef *kobuki, US_Typedef *object){
    uint16_t dist=object->range_mm;         //give distance reading
    int32_t current=kobuki->angle;          //give current direction

    switch (Deposit_State) {

        case Start_Dep:                     //start rotating from where storage finished
            Kobuki_Rotate(SEARCH_SPEED);
            Deposit_State=Dep_Detect;
        break;

        case Dep_Detect:                                    //rotate until object at 450mm +-50mm
            if (abs(dist-Object_Dist)<=Dist_Error){
                Kobuki_Drive(STOP);                         //stop rotating
                Deposit_State=Dep_Left;                     //start aligning
            } else {
                Kobuki_Rotate(SEARCH_SPEED);                //keep rotating
            }
        break;

        case Dep_Left:                          //align left side
            if (dist>N_Object_Sensed){          //if object not sensed
                theta_Left=current;             //set left angle
                seen=0;                         //reset for right side
                Deposit_State=Dep_Right;        //start align right side
            } else {
                Kobuki_Rotate(MIN_SPEED);       //keep rotating left
            }
        break;

        case Dep_Right:                                         //align right side
            if (!seen){
                if (dist<=Object_Sensed){
                    seen=1;                                     //object sensed again
                }
            }

            else if (dist>N_Object_Sensed){                     //find right edge
                theta_Right=current;                            //set right angle
                Kobuki_Drive(STOP);                             //stop rotating
                int32_t width=theta_Left-theta_Right;           //object width (ticks)
                if (width<0) width+=Full_Rotation;              //handle wrap at +-180 degrees
                Kobuki_Rotate_Angle_Setpoint(kobuki->angle,(int16_t)(width/200));  //rotate back half the width to center
                kobuki->rotation_complete=0;
                Deposit_State=Dep_Center;                       //start align center
                break;
            }

            Kobuki_Rotate(-MIN_SPEED);                          //keep rotating right
        break;

        case Dep_Center:                                        //rotate to center of object
            if (kobuki->rotation_complete){
                Kobuki_Drive(STOP);
                Kobuki_Drive_Distance_Setpoint(kobuki->distance,Deposit_Move_Dist);
                kobuki->distance_complete=0;
                Deposit_State=Dep_Move;
            } else {
                Kobuki_Rotate_Angle(kobuki->angle);
            }
        break;

        case Dep_Move:                                          //drive forward set distance
            if (kobuki->distance_complete){
                Kobuki_Drive(STOP);
                Deposit_State=Deposit_Finished;
            } else {
                Kobuki_Drive_Distance(kobuki->distance);
            }
        break;

        case Deposit_Finished:          //deposit finished
        return 1;
        break;

        default:
        //null

        break;

    }
return 0;
}
