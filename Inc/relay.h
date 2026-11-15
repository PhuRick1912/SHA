/*
 * relay.h
 *
 *  Created on: Oct 3, 2026
 *      Author: PHU
 */

// #ifndef RELAY_H_
// #define RELAY_H_

// #include "stm32f1xx.h"
// #include "stdbool.h"

// #define RELAY_PIN 8

// typedef enum {
//     RELAY_OFF = 0,
//     RELAY_ON  = 1
// } RelayState_t;

// extern volatile bool flag_relay_changed;//relay change
// extern volatile RelayState_t current_relay_state;//relay state

// void relay_driver_init(void);
// void relay_set_state(RelayState_t state);

// #endif /* RELAY_H_ */
#ifndef RELAY_H_
#define RELAY_H_

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>

#define RELAY_COUNT 4U
/* Đặt RELAY_ACTIVE_LOW = 1U nếu module Relay kích mức THẤP, đặt = 0U nếu kích mức CAO */
#define RELAY_ACTIVE_LOW 0U

typedef enum {
    RELAY_CH1 = 0,
    RELAY_CH2 = 1,
    RELAY_CH3 = 2,
    RELAY_CH4 = 3
} RelayChannel_t;

typedef enum {
    RELAY_OFF = 0,
    RELAY_ON = 1
} RelayState_t;

#define RELAY_CH1_MASK (1U << 0)
#define RELAY_CH2_MASK (1U << 1)
#define RELAY_CH3_MASK (1U << 2)
#define RELAY_CH4_MASK (1U << 3)
#define RELAY_ALL_MASK (0x0FU)

extern volatile uint8_t current_relay_mask;
extern volatile bool flag_relay_changed;

void relay_driver_init(void);
void relay_set_channel(RelayChannel_t ch, RelayState_t state);
void relay_toggle_channel(RelayChannel_t ch);
void relay_set_mask(uint8_t mask_on, uint8_t mask_affected);
void relay_set_all(RelayState_t state);
RelayState_t relay_get_channel(RelayChannel_t ch);
void relay_set_state(RelayState_t state);

#endif /* RELAY_H_ */
