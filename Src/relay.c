// #include "relay.h"

// volatile bool flag_relay_changed = false;
// volatile RelayState_t current_relay_state = RELAY_OFF;

// void relay_driver_init(void) {
//     RCC->APB2ENR |= (1U << 2);
//     GPIOA->CRH &= ~(0xFU);
//     GPIOA->CRH |=  (1U<<1);

//     // DEFAULT: OFF
//     GPIOA->BSRR = (1U << (RELAY_PIN + 16));
//     current_relay_state = RELAY_OFF;
//     flag_relay_changed = false;
// }

// void relay_set_state(RelayState_t new_state) {
//     if (new_state == current_relay_state) {
//         return; //no change
//     }

//     if (new_state == RELAY_ON) {
//         GPIOA->BSRR = (1U << RELAY_PIN);
//     }
//     else if (new_state == RELAY_OFF) {
//         GPIOA->BSRR = (1U << (RELAY_PIN + 16));
//     }

//     current_relay_state = new_state; // Luu trang thai moi
//     flag_relay_changed = true;       // relay thay doi
// }
#include "relay.h"

volatile uint8_t current_relay_mask = 0x00U;
volatile bool flag_relay_changed = false;

typedef struct {
    GPIO_TypeDef *port;
    uint8_t pin;
} RelayPinMap_t;

/* Bảng ánh xạ phần cứng 4 kênh Relay: CH1=PA8, CH2=PB6, CH3=PB7, CH4=PB8 */
static const RelayPinMap_t s_relay_pins[RELAY_COUNT] = { { GPIOA, 8U }, { GPIOB, 6U }, { GPIOB, 7U }, { GPIOB, 8U } };

static void Relay_WritePin(RelayChannel_t ch, RelayState_t state) {
    GPIO_TypeDef *port = s_relay_pins[ch].port;
    uint8_t pin = s_relay_pins[ch].pin;
#if (RELAY_ACTIVE_LOW == 1U)
    if (state == RELAY_ON) {
        port->BSRR = (1U << (pin + 16U));
    } else {
        port->BSRR = (1U << pin);
    }
#else
    if (state == RELAY_ON) {
        port->BSRR = (1U << pin);
    } else {
        port->BSRR = (1U << (pin + 16U));
    }
#endif
}

void relay_driver_init(void) {
    /* Cấp xung nhịp cho GPIOA (bit 2) và GPIOB (bit 3) */
    RCC->APB2ENR |= (1U << 2) | (1U << 3);
    for (uint8_t i = 0; i < RELAY_COUNT; i++) {
        Relay_WritePin((RelayChannel_t)i, RELAY_OFF);
    }
    /* Cấu hình Output Push-Pull 2MHz cho PA8, PB6, PB7, PB8 */
    GPIOA->CRH &= ~(0xFU << 0);
    GPIOA->CRH |= (0x2U << 0);
    GPIOB->CRL &= ~((0xFU << 24) | (0xFU << 28));
    GPIOB->CRL |= ((0x2U << 24) | (0x2U << 28));
    GPIOB->CRH &= ~(0xFU << 0);
    GPIOB->CRH |= (0x2U << 0);
    current_relay_mask = 0x00U;
    flag_relay_changed = false;
}

void relay_set_channel(RelayChannel_t ch, RelayState_t new_state) {
    if (ch >= RELAY_COUNT) {
        return;
    }
    uint8_t bit_mask = (uint8_t)(1U << ch);
    RelayState_t cur_state = ((current_relay_mask & bit_mask) != 0U) ? RELAY_ON : RELAY_OFF;
    if (new_state == cur_state) {
        return;
    }
    Relay_WritePin(ch, new_state);
    if (new_state == RELAY_ON) {
        current_relay_mask |= bit_mask;
    } else {
        current_relay_mask &= (uint8_t)(~bit_mask);
    }
    flag_relay_changed = true;
}

void relay_toggle_channel(RelayChannel_t ch) {
    if (ch >= RELAY_COUNT) {
        return;
    }
    RelayState_t cur = relay_get_channel(ch);
    relay_set_channel(ch, (cur == RELAY_ON) ? RELAY_OFF : RELAY_ON);
}

void relay_set_mask(uint8_t mask_on, uint8_t mask_affected) {
    mask_affected &= RELAY_ALL_MASK;
    uint8_t next_mask = (current_relay_mask & (uint8_t)(~mask_affected)) | (mask_on & mask_affected);
    if (next_mask == current_relay_mask) {
        return;
    }
    for (uint8_t i = 0; i < RELAY_COUNT; i++) {
        if (mask_affected & (1U << i)) {
            RelayState_t st = (next_mask & (1U << i)) ? RELAY_ON : RELAY_OFF;
            Relay_WritePin((RelayChannel_t)i, st);
        }
    }
    current_relay_mask = next_mask;
    flag_relay_changed = true;
}

void relay_set_all(RelayState_t state) {
    relay_set_mask((state == RELAY_ON) ? RELAY_ALL_MASK : 0x00U, RELAY_ALL_MASK);
}

RelayState_t relay_get_channel(RelayChannel_t ch) {
    if (ch >= RELAY_COUNT) {
        return RELAY_OFF;
    }
    return ((current_relay_mask & (1U << ch)) != 0U) ? RELAY_ON : RELAY_OFF;
}

void relay_set_state(RelayState_t state) {
    relay_set_channel(RELAY_CH1, state);
}
