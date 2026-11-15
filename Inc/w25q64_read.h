#ifndef W25Q64_READ_H_
#define W25Q64_READ_H_

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>

#define W25Q64_PAGE_SIZE 256
#define W25Q64_SECTOR_SIZE 4096
#define W25Q64_BLOCK_32K_SIZE 32768
#define W25Q64_BLOCK_64K_SIZE 65536
#define W25Q64_TOTAL_SIZE (8 * 1024 * 1024)
#define W25Q64_NUM_SECTORS 2048
#define W25Q64_NUM_PAGES 32768

#define W25Q64_CMD_READ_STATUS_REG1 0x05
#define W25Q64_CMD_READ_STATUS_REG2 0x35
#define W25Q64_CMD_READ_DATA 0x03
#define W25Q64_CMD_FAST_READ 0x0B

#define W25Q64_CMD_JEDEC_ID 0x9F
#define W25Q64_CMD_READ_UNIQUE_ID 0x4B
#define W25Q64_CMD_DEVICE_ID 0xAB
#define W25Q64_CMD_MFTR_DEVICE_ID 0x90
#define W25Q64_CMD_POWER_DOWN 0xB9
#define W25Q64_CMD_RELEASE_POWER_DOWN 0xAB

#define W25Q64_STATUS_BUSY 0x01
#define W25Q64_JEDEC_MANUFACTURER 0xEF
#define W25Q64_JEDEC_MEMORY_TYPE 0x40
#define W25Q64_JEDEC_CAPACITY 0x17

#define W25Q64_CS_LOW() (GPIOB->BRR = (1U << 12))
#define W25Q64_CS_HIGH() (GPIOB->BSRR = (1U << 12))

uint8_t W25Q64_SPI_TransferByte(uint8_t tx_data);
void W25Q64_Init(void);
void W25Q64_PowerDown(void);
void W25Q64_WakeUp(void);
uint8_t W25Q64_ReadStatusReg1(void);
void W25Q64_WaitBusy(void);
uint32_t W25Q64_ReadJEDEC_ID(void);
uint16_t W25Q64_ReadDeviceID(void);
void W25Q64_ReadData(uint32_t address, uint8_t *buffer, uint32_t length);
void W25Q64_FastRead(uint32_t address, uint8_t *buffer, uint32_t length);

#endif /* W25Q64_READ_H_ */
