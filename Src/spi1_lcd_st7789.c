#include "spi1_lcd_st7789.h"

#define AFIOEN (1U << 0)
#define GPIOAEN (1U << 2)
#define GPIOBEN (1U << 3)
#define SPI1EN (1U << 12)
#define DMA1EN (1U << 0)

#define SPI_CR2_TXDMAEN (1U << 1)
#define RXONLY (1U << 10)
#define LSBFIRST (1U << 7)
#define MSTR (1U << 2)
#define DFF (1U << 11)
#define SSM (1U << 9)
#define SSI (1U << 8)
#define SPEN (1U << 6)
#define SR_TXE (1U << 1)
#define SR_BSY (1U << 7)

#define DMA_CCR_EN (1U << 0)
#define DMA_CCR_DIR (1U << 4)
#define DMA_CCR_MINC (1U << 7)
#define DMA_ISR_TCIF3 (1U << 9)
#define DMA_IFCR_CGIF3 (1U << 8)

static void delay_ms(uint32_t ms) {
    uint32_t start = Get_System_Tick();
    while ((Get_System_Tick() - start) < ms) {}
}

void spi1_gpio_init(void) {
    RCC->APB2ENR |= GPIOAEN | GPIOBEN | AFIOEN | SPI1EN;
    GPIOA->BSRR = (1U << 4) | (1U << 3) | (1U << 2);
    GPIOB->BSRR = (1U << 0);
    /* Cấu hình PA2(RES), PA3(DC), PA4(CS), PA5(SCK), PA7(MOSI) */
    GPIOA->CRL &= ~((0xFU << 8) | (0xFU << 12) | (0xFU << 16) | (0xFU << 20) | (0xFU << 28));
    GPIOA->CRL |= ((0x3U << 8) | (0x3U << 12) | (0x3U << 16) | (0xBU << 20) | (0xBU << 28));
    /* Cấu hình PB0(BLK) Output Push-Pull 50MHz */
    GPIOB->CRL &= ~(0xFU << 0);
    GPIOB->CRL |= (0x3U << 0);
}

void spi1_config(void) {
    SPI1->CR1 &= ~(RXONLY | LSBFIRST | DFF);
    SPI1->CR1 |= MSTR | SSI | SSM;
    SPI1->CR2 |= SPI_CR2_TXDMAEN;
    SPI1->CR1 |= SPEN;
}

void spi1_dma_init(void) {
    RCC->AHBENR |= DMA1EN;
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    DMA1_Channel3->CPAR = (uint32_t)&SPI1->DR;
    DMA1_Channel3->CCR = DMA_CCR_DIR | DMA_CCR_MINC;
}

void spi1_dma_wait_complete(void) {
    if (DMA1_Channel3->CCR & DMA_CCR_EN) {
        while (!(DMA1->ISR & DMA_ISR_TCIF3)) {}
        DMA1->IFCR = DMA_IFCR_CGIF3;
        DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    }
    while (!(SPI1->SR & SR_TXE)) {}
    while (SPI1->SR & SR_BSY) {}
    (void)SPI1->DR;
    (void)SPI1->SR;
}

void spi1_dma_transmit(uint8_t *data, uint32_t size) {
    if (size == 0) {
        return;
    }
    spi1_dma_wait_complete();
    DMA1_Channel3->CMAR = (uint32_t)data;
    DMA1_Channel3->CNDTR = size;
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

void ST7789_SendCommand(uint8_t cmd) {
    spi1_dma_wait_complete();
    ST7789_DC_CMD();
    ST7789_CS_SELECT();
    while (!(SPI1->SR & SR_TXE)) {}
    SPI1->DR = cmd;
    while (SPI1->SR & SR_BSY) {}
    ST7789_CS_DESELECT();
}

void ST7789_SendData(uint8_t data) {
    spi1_dma_wait_complete();
    ST7789_DC_DATA();
    ST7789_CS_SELECT();
    while (!(SPI1->SR & SR_TXE)) {}
    SPI1->DR = data;
    while (SPI1->SR & SR_BSY) {}
    ST7789_CS_DESELECT();
}

void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ST7789_SendCommand(0x2A);
    ST7789_SendData(x0 >> 8);
    ST7789_SendData(x0 & 0xFF);
    ST7789_SendData(x1 >> 8);
    ST7789_SendData(x1 & 0xFF);
    ST7789_SendCommand(0x2B);
    ST7789_SendData(y0 >> 8);
    ST7789_SendData(y0 & 0xFF);
    ST7789_SendData(y1 >> 8);
    ST7789_SendData(y1 & 0xFF);
    ST7789_SendCommand(0x2C);
}

void ST7789_WriteDataBuffer(uint8_t *data, uint32_t len) {
    if (len == 0) {
        return;
    }
    ST7789_DC_DATA();
    ST7789_CS_SELECT();
    spi1_dma_transmit(data, len);
    spi1_dma_wait_complete();
    ST7789_CS_DESELECT();
}

void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (w == 0 || h == 0) {
        return;
    }
    static uint8_t fill_buf[512];
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFF);
    for (uint16_t i = 0; i < sizeof(fill_buf); i += 2) {
        fill_buf[i] = hi;
        fill_buf[i + 1] = lo;
    }
    ST7789_SetWindow(x, y, x + w - 1U, y + h - 1U);
    uint32_t total_bytes = (uint32_t)w * (uint32_t)h * 2UL;
    ST7789_DC_DATA();
    ST7789_CS_SELECT();
    while (total_bytes > 0) {
        uint32_t chunk = (total_bytes > sizeof(fill_buf)) ? sizeof(fill_buf) : total_bytes;
        spi1_dma_transmit(fill_buf, chunk);
        spi1_dma_wait_complete();
        total_bytes -= chunk;
    }
    ST7789_CS_DESELECT();
}

void ST7789_FillColor(uint16_t color) {
    ST7789_FillRect(0, 0, 240, 240, color);
}

void ST7789_TestRedSquare50x50(void) {
    ST7789_FillRect(95, 95, 50, 50, 0xF800);
}

void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *img_data) {
    ST7789_SetWindow(x, y, x + w - 1U, y + h - 1U);
    ST7789_WriteDataBuffer((uint8_t *)img_data, (uint32_t)w * (uint32_t)h * 2UL);
}

void ST7789_Init(void) {
    ST7789_BLK_ON();
    ST7789_RST_LOW();
    delay_ms(50);
    ST7789_RST_HIGH();
    delay_ms(50);
    ST7789_SendCommand(0x01);
    delay_ms(150);
    ST7789_SendCommand(0x11);
    delay_ms(120);
    ST7789_SendCommand(0x3A);
    ST7789_SendData(0x55);
    ST7789_SendCommand(0x36);
    ST7789_SendData(0x00);
    ST7789_SendCommand(0x21);
    ST7789_SendCommand(0x13);
    ST7789_SendCommand(0x29);
    delay_ms(50);
}
