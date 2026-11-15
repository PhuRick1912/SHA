#include "w25q64_read.h"

#define GPIOBEN (1U << 3)
#define SPI2EN (1U << 14)

#define SPI_CR1_CPHA (1U << 0)
#define SPI_CR1_CPOL (1U << 1)
#define SPI_CR1_MSTR (1U << 2)
#define SPI_CR1_BR_DIV4 (1U << 3)
#define SPI_CR1_SPE (1U << 6)
#define SPI_CR1_LSBFIRST (1U << 7)
#define SPI_CR1_SSI (1U << 8)
#define SPI_CR1_SSM (1U << 9)
#define SPI_CR1_DFF (1U << 11)

#define SPI_SR_RXNE (1U << 0)
#define SPI_SR_TXE (1U << 1)

uint8_t W25Q64_SPI_TransferByte(uint8_t tx_data) {
    while (!(SPI2->SR & SPI_SR_TXE)) {}
    SPI2->DR = tx_data;
    while (!(SPI2->SR & SPI_SR_RXNE)) {}
    return (uint8_t)SPI2->DR;
}

static void SPI2_ReceiveBytes(uint8_t *buffer, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        buffer[i] = W25Q64_SPI_TransferByte(0xFF);
    }
}

void W25Q64_Init(void) {
    RCC->APB2ENR |= GPIOBEN;
    RCC->APB1ENR |= SPI2EN;
    /* PB12(CS), PB13(SCK), PB14(MISO), PB15(MOSI) */
    GPIOB->CRH &= ~(0xFFFFU << 16);
    GPIOB->CRH |= (0x3U << 16) | (0xBU << 20) | (0x4U << 24) | (0xBU << 28);
    W25Q64_CS_HIGH();
    SPI2->CR1 = 0;
    SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_BR_DIV4;
    SPI2->CR1 |= SPI_CR1_SPE;
    (void)SPI2->DR;
    (void)SPI2->SR;
    W25Q64_WakeUp();
    volatile uint32_t cnt = 1000;
    while (cnt--) {}
}

uint8_t W25Q64_ReadStatusReg1(void) {
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_READ_STATUS_REG1);
    uint8_t status = W25Q64_SPI_TransferByte(0xFF);
    W25Q64_CS_HIGH();
    return status;
}

void W25Q64_WaitBusy(void) {
    while (W25Q64_ReadStatusReg1() & W25Q64_STATUS_BUSY) {}
}

void W25Q64_PowerDown(void) {
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_POWER_DOWN);
    W25Q64_CS_HIGH();
}

void W25Q64_WakeUp(void) {
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_RELEASE_POWER_DOWN);
    W25Q64_CS_HIGH();
    /* Đợi tRES1 (~3us) để W25Q64 thoát hẳn khỏi Power-Down */
    volatile uint32_t cnt = 300;
    while (cnt--) {}
}

uint32_t W25Q64_ReadJEDEC_ID(void) {
    uint32_t id = 0;
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_JEDEC_ID);
    id = (uint32_t)W25Q64_SPI_TransferByte(0xFF) << 16;
    id |= (uint32_t)W25Q64_SPI_TransferByte(0xFF) << 8;
    id |= (uint32_t)W25Q64_SPI_TransferByte(0xFF);
    W25Q64_CS_HIGH();
    return id;
}

uint16_t W25Q64_ReadDeviceID(void) {
    uint16_t id = 0;
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_MFTR_DEVICE_ID);
    W25Q64_SPI_TransferByte(0x00);
    W25Q64_SPI_TransferByte(0x00);
    W25Q64_SPI_TransferByte(0x00);
    id = (uint16_t)W25Q64_SPI_TransferByte(0xFF) << 8;
    id |= (uint16_t)W25Q64_SPI_TransferByte(0xFF);
    W25Q64_CS_HIGH();
    return id;
}

void W25Q64_ReadData(uint32_t address, uint8_t *buffer, uint32_t length) {
    if (length == 0) {
        return;
    }
    W25Q64_WaitBusy();
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_READ_DATA);
    W25Q64_SPI_TransferByte((address >> 16) & 0xFF);
    W25Q64_SPI_TransferByte((address >> 8) & 0xFF);
    W25Q64_SPI_TransferByte(address & 0xFF);
    SPI2_ReceiveBytes(buffer, length);
    W25Q64_CS_HIGH();
}

void W25Q64_FastRead(uint32_t address, uint8_t *buffer, uint32_t length) {
    if (length == 0) {
        return;
    }
    W25Q64_WaitBusy();
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_FAST_READ);
    W25Q64_SPI_TransferByte((address >> 16) & 0xFF);
    W25Q64_SPI_TransferByte((address >> 8) & 0xFF);
    W25Q64_SPI_TransferByte(address & 0xFF);
    W25Q64_SPI_TransferByte(0xFF);
    SPI2_ReceiveBytes(buffer, length);
    W25Q64_CS_HIGH();
}
