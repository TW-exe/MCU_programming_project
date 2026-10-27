#include "pathfind.h"
#define First_MoveA 1200        //moving to section 4 for layout A
#define Second_MoveA 1400       //moving to section 3 for layout A
#define First_MoveB 1400        //moving to section 2 for layout B
#define Second_MoveB 1200       //moving to section 3 for layout B
#define ATurn (90)              //turning towards section 3 for layout A
#define BTurn (-90)             //turning towards section 3 for layout B

//Cliff / bumper recovery
#define Reverse_Dist (-30)      //back up 30mm when a cliff or bumper is detected
#define A_Cliff_Turn (10)       //layout A: rotate +10 degrees after a cliff
#define A_Bump_Turn (-10)       //layout A: rotate -10 degrees after a bumper hit
#define B_Cliff_Turn (-10)      //layout B: rotate -10 degrees after a cliff
#define B_Bump_Turn (10)        //layout B: rotate +10 degrees after a bumper hit

//Odometry conversions (must match kobuki.c)
#define PF_MM_PER_TICK (2.0*3.14159*35.0*((132.0/6545.0)/52.0))   //mm per encoder tick
#define PF_TICKS_PER_DEG 100    //kobuki angle is in 0.01 degree ticks
#define PF_HALF_TURN 18000      //180 degrees in kobuki angle ticks


//Pathfind
static enum {
    Start_Path,
    First_Move,
    First_Turn,
    Second_Move,
    Recover_Reverse,
    Recover_Rotate,
    Pathfind_Finished
} Pathfind_State, Resume_State;  //pathfind states, Resume_State = leg to continue after a recovery

static int16_t leg_remaining;       //mm (drive leg) or degrees (turn leg) still to go in the current leg
static uint16_t leg_start_dist;     //encoder ticks when the current drive leg was (re)started
static int16_t leg_start_angle;     //angle ticks when the current turn leg was (re)started
static int16_t recover_turn;        //rotation to do after backing up (+-10 degrees)


//start (or restart) a drive leg of dist mm
static void Start_Leg_Drive(Kobuki_Typedef *kobuki, int16_t dist){
    leg_remaining=dist;
    leg_start_dist=kobuki->distance;
    Kobuki_Drive_Distance_Setpoint(kobuki->distance,dist);
    kobuki->distance_complete=0;
}


//start (or restart) a turn leg of deg degrees
static void Start_Leg_Turn(Kobuki_Typedef *kobuki, int16_t deg){
    leg_remaining=deg;
    leg_start_angle=kobuki->angle;
    Kobuki_Rotate_Angle_Setpoint(kobuki->angle,deg);
    kobuki->rotation_complete=0;
}


//if a cliff or bumper is detected, save how much of the current leg is left and start backing up
static uint8_t Hazard_Detected(Kobuki_Typedef *kobuki){
    if (!kobuki->cliff && !kobuki->bumper){
        return 0;                                                               //nothing detected
    }

    if (Pathfind_State==First_Turn){                                            //interrupted during a turn
        int32_t turned=(int32_t)kobuki->angle-leg_start_angle;                  //angle ticks turned so far
        if (turned>PF_HALF_TURN) turned-=2*PF_HALF_TURN;                        //handle wrap at +-180 degrees
        else if (turned<-PF_HALF_TURN) turned+=2*PF_HALF_TURN;
        leg_remaining-=(int16_t)(turned/PF_TICKS_PER_DEG);                      //degrees left to turn
    } else {                                                                    //interrupted during a drive
        int16_t travelled=(int16_t)(uint16_t)(kobuki->distance-leg_start_dist); //encoder ticks driven so far (handles wrap)
        leg_remaining-=(int16_t)(travelled*PF_MM_PER_TICK);                     //mm left to drive
        leg_remaining-=Reverse_Dist;                                            //make up the 30mm we are about to back up
        if (leg_remaining<0) leg_remaining=0;
    }

    if (kobuki->cliff){                                                         //cliff takes priority over bumper
        recover_turn=(decision==LayoutA)?A_Cliff_Turn:B_Cliff_Turn;
    } else {
        recover_turn=(decision==LayoutA)?A_Bump_Turn:B_Bump_Turn;
    }

    Resume_State=Pathfind_State;                                                //remember which leg to continue
    Kobuki_Drive(STOP);
    Kobuki_Drive_Distance_Setpoint(kobuki->distance,Reverse_Dist);              //back up 30mm
    kobuki->distance_complete=0;
    Pathfind_State=Recover_Reverse;
    return 1;
}


void Start_Pathfind(void) {
    Pathfind_State=Start_Path;      //start first step in pathfind
}


uint8_t Pathfind_FSM(Kobuki_Typedef *kobuki){
    switch (Pathfind_State) {

        case Start_Path:            //setting it up 
            Start_Leg_Drive(kobuki,(decision==LayoutA)?First_MoveA:First_MoveB); // understand what the following code does
            Pathfind_State=First_Move;
        break;

        case First_Move:
            if (Hazard_Detected(kobuki)) break;     //cliff / bumper -> recovery

            if (kobuki->distance_complete){
                Kobuki_Drive(STOP);
                Start_Leg_Turn(kobuki,(decision==LayoutA)?ATurn:BTurn);
                Pathfind_State=First_Turn;
            }
        
            else{
                Kobuki_Drive_Distance(kobuki->distance);
            }

        break;

        case First_Turn:
            if (Hazard_Detected(kobuki)) break;     //cliff / bumper -> recovery

            if (kobuki->rotation_complete){
                Kobuki_Drive(STOP);
                Start_Leg_Drive(kobuki,(decision==LayoutA)?Second_MoveA:Second_MoveB);
                Pathfind_State=Second_Move;
            } 
        
            else{
                Kobuki_Rotate_Angle(kobuki->angle);
            }
        break;

        case Second_Move:
            if (Hazard_Detected(kobuki)) break;     //cliff / bumper -> recovery

            if (kobuki->distance_complete){
                Kobuki_Drive(STOP);
                Pathfind_State=Pathfind_Finished;
            } else {
                Kobuki_Drive_Distance(kobuki->distance);
            }
        break;

        case Recover_Reverse:       //backing up 30mm away from the cliff / bumper
            if (kobuki->distance_complete){
                Kobuki_Drive(STOP);
                Kobuki_Rotate_Angle_Setpoint(kobuki->angle,recover_turn);   //+-10 degrees depending on layout and sensor
                kobuki->rotation_complete=0;
                Pathfind_State=Recover_Rotate;
            } else {
                Kobuki_Drive_Distance(kobuki->distance);
            }
        break;

        case Recover_Rotate:        //rotating away from the cliff / bumper, then continue the interrupted leg
            if (kobuki->rotation_complete){
                Kobuki_Drive(STOP);
                if (Resume_State==First_Turn){
                    Start_Leg_Turn(kobuki,leg_remaining);       //finish the rest of the turn
                } else {
                    Start_Leg_Drive(kobuki,leg_remaining);      //drive the rest of the leg
                }
                Pathfind_State=Resume_State;
            } else {
                Kobuki_Rotate_Angle(kobuki->angle);
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

