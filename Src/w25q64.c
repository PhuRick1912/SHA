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
#define SPI_SR_BSY          (1U << 7)



static uint8_t SPI2_TransferByte(uint8_t tx_data)
{
    /* Đợi TXE (TX buffer trống) */
    while (!(SPI2->SR & SPI_SR_TXE)) {}

    /* Ghi data vào Data Register => phần cứng tự bắn ra MOSI */
    SPI2->DR = tx_data;

    /* Đợi RXNE (RX buffer có data) => đã nhận xong 1 byte từ MISO */
    while (!(SPI2->SR & SPI_SR_RXNE)) {}

    /* Đọc data nhận được (đồng thời xóa cờ RXNE) */
    return (uint8_t)SPI2->DR;
}


static void SPI2_TransmitBytes(const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        SPI2_TransferByte(data[i]);
    }
}


static void SPI2_ReceiveBytes(uint8_t *buffer, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        buffer[i] = SPI2_TransferByte(0xFF);
    }
}

void w25q64_init (void){
	RCC->APB1ENR |= SPI2EN | GPIOBEN;
	
	GPIOB->CRH &= ~(0xFFFFU << 16);
	GPIOB->CRH |=  (0x3U << 16)   /* PB12: CS   */
	           |  (0xBU << 20)   /* PB13: SCK  */
	           |  (0x4U << 24)   /* PB14: MISO */
	           |  (0xBU << 28);  /* PB15: MOSI */
	
	W25Q64_CS_HIGH();

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
/* ============================================================
 *  LỆNH TRẠNG THÁI & ĐIỀU KHIỂN
 * ============================================================ */

void W25Q64_WriteEnable(void)
{
    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_WRITE_ENABLE);
    W25Q64_CS_HIGH();
}

void W25Q64_WriteDisable(void)
{
    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_WRITE_DISABLE);
    W25Q64_CS_HIGH();
}

uint8_t W25Q64_ReadStatusReg1(void)
{
    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_READ_STATUS_REG1);
    uint8_t status = SPI2_TransferByte(0xFF);
    W25Q64_CS_HIGH();
    return status;
}

void W25Q64_WaitBusy(void)
{
    while (W25Q64_ReadStatusReg1() & W25Q64_STATUS_BUSY) {}
}

void W25Q64_PowerDown(void)
{
    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_POWER_DOWN);
    W25Q64_CS_HIGH();
}

void W25Q64_WakeUp(void)
{
    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_RELEASE_POWER_DOWN);
    W25Q64_CS_HIGH();
}

/* ============================================================
 *  NHẬN DẠNG CHIP
 * ============================================================ */

uint32_t W25Q64_ReadJEDEC_ID(void)
{
    uint32_t id = 0;

    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_JEDEC_ID);
    id  = (uint32_t)SPI2_TransferByte(0xFF) << 16;  /* Manufacturer ID (0xEF) */
    id |= (uint32_t)SPI2_TransferByte(0xFF) << 8;   /* Memory Type     (0x40) */
    id |= (uint32_t)SPI2_TransferByte(0xFF);         /* Capacity        (0x17) */
    W25Q64_CS_HIGH();

    return id;
}

uint16_t W25Q64_ReadDeviceID(void)
{
    uint16_t id = 0;

    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_MFTR_DEVICE_ID);
    SPI2_TransferByte(0x00);
    SPI2_TransferByte(0x00);
    SPI2_TransferByte(0x00);
    id  = (uint16_t)SPI2_TransferByte(0xFF) << 8;
    id |= (uint16_t)SPI2_TransferByte(0xFF);
    W25Q64_CS_HIGH();

    return id;
}

/* ============================================================
 *  ĐỌC DỮ LIỆU
 * ============================================================ */

void W25Q64_ReadData(uint32_t address, uint8_t *buffer, uint32_t length)
{
    if (length == 0) return;

    W25Q64_WaitBusy();

    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_READ_DATA);
    SPI2_TransferByte((address >> 16) & 0xFF);
    SPI2_TransferByte((address >> 8)  & 0xFF);
    SPI2_TransferByte(address & 0xFF);

    SPI2_ReceiveBytes(buffer, length);
    W25Q64_CS_HIGH();
}

void W25Q64_FastRead(uint32_t address, uint8_t *buffer, uint32_t length)
{
    if (length == 0) return;

    W25Q64_WaitBusy();

    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_FAST_READ);
    SPI2_TransferByte((address >> 16) & 0xFF);
    SPI2_TransferByte((address >> 8)  & 0xFF);
    SPI2_TransferByte(address & 0xFF);
    SPI2_TransferByte(0xFF); /* Dummy byte */

    SPI2_ReceiveBytes(buffer, length);
    W25Q64_CS_HIGH();
}

/* ============================================================
 *  GHI DỮ LIỆU
 * ============================================================ */

void W25Q64_WritePage(uint32_t address, const uint8_t *data, uint16_t length)
{
    if (length == 0 || length > W25Q64_PAGE_SIZE) return;

    W25Q64_WaitBusy();
    W25Q64_WriteEnable();

    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_PAGE_PROGRAM);
    SPI2_TransferByte((address >> 16) & 0xFF);
    SPI2_TransferByte((address >> 8)  & 0xFF);
    SPI2_TransferByte(address & 0xFF);

    SPI2_TransmitBytes(data, length);
    W25Q64_CS_HIGH();

    W25Q64_WaitBusy();
}

void W25Q64_WriteData(uint32_t address, const uint8_t *data, uint32_t length)
{
    uint32_t offset = 0;

    while (offset < length) {
        uint16_t page_remain = W25Q64_PAGE_SIZE - (uint16_t)(address % W25Q64_PAGE_SIZE);
        uint16_t write_len = (length - offset > page_remain) ? page_remain : (uint16_t)(length - offset);

        W25Q64_WritePage(address, data + offset, write_len);

        address += write_len;
        offset  += write_len;
    }
}

/* ============================================================
 *  XÓA DỮ LIỆU
 * ============================================================ */

void W25Q64_EraseSector(uint32_t sector_address)
{
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();

    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_SECTOR_ERASE_4K);
    SPI2_TransferByte((sector_address >> 16) & 0xFF);
    SPI2_TransferByte((sector_address >> 8)  & 0xFF);
    SPI2_TransferByte(sector_address & 0xFF);
    W25Q64_CS_HIGH();

    W25Q64_WaitBusy();
}

void W25Q64_EraseBlock32K(uint32_t block_address)
{
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();

    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_BLOCK_ERASE_32K);
    SPI2_TransferByte((block_address >> 16) & 0xFF);
    SPI2_TransferByte((block_address >> 8)  & 0xFF);
    SPI2_TransferByte(block_address & 0xFF);
    W25Q64_CS_HIGH();

    W25Q64_WaitBusy();
}

void W25Q64_EraseBlock64K(uint32_t block_address)
{
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();

    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_BLOCK_ERASE_64K);
    SPI2_TransferByte((block_address >> 16) & 0xFF);
    SPI2_TransferByte((block_address >> 8)  & 0xFF);
    SPI2_TransferByte(block_address & 0xFF);
    W25Q64_CS_HIGH();

    W25Q64_WaitBusy();
}

void W25Q64_EraseChip(void)
{
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();

    W25Q64_CS_LOW();
    SPI2_TransferByte(W25Q64_CMD_CHIP_ERASE);
    W25Q64_CS_HIGH();

    W25Q64_WaitBusy();
}


