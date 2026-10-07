#include "spi1_lcd_st7789.h"

#define GPIOAEN		(1U << 2)
#define GPIOBEN		(1U << 3)
#define AFIOEN		(1U << 1)
#define SPI1EN		(1U << 12)
#define DMA1EN      (1U << 0)

#define SPI_CR2_TXDMAEN (1U << 1)

#define RXONLY		(1U<<10)
#define LSBFIRST	(1U<<7)
#define MSTR		(1U<<2)
#define DFF		(1U<<11)
#define SSM		(1U<<9)
#define SSI		(1U<<8)
#define SPEN	(1U<<6)
#define SR_TXE	(1U<<1)
#define SR_BSY	(1U<<7)

#define DMA_CCR_EN      (1U << 0)
#define DMA_CCR_DIR     (1U << 4) // Hướng Truyền: Memory to Peripheral
#define DMA_CCR_MINC    (1U << 7) // Tự động tăng địa chỉ Memory
#define DMA_ISR_TCIF3   (1U << 9) // Cờ hoàn thành truyền Channel 3
#define DMA_IFCR_CGIF3  (1U << 8) // Cờ xóa ngắt Global Channel 3
void spi1_gpio_init(void)
{
	RCC->APB2ENR |= GPIOAEN | GPIOBEN | AFIOEN | SPI1EN;

	GPIOA->BSRR = (1 << 4) | (1 << 3) | (1 << 2); // CS, DC, RES
	GPIOB->BSRR = (1 << 0);                       // BLK trên PB0

	GPIOA->CRL &= ~((0xFU << 8)  | // PA2
	                    (0xFU << 12) | // PA3
	                    (0xFU << 16) | // PA4
	                    (0xFU << 20) | // PA5
	                    (0xFU << 28)); // PA7


	    GPIOA->CRL |=  ((0x3U << 8)  | // PA2: Output Push-Pull (50MHz)
	                    (0x3U << 12) | // PA3: Output Push-Pull (50MHz)
	                    (0x3U << 16) | // PA4: Output Push-Pull (50MHz)
	                    (0xBU << 20) | // PA5: Alternate Function Push-Pull
	                    (0xBU << 28)); // PA7: Alternate Function Push-Pull



	    GPIOB->CRL &= ~(0xFU << 0);
	    GPIOB->CRL |=  (0x3U << 0);
}


void spi1_config(void)
{
	SPI1->CR1 &= ~RXONLY;
	SPI1->CR1 &= ~LSBFIRST;
	SPI1->CR1 |= MSTR;
	SPI1->CR1 &= ~DFF;
	SPI1->CR1 |= SSI;
	SPI1->CR1 |= SSM;

	// KÍCH HOẠT: Yêu cầu DMA khi buffer TX của SPI trống
	    SPI1->CR2 |= SPI_CR2_TXDMAEN;

	SPI1->CR1 |= SPEN;
}

void spi1_dma_init(void) {
    // 1. Cấp xung nhịp cho DMA1
    RCC->AHBENR |= DMA1EN;

    // 2. Tắt Channel 3 trước khi cấu hình (Bảo vệ phần cứng)
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;

    // 3. Trỏ địa chỉ Ngoại vi (Peripheral) cố định vào thanh ghi Data của SPI1
    DMA1_Channel3->CPAR = (uint32_t)&SPI1->DR;

    // 4. Cấu hình Channel 3: Hướng Mem->Periph, Auto-increment Memory
    // MSIZE = 0 (8-bit), PSIZE = 0 (8-bit), không dùng ngắt (Polling cờ ISR)
    DMA1_Channel3->CCR = DMA_CCR_DIR | DMA_CCR_MINC;
}

// Hàm gửi dữ liệu Non-blocking / Blocking-on-Start
void spi1_dma_transmit(uint8_t *data, uint32_t size) {
    // Đợi nếu DMA vẫn đang bận gửi gói trước đó
    if (DMA1_Channel3->CCR & DMA_CCR_EN) {
        while (!(DMA1->ISR & DMA_ISR_TCIF3)) {}
    }

    // Tắt DMA, xóa cờ để chuẩn bị tải gói mới
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    DMA1->IFCR = DMA_IFCR_CGIF3;

    // Nạp địa chỉ bắt đầu của Buffer và số lượng Byte cần bắn
    DMA1_Channel3->CMAR = (uint32_t)data;
    DMA1_Channel3->CNDTR = size;

    // Bật lại DMA, phần cứng tự động rút data đưa vào SPI
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

// Hàm đợi phần cứng hoàn tất thực sự (Quan trọng khi điều khiển chân CS/DC)
void spi1_dma_wait_complete(void) {
    // 1. Đợi DMA đẩy xong byte cuối cùng vào thanh ghi của SPI
    while (!(DMA1->ISR & DMA_ISR_TCIF3)) {}

    // 2. Đợi bản thân SPI shift xong bit cuối cùng ra đường dây vật lý MOSI
    while (SPI1->SR & SR_BSY) {}
}

// Hàm trễ Non-blocking cơ bản dựa trên SysTick
static void Delay_ms(uint32_t ms) {
    uint32_t start = Get_System_Tick();
    while ((Get_System_Tick() - start) < ms);
}

// Hàm gửi 1 Byte Polling tốc độ cao (Bỏ qua cấu trúc rườm rà)
static void SPI_SendByte(uint8_t byte) {
    while(!(SPI1->SR & SR_TXE)) {}  // Đợi thanh ghi trống
    SPI1->DR = byte;                // Đẩy data
    while(!(SPI1->SR & SR_TXE)) {}  // Đợi đẩy xong vào Shift Register
    while(SPI1->SR & SR_BSY) {}     // Đợi nhịp Clock cuối cùng kết thúc
}

//void ST7789_SendCommand(uint8_t cmd) {
//    ST7789_DC_CMD();       // Kéo DC xuống mức 0
//    ST7789_CS_SELECT();    // Kéo CS xuống mức 0
//    SPI_SendByte(cmd);
//    ST7789_CS_DESELECT();  // Trả CS lên mức 1
//}
//
//void ST7789_SendData(uint8_t data) {
//    ST7789_DC_DATA();      // Kéo DC lên mức 1
//    ST7789_CS_SELECT();
//    SPI_SendByte(data);
//    ST7789_CS_DESELECT();
//}
//
//void ST7789_Init(void) {
//    // 1. Hard Reset phần cứng
//    ST7789_CS_DESELECT();
//    ST7789_RST_HIGH();
//    Delay_ms(10);
//    ST7789_RST_LOW();
//    Delay_ms(10);
//    ST7789_RST_HIGH();
//    Delay_ms(120); // ST7789 cần ít nhất 120ms sau khi Reset
//
//    // 2. Thoát chế độ ngủ (Sleep Out)
//    ST7789_SendCommand(0x11);
//    Delay_ms(120);
//
//    // 3. Cấu hình định dạng màu 16-bit (RGB565)
//    ST7789_SendCommand(0x3A);
//    ST7789_SendData(0x05); // 0x05 = 16-bit/pixel
//
//    // 4. Cấu hình hướng hiển thị (MADCTL)
//    ST7789_SendCommand(0x36);
//    ST7789_SendData(0x00); // 0x00: Mặc định. Có thể thay đổi 0x70, 0xA0 để xoay ngang/dọc
//
//    // 5. Cấu hình Inversion (Panel IPS của ST7789 bắt buộc bật Invert)
//    ST7789_SendCommand(0x21); // Inversion ON (Nếu màu bị âm bản, đổi thành 0x20)
//
//    // 6. Bật màn hình hiển thị
//    ST7789_SendCommand(0x13); // Normal Display Mode On
//    ST7789_SendCommand(0x29); // Display On
//    Delay_ms(20);
//
//    // 7. Bật đèn nền
//    ST7789_BLK_ON();
//}
//
//// Hàm khoanh vùng cập nhật Pixel (Ứng dụng cụ thể đoạn code bạn hỏi)
//void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
//    // Cài đặt địa chỉ cột (Column Address Set)
//    ST7789_SendCommand(0x2A);
//    ST7789_SendData(x0 >> 8);
//    ST7789_SendData(x0 & 0xFF);
//    ST7789_SendData(x1 >> 8);
//    ST7789_SendData(x1 & 0xFF);
//
//    // Cài đặt địa chỉ hàng (Row Address Set)
//    ST7789_SendCommand(0x2B);
//    ST7789_SendData(y0 >> 8);
//    ST7789_SendData(y0 & 0xFF);
//    ST7789_SendData(y1 >> 8);
//    ST7789_SendData(y1 & 0xFF);
//
//    // Lệnh chuẩn bị ghi RAM (Memory Write) - Ngay sau lệnh này là đẩy Data Pixel
//    ST7789_SendCommand(0x2C);
//}

void ILI9341_SendCommand(uint8_t cmd) {
    ILI9341_DC_CMD();       // Kéo DC xuống mức 0
    ILI9341_CS_SELECT();    // Kéo CS xuống mức 0
    SPI_SendByte(cmd);
    ILI9341_CS_DESELECT();  // Trả CS lên mức 1
}

void ILI9341_SendData(uint8_t data) {
    ILI9341_DC_DATA();      // Kéo DC lên mức 1
    ILI9341_CS_SELECT();
    SPI_SendByte(data);
    ILI9341_CS_DESELECT();
}

void ILI9341_Init(void) {
    // 1. Hard Reset phần cứng
    ILI9341_CS_DESELECT();
    ILI9341_RST_HIGH();
    Delay_ms(10);
    ILI9341_RST_LOW();
    Delay_ms(20);
    ILI9341_RST_HIGH();
    Delay_ms(120); // Chờ IC ổn định

    // 2. Software Reset (Khuyên dùng trên ILI9341)
    ILI9341_SendCommand(0x01);
    Delay_ms(100);

    // 3. Chuỗi cấu hình Nguồn & VCOM (BẮT BUỘC cho Proteus)
    ILI9341_SendCommand(0xC0); // Power Control 1
    ILI9341_SendData(0x23);    // VRH[5:0]

    ILI9341_SendCommand(0xC1); // Power Control 2
    ILI9341_SendData(0x10);    // SAP[2:0];BT[3:0]

    ILI9341_SendCommand(0xC5); // VCOM Control 1
    ILI9341_SendData(0x3E);
    ILI9341_SendData(0x28);

    ILI9341_SendCommand(0xC7); // VCOM Control 2
    ILI9341_SendData(0x86);

    // 4. Memory Access Control (MADCTL)
    ILI9341_SendCommand(0x36);
    ILI9341_SendData(0x48);    // 0x48: Định dạng BGR, quét dọc (Chuẩn của ILI9341 Proteus)

    // 5. Pixel Format (16-bit RGB565)
    ILI9341_SendCommand(0x3A);
    ILI9341_SendData(0x55);    // 0x55 thay vì 0x05 của ST7789

    // 6. Normal Display (Tắt Inversion)
    ILI9341_SendCommand(0x20);

    // 7. Thoát chế độ ngủ & Bật hiển thị
    ILI9341_SendCommand(0x11); // Sleep Out
    Delay_ms(120);

    ILI9341_SendCommand(0x29); // Display ON
    Delay_ms(20);


    ILI9341_BLK_ON();
}

void ILI9341_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

    ILI9341_SendCommand(0x2A);
    ILI9341_SendData(x0 >> 8);
    ILI9341_SendData(x0 & 0xFF);
    ILI9341_SendData(x1 >> 8);
    ILI9341_SendData(x1 & 0xFF);


    ILI9341_SendCommand(0x2B);
    ILI9341_SendData(y0 >> 8);
    ILI9341_SendData(y0 & 0xFF);
    ILI9341_SendData(y1 >> 8);
    ILI9341_SendData(y1 & 0xFF);


    ILI9341_SendCommand(0x2C);
}
void ILI9341_FillColor(uint16_t color) {
    uint8_t color_high = color >> 8;
    uint8_t color_low = color & 0xFF;

    uint8_t dma_buffer[4800];
    for(uint16_t i = 0; i < 4800; i += 2) {
        dma_buffer[i] = color_high;
        dma_buffer[i+1] = color_low;
    }

    ILI9341_SetWindow(0, 0, 239, 319);

    ILI9341_DC_DATA();
    ILI9341_CS_SELECT();


    for(uint16_t chunk = 0; chunk < 32; chunk++) {
        spi1_dma_transmit(dma_buffer, 4800);
        spi1_dma_wait_complete();
    }

    ILI9341_CS_DESELECT();
}
