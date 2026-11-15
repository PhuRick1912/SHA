#include "esp8266_mqtt.h"
#include "uart.h"
#include "relay.h"
#include "spi1_lcd_st7789.h"
#include "sha_icon_lut.h"
#include "lv_font_shim.h"
#include <string.h>
#include <stdio.h>

extern const lv_font_t arial_20;

volatile uint8_t people_count = 0;
volatile bool flag_count_changed = false;

volatile Weather_Data_t current_weather = { 113, 1, 30, 5 };
volatile bool flag_weather_changed = true;

static char tx_dma_buffer[32];

#define LCD_BG_COLOR 0x0000U
#define LCD_TEXT_COLOR 0xFFFFU
#define LCD_UV_COLOR 0xFFE0U
#define LCD_RELAY_ON_COLOR 0x07E0U
#define LCD_RELAY_OFF_COLOR 0x3186U

#define LCD_WIFI_ICON_X 15U
#define LCD_WIFI_ICON_Y 12U
#define LCD_LOC_ICON_X 110U
#define LCD_LOC_ICON_Y 12U
#define LCD_LOC_TEXT_X 140U
#define LCD_LOC_TEXT_Y 14U

#define LCD_WEATHER_X 20U
#define LCD_WEATHER_Y 70U
#define LCD_TEMP_X 100U
#define LCD_TEMP_Y 80U
#define LCD_UV_X 100U
#define LCD_UV_Y 115U

#define LCD_RELAY_BAR_Y 148U
#define LCD_RELAY_BOX_W 46U
#define LCD_RELAY_BOX_H 18U
#define LCD_RELAY_START_X 14U
#define LCD_RELAY_STEP_X 54U

#define LCD_PEOPLE_TEXT_X 75U
#define LCD_PEOPLE_TEXT_Y 187U
#define LCD_PEOPLE_TEXT_W 40U
#define LCD_PEOPLE_TEXT_H 26U
#define LCD_PEOPLE_ICON_X 120U
#define LCD_PEOPLE_ICON_Y 175U

static uint16_t parse_uint_csv(char **p) {
    uint16_t val = 0;
    while (**p >= '0' && **p <= '9') {
        val = val * 10U + (uint16_t)(**p - '0');
        (*p)++;
    }
    if (**p == ',') {
        (*p)++;
    }
    return val;
}

void ESP8266_MQTT_Init(void) {
    UART1_Clear_Rx_Buffer();
    UART1_Clear_Data_Flag();
}

void LCD_Draw_Static_Header(void) {
    ST7789_FillColor(LCD_BG_COLOR);
    ImageAsset_t asset;
    if (SHA_Icon_GetSystemAsset(SYS_ICON_WIFI_CONNECTED, &asset)) {
        SHA_Icon_DrawFromFlash(LCD_WIFI_ICON_X, LCD_WIFI_ICON_Y, &asset);
    }
    if (SHA_Icon_GetSystemAsset(SYS_ICON_LOCATION, &asset)) {
        SHA_Icon_DrawFromFlash(LCD_LOC_ICON_X, LCD_LOC_ICON_Y, &asset);
    }
    SHA_Font_DrawString(LCD_LOC_TEXT_X, LCD_LOC_TEXT_Y, "TP.HCM", &arial_20, LCD_TEXT_COLOR, LCD_BG_COLOR);
}

void LCD_Update_Relays_If_Changed(void) {
    static uint8_t last_drawn_mask = 0xFFU;
    uint8_t cur_mask = current_relay_mask;
    if (cur_mask == last_drawn_mask) {
        return;
    }
    for (uint8_t i = 0; i < RELAY_COUNT; i++) {
        uint8_t bit = (uint8_t)(1U << i);
        if ((last_drawn_mask == 0xFFU) || (((cur_mask ^ last_drawn_mask) & bit) != 0U)) {
            uint16_t box_x = LCD_RELAY_START_X + (uint16_t)i * LCD_RELAY_STEP_X;
            uint16_t color = (cur_mask & bit) ? LCD_RELAY_ON_COLOR : LCD_RELAY_OFF_COLOR;
            ST7789_FillRect(box_x, LCD_RELAY_BAR_Y, LCD_RELAY_BOX_W, LCD_RELAY_BOX_H, color);
        }
    }
    last_drawn_mask = cur_mask;
}

void LCD_Update_People_If_Changed(void) {
    static uint8_t last_drawn_count = 0xFFU;
    if (last_drawn_count == 0xFFU) {
        ImageAsset_t people_asset;
        if (SHA_Icon_GetSystemAsset(SYS_ICON_PEOPLE, &people_asset)) {
            SHA_Icon_DrawFromFlash(LCD_PEOPLE_ICON_X, LCD_PEOPLE_ICON_Y, &people_asset);
        }
    }
    uint8_t cur_cnt = people_count;
    if (cur_cnt != last_drawn_count) {
        char cnt_str[8];
        snprintf(cnt_str, sizeof(cnt_str), "%02u", (unsigned int)cur_cnt);
        ST7789_FillRect(LCD_PEOPLE_TEXT_X, LCD_PEOPLE_TEXT_Y, LCD_PEOPLE_TEXT_W, LCD_PEOPLE_TEXT_H, LCD_BG_COLOR);
        SHA_Font_DrawString(LCD_PEOPLE_TEXT_X, LCD_PEOPLE_TEXT_Y, cnt_str, &arial_20, LCD_TEXT_COLOR, LCD_BG_COLOR);
        last_drawn_count = cur_cnt;
    }
}

void LCD_Update_Weather_If_Changed(void) {
    if (!flag_weather_changed) {
        return;
    }
    static Weather_Data_t last_drawn = { 0xFFFF, 0xFF, -128, 0xFF };
    Weather_Data_t w = current_weather;
    flag_weather_changed = false;
    if (w.code != last_drawn.code || w.is_day != last_drawn.is_day) {
        ImageAsset_t icon_asset;
        if (SHA_Icon_GetWeatherAsset(w.code, w.is_day, &icon_asset)) {
            SHA_Icon_DrawFromFlash(LCD_WEATHER_X, LCD_WEATHER_Y, &icon_asset);
        }
        last_drawn.code = w.code;
        last_drawn.is_day = w.is_day;
    }
    if (w.temp_c != last_drawn.temp_c) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d °C", (int)w.temp_c);
        ST7789_FillRect(LCD_TEMP_X, LCD_TEMP_Y, 100, 26, LCD_BG_COLOR);
        SHA_Font_DrawString(LCD_TEMP_X, LCD_TEMP_Y, buf, &arial_20, LCD_TEXT_COLOR, LCD_BG_COLOR);
        last_drawn.temp_c = w.temp_c;
    }
    if (w.uv != last_drawn.uv) {
        char buf[16];
        snprintf(buf, sizeof(buf), "UV: %u", (unsigned int)w.uv);
        ST7789_FillRect(LCD_UV_X, LCD_UV_Y, 100, 26, LCD_BG_COLOR);
        SHA_Font_DrawString(LCD_UV_X, LCD_UV_Y, buf, &arial_20, LCD_UV_COLOR, LCD_BG_COLOR);
        last_drawn.uv = w.uv;
    }
}

void ESP8266_MQTT_Update(void) {
    GPIOC->ODR &= ~(1U << 13);
    /* 1. Nhận và bóc tách khung truyền từ ESP8266 */
    if (UART1_Is_Data_Ready()) {
        char* rx_buf = (char*)UART1_Get_Rx_Buffer();
        char* ptr = rx_buf;
        bool matched = false;
        while ((ptr = strstr(ptr, "CMD:R")) != NULL) {
            ptr += 5;
            if (*ptr >= '1' && *ptr <= '4' && *(ptr + 1) == ':') {
                RelayChannel_t ch = (RelayChannel_t)(*ptr - '1');
                ptr += 2;
                if (strncmp(ptr, "ON", 2) == 0 || *ptr == '1') {
                    relay_set_channel(ch, RELAY_ON);
                    matched = true;
                } else if (strncmp(ptr, "OFF", 3) == 0 || *ptr == '0') {
                    relay_set_channel(ch, RELAY_OFF);
                    matched = true;
                }
            }
        }
        if (strstr(rx_buf, "CMD:ALL:ON") != NULL) {
            relay_set_all(RELAY_ON);
            matched = true;
        } else if (strstr(rx_buf, "CMD:ALL:OFF") != NULL) {
            relay_set_all(RELAY_OFF);
            matched = true;
        } else if (strstr(rx_buf, "CMD:ON") != NULL) {
            relay_set_channel(RELAY_CH1, RELAY_ON);
            matched = true;
        } else if (strstr(rx_buf, "CMD:OFF") != NULL) {
            relay_set_channel(RELAY_CH1, RELAY_OFF);
            matched = true;
        }
        if ((ptr = strstr(rx_buf, "CMD:SET_CNT:")) != NULL) {
            ptr += 12;
            uint8_t new_count = 0;
            while (*ptr >= '0' && *ptr <= '9') {
                new_count = new_count * 10 + (*ptr - '0');
                ptr++;
            }
            people_count = new_count;
            flag_count_changed = true;
            matched = true;
        }
        if ((ptr = strstr(rx_buf, "API:")) != NULL) {
            ptr += 4;
            current_weather.code = parse_uint_csv(&ptr);
            current_weather.is_day = (uint8_t)parse_uint_csv(&ptr);
            current_weather.temp_c = (int8_t)parse_uint_csv(&ptr);
            current_weather.uv = (uint8_t)parse_uint_csv(&ptr);
            flag_weather_changed = true;
            matched = true;
        }
        if (matched || (strlen(rx_buf) > (RX_RING_SIZE - 20))) {
            UART1_Clear_Rx_Buffer();
        } else {
            UART1_Clear_Data_Flag();
        }
    }
    /* 2. Gửi trạng thái lên ESP8266 qua DMA1_Channel4 */
    if (flag_relay_changed == true) {
        uint8_t m = current_relay_mask;
        uint16_t len = (uint16_t)snprintf(tx_dma_buffer, sizeof(tx_dma_buffer), "STS:REL:%u,%u,%u,%u\n", (m >> 0) & 1U, (m >> 1) & 1U, (m >> 2) & 1U, (m >> 3) & 1U);
        UART1_DMA_Transmit((uint8_t*)tx_dma_buffer, len);
        flag_relay_changed = false;
    } else if (flag_count_changed == true) {
        uint16_t len = (uint16_t)snprintf(tx_dma_buffer, sizeof(tx_dma_buffer), "STS:CNT:%u\n", people_count);
        UART1_DMA_Transmit((uint8_t*)tx_dma_buffer, len);
        flag_count_changed = false;
    }
    /* 3. Đồng bộ giao diện LCD */
    LCD_Update_Relays_If_Changed();
    LCD_Update_People_If_Changed();
    LCD_Update_Weather_If_Changed();
}
