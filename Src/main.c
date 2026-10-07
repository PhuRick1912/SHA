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

	SysTick_Init();

	relay_driver_init();

	sensor_exti_init();

	spi1_gpio_init();

	spi1_config();

	ILI9341_Init();

	spi1_dma_init();

	ILI9341_FillColor(0xF800);

	UART1_DMA_Init();

	Home_Automation_FSM_Init();

	ESP8266_MQTT_Init();
	while(1)
	{
	Home_Automation_FSM_Update();

	ESP8266_MQTT_Update();



	UART_Process_MQTT_Publish();

	}
	return 0;
}
