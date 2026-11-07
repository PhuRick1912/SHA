#include "w25q64.h"

#define GPIOBEN (1U<<3)
#define SPI2EN	(1U<<14)



void w25q64_init (void){
	RCC->APB1ENR |= SPI2EN | GPIOBEN;
	
	GPIOB->CRH &= ~(0xFFFFU << 16);
	GPIOB->CRH |=  (0x3U << 16)   /* PB12: CS   */
	           |  (0xBU << 20)   /* PB13: SCK  */
	           |  (0x4U << 24)   /* PB14: MISO */
	           |  (0xBU << 28);  /* PB15: MOSI */
	
	
	
}



