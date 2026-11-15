#include "sha_icon_lut.h"
#include "w25q64_read.h"
#include "spi1_lcd_st7789.h"

#define SZ_RGB565(w, h) ((uint32_t)(w) * (uint32_t)(h) * 2UL)

#define ADDR_WIFI_CONNECTED (W25Q64_UI_BASE_ADDR)
#define ADDR_WIFI_DISCONNECTED (ADDR_WIFI_CONNECTED + SZ_RGB565(ICON_WIFI_W, ICON_WIFI_H))
#define ADDR_LOCATION (ADDR_WIFI_DISCONNECTED + SZ_RGB565(ICON_WIFI_W, ICON_WIFI_H))
#define ADDR_PEOPLE (ADDR_LOCATION + SZ_RGB565(ICON_LOCATION_W, ICON_LOCATION_H))

static const ImageAsset_t s_sys_icon_lut[SYS_ICON_COUNT] = { { ADDR_WIFI_CONNECTED, ICON_WIFI_W, ICON_WIFI_H }, { ADDR_WIFI_DISCONNECTED, ICON_WIFI_W, ICON_WIFI_H }, { ADDR_LOCATION, ICON_LOCATION_W, ICON_LOCATION_H }, { ADDR_PEOPLE, ICON_PEOPLE_W, ICON_PEOPLE_H } };

/* 24 mã thời tiết trong thư mục ICON/weather_icons/day & night */
static const uint16_t s_weather_codes[WEATHER_ICON_COUNT] = { 113, 116, 119, 122, 125, 143, 149, 152, 176, 200, 248, 263, 266, 293, 296, 299, 302, 305, 308, 353, 356, 359, 386, 389 };

bool SHA_Icon_GetSystemAsset(SysIconId_t id, ImageAsset_t *out_asset) {
    if ((id >= SYS_ICON_COUNT) || (out_asset == (void *)0)) {
        return false;
    }
    *out_asset = s_sys_icon_lut[id];
    return true;
}

bool SHA_Icon_GetWeatherAsset(uint16_t weather_code, uint8_t is_day, ImageAsset_t *out_asset) {
    if (out_asset == (void *)0) {
        return false;
    }
    int32_t left = 0;
    int32_t right = (int32_t)WEATHER_ICON_COUNT - 1;
    int32_t found_idx = -1;
    while (left <= right) {
        int32_t mid = left + ((right - left) >> 1);
        if (s_weather_codes[mid] == weather_code) {
            found_idx = mid;
            break;
        } else if (s_weather_codes[mid] < weather_code) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    if (found_idx < 0) {
        found_idx = 0;
    }
    uint32_t slot_index = (is_day != 0U) ? (uint32_t)found_idx : ((uint32_t)WEATHER_ICON_COUNT + (uint32_t)found_idx);
    out_asset->flash_addr = W25Q64_WEATHER_BASE_ADDR + (slot_index * ICON_WEATHER_BYTE_SIZE);
    out_asset->width = ICON_WEATHER_W;
    out_asset->height = ICON_WEATHER_H;
    return true;
}

#define FLASH_READ_CHUNK_BYTES 1024U

void SHA_Icon_DrawFromFlash(uint16_t x, uint16_t y, const ImageAsset_t *asset) {
    if (asset == (void *)0) {
        return;
    }
    static uint8_t s_chunk_buf[FLASH_READ_CHUNK_BYTES];
    uint32_t total_bytes = (uint32_t)asset->width * (uint32_t)asset->height * 2UL;
    uint32_t cur_addr = asset->flash_addr;
    /* Đảm bảo W25Q64 luôn thức trước khi đọc dữ liệu */
    W25Q64_WakeUp();
    ST7789_SetWindow(x, y, x + asset->width - 1U, y + asset->height - 1U);
    while (total_bytes > 0UL) {
        uint32_t chunk = (total_bytes > FLASH_READ_CHUNK_BYTES) ? FLASH_READ_CHUNK_BYTES : total_bytes;
        W25Q64_ReadData(cur_addr, s_chunk_buf, chunk);
        ST7789_WriteDataBuffer(s_chunk_buf, chunk);
        cur_addr += chunk;
        total_bytes -= chunk;
    }
}
