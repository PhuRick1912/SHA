#include "sensor.h"

//Cấu hình PA0 (Cửa), PA1 (IR)
static volatile bool state_door = false;
static volatile bool state_ir = false;

void sensor_exti_init(void) {
	__disable_irq();

	RCC->APB2ENR |= GPIOAEN | AFIOEN;

	GPIOA->CRL &= ~((0xFU << (0 * 4)) | (0xFU << (1 * 4)));

    GPIOA->CRL |=  ((0x8U << (0 * 4)) | (0x8U << (1 * 4)));

	GPIOA->ODR |= (1U << 0) | (1U << 1);

	AFIO->EXTICR[0] &= ~((0xFU << (0 * 4)) | (0xFU << (1 * 4)));

	EXTI->IMR |= (1U<<0) | (1U<<1);

	EXTI->FTSR |= (1U << 0) | (1U << 1);
	EXTI->RTSR |= (1U << 0) | (1U << 1);

	NVIC_SetPriority(EXTI0_IRQn, 1);
	NVIC_SetPriority(EXTI1_IRQn, 2);
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);

	__enable_irq();
}
// Hàm ngắt PA0 (Cảm biến từ - Cửa)
void EXTI0_IRQHandler(void) {
    if (EXTI->PR & (1U << 0)) {
        EXTI->PR |= (1U << 0); // Xóa cờ ngắt phần cứng

        // Đọc lại thanh ghi IDR

        if (GPIOA->IDR & (1U << 0)) {
            state_door = true;  // Chân PA0 bị thả nổi lên mức 1 -> Cửa Mở
        } else {
            state_door = false; // Chân PA0 bị kéo xuống mức 0 -> Cửa Đóng
        }
    }
}

// Hàm ngắt PA1 (Cảm biến IR)
void EXTI1_IRQHandler(void) {
    if (EXTI->PR & (1U << 1)) {
        EXTI->PR |= (1U << 1);

        if ((GPIOA->IDR & (1U << 1)) == 0) {
            state_ir = true; // Kéo xuống mức 0 -> Có người
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
