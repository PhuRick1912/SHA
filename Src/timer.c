#include "timer.h"



static volatile uint32_t system_tick_ms = 0;


void SysTick_Init(void) {
    // Cấu hình SysTick ngắt mỗi 1ms
    SysTick_Config(72000000U / 1000);
}


void SysTick_Handler(void) {
    system_tick_ms++;
}


uint32_t Get_System_Tick(void) {
    return system_tick_ms;
}
