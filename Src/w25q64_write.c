#include "w25q64_write.h"

static void SPI2_TransmitBytes(const uint8_t *data, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        W25Q64_SPI_TransferByte(data[i]);
    }
}

void W25Q64_WriteEnable(void) {
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_WRITE_ENABLE);
    W25Q64_CS_HIGH();
}

void W25Q64_WriteDisable(void) {
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_WRITE_DISABLE);
    W25Q64_CS_HIGH();
}

void W25Q64_WritePage(uint32_t address, const uint8_t *data, uint16_t length) {
    if (length == 0 || length > W25Q64_PAGE_SIZE) {
        return;
    }
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_PAGE_PROGRAM);
    W25Q64_SPI_TransferByte((address >> 16) & 0xFF);
    W25Q64_SPI_TransferByte((address >> 8) & 0xFF);
    W25Q64_SPI_TransferByte(address & 0xFF);
    SPI2_TransmitBytes(data, length);
    W25Q64_CS_HIGH();
    W25Q64_WaitBusy();
}

void W25Q64_WriteData(uint32_t address, const uint8_t *data, uint32_t length) {
    uint32_t offset = 0;
    while (offset < length) {
        uint16_t page_remain = W25Q64_PAGE_SIZE - (uint16_t)(address % W25Q64_PAGE_SIZE);
        uint16_t write_len = (length - offset > page_remain) ? page_remain : (uint16_t)(length - offset);
        W25Q64_WritePage(address, data + offset, write_len);
        address += write_len;
        offset += write_len;
    }
}

void W25Q64_EraseSector(uint32_t sector_address) {
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_SECTOR_ERASE_4K);
    W25Q64_SPI_TransferByte((sector_address >> 16) & 0xFF);
    W25Q64_SPI_TransferByte((sector_address >> 8) & 0xFF);
    W25Q64_SPI_TransferByte(sector_address & 0xFF);
    W25Q64_CS_HIGH();
    W25Q64_WaitBusy();
}

void W25Q64_EraseBlock32K(uint32_t block_address) {
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_BLOCK_ERASE_32K);
    W25Q64_SPI_TransferByte((block_address >> 16) & 0xFF);
    W25Q64_SPI_TransferByte((block_address >> 8) & 0xFF);
    W25Q64_SPI_TransferByte(block_address & 0xFF);
    W25Q64_CS_HIGH();
    W25Q64_WaitBusy();
}

void W25Q64_EraseBlock64K(uint32_t block_address) {
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_BLOCK_ERASE_64K);
    W25Q64_SPI_TransferByte((block_address >> 16) & 0xFF);
    W25Q64_SPI_TransferByte((block_address >> 8) & 0xFF);
    W25Q64_SPI_TransferByte(block_address & 0xFF);
    W25Q64_CS_HIGH();
    W25Q64_WaitBusy();
}

void W25Q64_EraseChip(void) {
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();
    W25Q64_CS_LOW();
    W25Q64_SPI_TransferByte(W25Q64_CMD_CHIP_ERASE);
    W25Q64_CS_HIGH();
    W25Q64_WaitBusy();
}
