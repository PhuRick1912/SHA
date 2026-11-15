#include "stm32f1xx.h"
#include "sensor.h"
#include "relay.h"
#include "timer.h"
#include "fsm.h"
#include "uart.h"
#include "spi1_lcd_st7789.h"
#include "w25q64_read.h"
#include "esp8266_mqtt.h"

void SystemClock_Config_72MHz(void);

int main(void) {
    SystemClock_Config_72MHz();
    /* Giữ kết nối nạp/debug SWD luôn hoạt động khi lõi CPU ở chế độ Sleep (__WFI) */
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;
    SysTick_Init();
    relay_driver_init();
    sensor_exti_init();
    W25Q64_Init();
    spi1_gpio_init();
    spi1_config();
    spi1_dma_init();
    ST7789_Init();

    LCD_Draw_Static_Header();
    LCD_Update_Relays_If_Changed();
    LCD_Update_People_If_Changed();
    LCD_Update_Weather_If_Changed();

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(0xFU << (5 * 4));
    GPIOC->CRH |= (0x3U << (5 * 4));
    GPIOC->ODR &= ~(1U << 13);

    uint32_t boot_delay = Get_System_Tick();
    while ((Get_System_Tick() - boot_delay) < 5000UL) {}

    UART1_DMA_Init();
    Home_Automation_FSM_Init();
    ESP8266_MQTT_Init();

    while (1) {
        Home_Automation_FSM_Update();
        ESP8266_MQTT_Update();
        /* Đưa lõi Cortex-M3 vào chế độ Sleep cho đến khi có ngắt đánh thức tiếp theo */
        __WFI();
    }
    return 0;
}

void SystemClock_Config_72MHz(void) {
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY)) {}
    FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
    RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1;
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {}
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}
}
