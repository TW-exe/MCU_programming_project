//a1923644
#include <stdlib.h>
#include "search_orient.h"
#define Bollard_Sensed 420      //closer then bollard sensed
#define N_Bollard_Sensed 550    //futher then bollard not sensed
#define Dist_LayoutA 350        //expected distance of bollard for layout A
#define Dist_LayoutB 200        //expected distance of bollard for layout B
#define Dist_Error 50           //error for each side
#define RotateA (-90)           //rotation for layout A
#define RotateB (90)            //rotation for layout B

//SEARCH
static enum {Search_Rotate, Keep_Searching, Search_Finished} Search_State;  //search states
static Layout decision;                                                     //layout decision                   
void Reset_Search(void){
    Search_State = Search_Rotate;                                           //start rotating
}

uint8_t Search_FSM(Kobuki_Typedef *kobuki, US_Typedef *object){
    uint16_t dist=object->range_mm;                                            //give distance reading
    switch (Search_State){
        case Search_Rotate:
            if (dist<=Bollard_Sensed){                                      //if bollard sensed
                if (abs(dist - Dist_LayoutB)<=Dist_Error){                  //layout B?
                    Kobuki_Drive(STOP);                                     //stop rotating
                    decision = LayoutB;                                     //layout B
                    Search_State = Search_Finished;                         //search finished
                }   else if (abs(dist - Dist_LayoutA)<=Dist_Error){         //layout A?
                    Kobuki_Drive(STOP);                                     //stop rotating
                    decision = LayoutA;                                     //layout A
                    Search_State = Search_Finished;                         //search finished
                }   else {
                    Search_State = Keep_Searching;                          //keep searching
                    Kobuki_Rotate(SEARCH_SPEED);                            //keep rotating
            }
        } else {
            Kobuki_Rotate(SEARCH_SPEED);                                    //keep rotating
        }
        break;

    case Keep_Searching:                    //wrong object sensed, keep searching
        if (dist>N_Bollard_Sensed){         //if bollard not sensed
            Search_State = Search_Rotate;   //start rotating
        }
        Kobuki_Rotate(SEARCH_SPEED);        //keep rotating
        break;

    case Search_Finished:       //finishing search, do nothing
        return 1;               //search finished
    }
    return 0;                   //search not finished
}    

//ORIENT
static enum {Orient_Left, Orient_Right, Orient_Center, Orient_Turn, Orient_Finished} Orient_State;  //orient states
static int32_t theta_Left, theta_Right;                                                             //orient angles
static uint8_t seen;                                                                                //bollard is seen

void Start_Orient(void) {
    Orient_State = Orient_Left;                                     //start with left side
}
uint8_t Orient_FSM(Kobuki_Typedef *kobuki, US_Typedef *object){
    uint16_t dist=object->range_mm;                                    //give distance reading
    int32_t current=kobuki->angle;                                  //give current direction
    switch (Orient_State){
        case Orient_Left:                                           //orient left side
            if (dist>N_Bollard_Sensed){                             //if bollard not sensed
                theta_Left=current;                                 //set left angle
                seen=0;                                             //reset for right side
                Orient_State=Orient_Right;                          //start orient right side
            } else {
                Kobuki_Rotate(MIN_SPEED);                           //start rotating left
            }
            break;

        case Orient_Right:                                                              //orient right side
        if (!seen){                            
            if (dist<=Bollard_Sensed) seen=1;                                           //if bollard sensed
        } else if (dist>N_Bollard_Sensed){                                              //find right edge
                theta_Right=current;                                                    //set right angle
                Kobuki_Drive(STOP);                                                     //stop rotating
                int32_t theta_Center=(theta_Left+theta_Right)/2;                        //calculate center angle
                Kobuki_Rotate_Angle_Setpoint(kobuki->angle,(theta_Center-current)/100   ); //rotate to center
                kobuki->rotation_complete=0;                                            
                Orient_State=Orient_Center;                                             //start orient center
                break;
            } 
            Kobuki_Rotate(-MIN_SPEED);                                                  //keep rotating right
            break;
        
        case Orient_Center:                                                                     //orient center
        if (kobuki->rotation_complete){                                                         //if rotation complete
            Kobuki_Drive(STOP);                                                                 //stop rotating
            Kobuki_Rotate_Angle_Setpoint(kobuki->angle,(decision==LayoutA)?RotateA:RotateB);    //rotate to either layouts open area
            kobuki->rotation_complete=0;                                                        //reset rotation complete
            Orient_State=Orient_Turn;                                                           //start orient turn
        } else {
            Kobuki_Rotate_Angle(kobuki->angle);
        }
        break;

        case Orient_Turn:                       //orient to open area
            if (kobuki->rotation_complete){     //if rotation complete
                Kobuki_Drive(STOP);             //stop rotating
                Orient_State=Orient_Finished;   //orient finished
            } else {
                Kobuki_Rotate_Angle(kobuki->angle);
            }
            break;

        case Orient_Finished:                   //orient finished, do nothing
            return 1;                           //orient finished
        }
        return 0;
    }
