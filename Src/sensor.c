#include "sensor.h"
#include "timer.h"

#define DEBOUNCE_TIME_MS 30U

static volatile bool state_door = false;
static volatile bool state_ir = false;
static volatile uint32_t last_door_irq_tick = 0;

void sensor_exti_init(void) {
    __disable_irq();

    RCC->APB2ENR |= GPIOAEN | AFIOEN;

    GPIOA->CRL &= ~((0xFU << (0 * 4)) | (0xFU << (1 * 4)));
    GPIOA->CRL |=  ((0x8U << (0 * 4)) | (0x8U << (1 * 4)));
    GPIOA->ODR |= (1U << 0) | (1U << 1); // Pull-up PA0, PA1

    // Đồng bộ trạng thái vật lý ban đầu ngay khi cấp nguồn
    state_door = (GPIOA->IDR & (1U << 0)) ? true : false;
    state_ir   = ((GPIOA->IDR & (1U << 1)) == 0) ? true : false;

    AFIO->EXTICR[0] &= ~((0xFU << (0 * 4)) | (0xFU << (1 * 4)));

    EXTI->IMR  |= (1U << 0) | (1U << 1);
    EXTI->FTSR |= (1U << 0) | (1U << 1);
    EXTI->RTSR |= (1U << 0) | (1U << 1);

    NVIC_SetPriority(EXTI0_IRQn, 1);
    NVIC_SetPriority(EXTI1_IRQn, 2);
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);

    __enable_irq();
}

void EXTI0_IRQHandler(void) {
    if (EXTI->PR & (1U << 0)) {
        EXTI->PR = (1U << 0); // Ghi 1 để xóa cờ (không dùng |= tránh xóa nhầm cờ khác)

        uint32_t now = Get_System_Tick();
        if ((now - last_door_irq_tick) >= DEBOUNCE_TIME_MS) {
            state_door = (GPIOA->IDR & (1U << 0)) ? true : false;
            last_door_irq_tick = now;
        }
    }
}

void EXTI1_IRQHandler(void) {
    if (EXTI->PR & (1U << 1)) {
        EXTI->PR = (1U << 1); // Ghi thẳng 1 vào thanh ghi PR (rc_w1)
        state_ir = ((GPIOA->IDR & (1U << 1)) == 0) ? true : false;
    }
}

bool Is_Door_Open(void) {
    return state_door;
}

bool Is_IR_Triggered(void) {
    return state_ir;
}
