#ifndef SHA_ICON_LUT_H_
#define SHA_ICON_LUT_H_

#include <stdint.h>
#include <stdbool.h>

#define W25Q64_UI_BASE_ADDR 0x00001000UL
#define W25Q64_WEATHER_BASE_ADDR 0x00020000UL

#define ICON_WIFI_W 24U
#define ICON_WIFI_H 24U
#define ICON_LOCATION_W 24U
#define ICON_LOCATION_H 24U
#define ICON_PEOPLE_W 50U
#define ICON_PEOPLE_H 50U
#define ICON_WEATHER_W 64U
#define ICON_WEATHER_H 64U
#define ICON_WEATHER_BYTE_SIZE (ICON_WEATHER_W * ICON_WEATHER_H * 2UL)
#define WEATHER_ICON_COUNT 24U

typedef enum {
    SYS_ICON_WIFI_CONNECTED = 0,
    SYS_ICON_WIFI_DISCONNECTED,
    SYS_ICON_LOCATION,
    SYS_ICON_PEOPLE,
    SYS_ICON_COUNT
} SysIconId_t;

typedef struct {
    uint32_t flash_addr;
    uint16_t width;
    uint16_t height;
} ImageAsset_t;

bool SHA_Icon_GetSystemAsset(SysIconId_t id, ImageAsset_t *out_asset);
bool SHA_Icon_GetWeatherAsset(uint16_t weather_code, uint8_t is_day, ImageAsset_t *out_asset);
void SHA_Icon_DrawFromFlash(uint16_t x, uint16_t y, const ImageAsset_t *asset);

#endif /* SHA_ICON_LUT_H_ */
