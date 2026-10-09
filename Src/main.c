#include "stm32f1xx.h"
#include "sensor.h"
#include "relay.h"
#include "timer.h"
#include "fsm.h"
#include "uart.h"
#include "spi1_lcd_st7789.h"
#include "esp8266_mqtt.h"

#define MAGNETIC_SENSOR 	(1U << 0)
#define IR_SENSOR			(1U << 1)

int main(void) {

	SysTick_Init();//Khởi tạo bộ đếm SYSTICK , ngắt mỗi 1ms

	relay_driver_init();//Khởi tạo ngõ ra điều khiển RELAY , push-pull, max 2Mhz

	sensor_exti_init();//khởi tạo ngõ vào cảm biến, pull-up, khởi tạo ngắt cho cảm biến

	spi1_gpio_init(); //khởi tạo giao tiếp với lcd

	spi1_config();// cấu hình spi giao tiếp với lcd

	ILI9341_Init();

	spi1_dma_init(); // khởi tạo DMA cho spi, mem->per, 8 bit

	ILI9341_FillColor(0xF800);

	UART1_DMA_Init();

	Home_Automation_FSM_Init();

	ESP8266_MQTT_Init();
	while(1)
	{
	Home_Automation_FSM_Update();

	ESP8266_MQTT_Update();

	//UART_Process_MQTT_Publish();

	}
	return 0;
}
