#ifndef W25Q64_WRITE_H_
#define W25Q64_WRITE_H_

#include "w25q64_read.h"

#define W25Q64_CMD_WRITE_ENABLE 0x06
#define W25Q64_CMD_WRITE_DISABLE 0x04
#define W25Q64_CMD_WRITE_STATUS_REG 0x01
#define W25Q64_CMD_PAGE_PROGRAM 0x02

#define W25Q64_CMD_SECTOR_ERASE_4K 0x20
#define W25Q64_CMD_BLOCK_ERASE_32K 0x52
#define W25Q64_CMD_BLOCK_ERASE_64K 0xD8
#define W25Q64_CMD_CHIP_ERASE 0xC7

#define W25Q64_STATUS_WEL 0x02

void W25Q64_WriteEnable(void);
void W25Q64_WriteDisable(void);
void W25Q64_WritePage(uint32_t address, const uint8_t *data, uint16_t length);
void W25Q64_WriteData(uint32_t address, const uint8_t *data, uint32_t length);
void W25Q64_EraseSector(uint32_t sector_address);
void W25Q64_EraseBlock32K(uint32_t block_address);
void W25Q64_EraseBlock64K(uint32_t block_address);
void W25Q64_EraseChip(void);

#endif /* W25Q64_WRITE_H_ */
