#include "w25q64.h"

#define GPIOBEN (1U<<3)
#define SPI2EN	(1U<<14)
/* --- SPI CR1 Bits --- */
#define SPI_CR1_CPHA        (1U << 0)
#define SPI_CR1_CPOL        (1U << 1)
#define SPI_CR1_MSTR        (1U << 2)
#define SPI_CR1_BR_DIV4     (1U << 3)    /* BR[2:0] = 001 => fPCLK/4 = 9MHz */
#define SPI_CR1_SPE         (1U << 6)
#define SPI_CR1_LSBFIRST    (1U << 7)
#define SPI_CR1_SSI         (1U << 8)
#define SPI_CR1_SSM         (1U << 9)
#define SPI_CR1_RXONLY      (1U << 10)
#define SPI_CR1_DFF         (1U << 11)

/* --- SPI SR Bits --- */
#define SPI_SR_RXNE         (1U << 0)
#define SPI_SR_TXE          (1U << 1)
#define SPI_SR_BSY          (1U << 7)*/


void w25q64_init (void){
	RCC->APB1ENR |= SPI2EN | GPIOBEN;
	
	GPIOB->CRH &= ~(0xFFFFU << 16);
	GPIOB->CRH |=  (0x3U << 16)   /* PB12: CS   */
	           |  (0xBU << 20)   /* PB13: SCK  */
	           |  (0x4U << 24)   /* PB14: MISO */
	           |  (0xBU << 28);  /* PB15: MOSI */
	
	SPI2->CR1 = 0;
	SPI2->CR1 |= SPI_CR1_MSTR;        /* Master mode                         */
	 SPI2->CR1 |= SPI_CR1_SSI;         /* Internal Slave Select = HIGH        */
     SPI2->CR1 |= SPI_CR1_SSM;         /* Software Slave Management           */
	SPI2->CR1 &= ~SPI_CR1_CPOL;       /* CPOL = 0 (Clock idle = LOW)         */
	SPI2->CR1 &= ~SPI_CR1_CPHA;       /* CPHA = 0 (Sample on rising edge)    */
    SPI2->CR1 &= ~SPI_CR1_DFF;        /* 8-bit data frame                    */
	SPI2->CR1 &= ~SPI_CR1_LSBFIRST;   /* MSB first                           */
	SPI2->CR1 |= SPI_CR1_BR_DIV4;     /* fPCLK1 / 4 = 36MHz / 4 = 9MHz       */

	 /* Bật SPI2 */
	SPI2->CR1 |= SPI_CR1_SPE;

	/* Đọc bỏ data rác */
	(void)SPI2->DR;
	 (void)SPI2->SR;

	  /* Đánh thức chip nếu đang ở chế độ ngủ */
	W25Q64_WakeUp();

   /* Delay ngắn ổn định */
    volatile uint32_t cnt = 1000;
    while (cnt--) {}
	
}



