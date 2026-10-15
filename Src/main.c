#include "stm32f1xx.h"
#include "sensor.h"
#include "relay.h"
#include "timer.h"
#include "fsm.h"
#include "uart.h"
#include "spi1_lcd_st7789.h"
#include "esp8266_mqtt.h"

#define MAGNETIC_SENSOR (1U << 0)
#define IR_SENSOR (1U << 1)

void SystemClock_Config_72MHz(void);

int main(void) {

	SystemClock_Config_72MHz();

	SysTick_Init();//Khởi tạo bộ đếm SYSTICK , ngắt mỗi 1ms

	relay_driver_init();//Khởi tạo ngõ ra điều khiển RELAY , push-pull, max 2Mhz

	sensor_exti_init();//khởi tạo ngõ vào cảm biến, pull-up, khởi tạo ngắt cho cảm biến

	spi1_gpio_init(); //khởi tạo giao tiếp với lcd

	spi1_config();// cấu hình spi giao tiếp với lcd

	spi1_dma_init(); // khởi tạo DMA cho spi, mem->per, 8 bit

	ST7789_Init();

	//ST7789_FillColor(0xFFFF);

	//ST7789_TestRedSquare50x50();

	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // Cấp xung clock cho Port C

	GPIOC->CRH &= ~(0x0F << (5 * 4));

	GPIOC->CRH |= (0x03 << (5 * 4));

	GPIOC->ODR &= ~(1 << 13);

	uint32_t boot_delay = Get_System_Tick();

	while((Get_System_Tick() - boot_delay) < 5000) {}

	UART1_Clear_Rx_Buffer();

	UART1_Clear_Data_Flag();

	UART1_DMA_Init();

	Home_Automation_FSM_Init();

	ESP8266_MQTT_Init();

while(1)

{

	Home_Automation_FSM_Update();

	ESP8266_MQTT_Update();
}

return 0;

}



void SystemClock_Config_72MHz(void) {

// 1. Bật thạch anh ngoài (HSE)

	RCC->CR |= RCC_CR_HSEON;

	while (!(RCC->CR & RCC_CR_HSERDY)); // Chờ HSE ổn định

	// 2. Cấu hình Flash Latency cho 72MHz
	FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;

	// 3. Cấu hình PLL: Nguồn HSE, Hệ số nhân x9 (8MHz * 9 = 72MHz)
	// Bus APB1 = 36MHz (DIV2), Bus APB2 = 72MHz (DIV1)
	RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1;

	// 4. Bật PLL và chờ ổn định
	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY));

	// 5. Chọn PLL làm xung nhịp hệ thống (System Clock)
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

}
