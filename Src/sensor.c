#include "sensor.h"
#include "timer.h"

static volatile bool state_door = false; // false = Đóng, true = Mở
static volatile bool state_ir = false;

static volatile uint32_t last_button_press_time = 0;
#define DEBOUNCE_DELAY_MS 200 // 200ms chống nhiễu

void sensor_exti_init(void) {
	__disable_irq();

	RCC->APB2ENR |= (1U << 2) | (1U << 0); // GPIOAEN | AFIOEN

	GPIOA->CRL &= ~((0xFU << (0 * 4)) | (0xFU << (1 * 4)));

    // Cấu hình PA0 (Nút nhấn), PA1 (IR) là Input Pull-up
    GPIOA->CRL |=  ((0x8U << (0 * 4)) | (0x8U << (1 * 4)));

	GPIOA->ODR |= (1U << 0) | (1U << 1); // Kéo lên VCC

	AFIO->EXTICR[0] &= ~((0xFU << (0 * 4)) | (0xFU << (1 * 4)));

	EXTI->IMR |= (1U<<0) | (1U<<1);

	EXTI->FTSR |= (1U << 0);
	EXTI->RTSR &= ~(1U << 0);


	EXTI->FTSR |= (1U << 1);
	EXTI->RTSR |= (1U << 1);

	NVIC_SetPriority(EXTI0_IRQn, 1);
	NVIC_SetPriority(EXTI1_IRQn, 2);
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);

	__enable_irq();
}

// Hàm ngắt PA0 
void EXTI0_IRQHandler(void) {
    if (EXTI->PR & (1U << 0)) {
        EXTI->PR |= (1U << 0); // Xóa cờ ngắt

        uint32_t current_time = Get_System_Tick();

        if ((current_time - last_button_press_time) > DEBOUNCE_DELAY_MS) {
            last_button_press_time = current_time;

            state_door = !state_door;

        }
    }
}

// Hàm ngắt PA1 (Cảm biến IR)
void EXTI1_IRQHandler(void) {
    if (EXTI->PR & (1U << 1)) {
        EXTI->PR |= (1U << 1);

        if ((GPIOA->IDR & (1U << 1)) == 1) {
            state_ir = true; // mức 1 -> Có người
        } else {
            state_ir = false;
        }
    }
}

bool Is_Door_Open(void) {
    return state_door;
}

bool Is_IR_Triggered(void) {
    return state_ir;
}
