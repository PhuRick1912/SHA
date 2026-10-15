/*
 * fsm.h
 *
 *  Created on: Oct 3, 2026
 *      Author: PHU
 */

#ifndef FSM_H_
#define FSM_H_

typedef enum {
    STATE_IDLE,
    STATE_DOOR_OPENED_IN,
    STATE_WAIT_IR_IN,
    STATE_IR_DETECTED_OUT,
    STATE_DOOR_OPENED_OUT,
    STATE_COUNTDOWN_5MIN
} SystemState_t;

void Home_Automation_FSM_Init(void);
void Home_Automation_FSM_Update(void);

#endif /* FSM_H_ */
