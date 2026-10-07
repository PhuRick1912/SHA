/*
 * relay.h
 *
 *  Created on: Oct 3, 2026
 *      Author: PHU
 */

#ifndef RELAY_H_
#define RELAY_H_

#include "stm32f1xx.h"
#include "stdbool.h"



#define RELAY_PIN 8

typedef enum {
    RELAY_OFF = 0,
    RELAY_ON  = 1
} RelayState_t;


extern volatile bool flag_relay_changed;
extern volatile RelayState_t current_relay_state;
void relay_driver_init(void);
void relay_set_state(RelayState_t state);

#endif /* RELAY_H_ */
