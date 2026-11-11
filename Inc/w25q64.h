/*
 * w25q64.h
 *
 *  Created on: Oct 8, 2026
 *      Author: PHU
 */

#ifndef W25Q64_H_
#define W25Q64_H_

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>

//thong so chip 
#define W25Q64_PAGE_SIZE          256           /* 1 Page = 256 bytes                */
#define W25Q64_SECTOR_SIZE        4096          /* 1 Sector = 4KB = 16 pages         */
#define W25Q64_BLOCK_32K_SIZE     32768         /* 1 Block 32KB                      */
#define W25Q64_BLOCK_64K_SIZE     65536         /* 1 Block 64KB = 16 sectors         */
#define W25Q64_TOTAL_SIZE         (8*1024*1024) /* 8MB = 64Mbit                      */
#define W25Q64_NUM_SECTORS        2048          /* Tổng số sectors                   */
#define W25Q64_NUM_PAGES          32768         /* Tổng số pages                     */

/*
 BẢNG LỆNH SPI CỦA W25Q64 (INSTRUCTION SET)
/* --- Lệnh đọc/ghi trạng thái --- */

#define W25Q64_CMD_WRITE_ENABLE         0x06  /* Cho phép ghi                   */
#define W25Q64_CMD_WRITE_DISABLE        0x04  /* Cấm ghi                        */
#define W25Q64_CMD_READ_STATUS_REG1     0x05  /* Đọc Status Register 1          */
#define W25Q64_CMD_READ_STATUS_REG2     0x35  /* Đọc Status Register 2          */
#define W25Q64_CMD_WRITE_STATUS_REG     0x01  /* Ghi Status Register            */

/* --- Lệnh đọc dữ liệu --- */
#define W25Q64_CMD_READ_DATA            0x03  /* Đọc thường (max 50MHz)         */
#define W25Q64_CMD_FAST_READ            0x0B  /* Đọc nhanh (cần dummy byte)     */

/* --- Lệnh ghi dữ liệu --- */
#define W25Q64_CMD_PAGE_PROGRAM         0x02  /* Ghi 1 page (max 256 bytes)     */

/* --- Lệnh xóa --- */
#define W25Q64_CMD_SECTOR_ERASE_4K      0x20  /* Xóa 1 Sector 4KB              */
#define W25Q64_CMD_BLOCK_ERASE_32K      0x52  /* Xóa 1 Block 32KB              */
#define W25Q64_CMD_BLOCK_ERASE_64K      0xD8  /* Xóa 1 Block 64KB              */
#define W25Q64_CMD_CHIP_ERASE           0xC7  /* Xóa toàn bộ chip              */

/* --- Lệnh nhận dạng --- */
#define W25Q64_CMD_JEDEC_ID             0x9F  /* Đọc JEDEC ID (Manufacturer)   */
#define W25Q64_CMD_READ_UNIQUE_ID       0x4B  /* Đọc Unique ID                 */
#define W25Q64_CMD_DEVICE_ID            0xAB  /* Đọc Device ID                 */
#define W25Q64_CMD_MFTR_DEVICE_ID       0x90  /* Manufacturer + Device ID      */

/* --- Lệnh điều khiển nguồn --- */
#define W25Q64_CMD_POWER_DOWN           0xB9  /* Vào chế độ tiết kiệm năng lượng */
#define W25Q64_CMD_RELEASE_POWER_DOWN   0xAB  /* Thoát chế độ tiết kiệm         */

/* --- Bit mask Status Register 1 --- */
#define W25Q64_STATUS_BUSY              0x01  /* Bit BUSY: Chip đang xử lý     */
#define W25Q64_STATUS_WEL               0x02  /* Bit WEL: Write Enable Latch   */

/* --- JEDEC ID kỳ vọng của W25Q64 --- */
#define W25Q64_JEDEC_MANUFACTURER       0xEF  /* Winbond                        */
#define W25Q64_JEDEC_MEMORY_TYPE        0x40
#define W25Q64_JEDEC_CAPACITY           0x17  /* 64Mbit                         */

//lenh dieu khien chan CS

#define W25Q64_CS_LOW()     (GPIOB->BRR  = (1U << 12))  /* Kéo CS xuống 0 */
#define W25Q64_CS_HIGH()    (GPIOB->BSRR = (1U << 12))  /* Kéo CS lên 1   */









/* --- Khởi tạo --- */
void W25Q64_Init(void);                 /* Khởi tạo SPI2 + GPIO */

/* --- Nhận dạng chip --- */
uint32_t W25Q64_ReadJEDEC_ID(void);     /* Trả về 3 byte: Manufacturer + Type + Capacity (Kỳ vọng: 0xEF4017) */
uint16_t W25Q64_ReadDeviceID(void);     /* Trả về Manufacturer + DeviceID */

/* --- Đọc dữ liệu --- */
void W25Q64_ReadData(uint32_t address, uint8_t *buffer, uint32_t length);
void W25Q64_FastRead(uint32_t address, uint8_t *buffer, uint32_t length);

/* --- Ghi dữ liệu --- */
void W25Q64_WritePage(uint32_t address, const uint8_t *data, uint16_t length);
void W25Q64_WriteData(uint32_t address, const uint8_t *data, uint32_t length);

/* --- Xóa --- */
void W25Q64_EraseSector(uint32_t sector_address);    /* Xóa 4KB Sector   */
void W25Q64_EraseBlock32K(uint32_t block_address);   /* Xóa 32KB Block   */
void W25Q64_EraseBlock64K(uint32_t block_address);   /* Xóa 64KB Block   */
void W25Q64_EraseChip(void);                         /* Xóa toàn bộ chip */

/* --- Trạng thái --- */
void W25Q64_WriteEnable(void);
void W25Q64_WriteDisable(void);
void W25Q64_WaitBusy(void);
uint8_t W25Q64_ReadStatusReg1(void);

/* --- Nguồn --- */
void W25Q64_PowerDown(void);
void W25Q64_WakeUp(void);

void W25Q64_Init(void);




#endif /* W25Q64_H_ */
