
#ifndef SPI1_LCD_ST7789_H_
#define SPI1_LCD_ST7789_H_

#include "stm32f1xx.h"
#include "timer.h"
//#define ST7789_CS_SELECT()      (GPIOA->BRR  = (1U << 4))
//#define ST7789_CS_DESELECT()    (GPIOA->BSRR = (1U << 4))
//
//#define ST7789_DC_CMD()         (GPIOA->BRR  = (1U << 3)) // DC = 0: Gửi lệnh
//#define ST7789_DC_DATA()        (GPIOA->BSRR = (1U << 3)) // DC = 1: Gửi dữ liệu
//
//#define ST7789_RST_LOW()        (GPIOA->BRR  = (1U << 2))
//#define ST7789_RST_HIGH()       (GPIOA->BSRR = (1U << 2))
//
//#define ST7789_BLK_ON()         (GPIOB->BSRR = (1U << 0))
//#define ST7789_BLK_OFF()        (GPIOB->BRR  = (1U << 0))
//
//// --- Khai báo hàm ---
//void ST7789_Init(void);
//void ST7789_SendCommand(uint8_t cmd);
//void ST7789_SendData(uint8_t data);
//void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
#define ILI9341_CS_SELECT()      (GPIOA->BRR  = (1U << 4))
#define ILI9341_CS_DESELECT()    (GPIOA->BSRR = (1U << 4))

#define ILI9341_DC_CMD()         (GPIOA->BRR  = (1U << 3)) // DC = 0: Gửi lệnh
#define ILI9341_DC_DATA()        (GPIOA->BSRR = (1U << 3)) // DC = 1: Gửi dữ liệu

#define ILI9341_RST_LOW()        (GPIOA->BRR  = (1U << 2))
#define ILI9341_RST_HIGH()       (GPIOA->BSRR = (1U << 2))

#define ILI9341_BLK_ON()         (GPIOB->BSRR = (1U << 0))
#define ILI9341_BLK_OFF()        (GPIOB->BRR  = (1U << 0))

// --- Khai báo hàm ---
void ILI9341_Init(void);
void ILI9341_SendCommand(uint8_t cmd);
void ILI9341_SendData(uint8_t data);
void ILI9341_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ILI9341_FillColor(uint16_t color); // Hàm vẽ test toàn màn hình

void spi1_gpio_init(void);
void spi1_config(void);

void spi1_dma_init(void);
void spi1_dma_transmit(uint8_t *data, uint32_t size);
void spi1_dma_wait_complete(void);

#endif /* SPI1_LCD_ST7789_H_ */
