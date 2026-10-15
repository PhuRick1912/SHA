#include "relay.h"

volatile bool flag_relay_changed = false;
volatile RelayState_t current_relay_state = RELAY_OFF;

void relay_driver_init(void) {
    RCC->APB2ENR |= (1U << 2);
    GPIOA->CRH &= ~(0xFU);
    GPIOA->CRH |=  (1U<<1);

    // DEFAULT: OFF
    GPIOA->BSRR = (1U << (RELAY_PIN + 16));
    current_relay_state = RELAY_OFF;
    flag_relay_changed = false;
}

void relay_set_state(RelayState_t new_state) {
    if (new_state == current_relay_state) {
        return; //no change
    }

    if (new_state == RELAY_ON) {
        GPIOA->BSRR = (1U << RELAY_PIN);
    }
    else if (new_state == RELAY_OFF) {
        GPIOA->BSRR = (1U << (RELAY_PIN + 16));
    }

    current_relay_state = new_state; // Luu trang thai moi
    flag_relay_changed = true;       // relay thay doi
}
