
#ifndef SPI1_LCD_ST7789_H_
#define SPI1_LCD_ST7789_H_

#include "stm32f1xx.h"
#include "timer.h"
#include <stdint.h>

#define ST7789_CS_SELECT() (GPIOA->BRR = (1U << 4))
#define ST7789_CS_DESELECT() (GPIOA->BSRR = (1U << 4))
#define ST7789_DC_CMD() (GPIOA->BRR = (1U << 3))
#define ST7789_DC_DATA() (GPIOA->BSRR = (1U << 3))
#define ST7789_RST_LOW() (GPIOA->BRR = (1U << 2))
#define ST7789_RST_HIGH() (GPIOA->BSRR = (1U << 2))
#define ST7789_BLK_ON() (GPIOB->BSRR = (1U << 0))
#define ST7789_BLK_OFF() (GPIOB->BRR = (1U << 0))

void spi1_gpio_init(void);
void spi1_config(void);
void spi1_dma_init(void);
void spi1_dma_transmit(uint8_t *data, uint32_t size);
void spi1_dma_wait_complete(void);

void ST7789_Init(void);
void ST7789_SendCommand(uint8_t cmd);
void ST7789_SendData(uint8_t data);
void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ST7789_FillColor(uint16_t color);
void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7789_WriteDataBuffer(uint8_t *data, uint32_t len);
void ST7789_TestRedSquare50x50(void);
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *img_data);

#endif /* SPI1_LCD_ST7789_H_ */
