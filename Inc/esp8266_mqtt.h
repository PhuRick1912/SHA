#ifndef ESP8266_MQTT_H_
#define ESP8266_MQTT_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t code;
    uint8_t is_day;
    int8_t temp_c;
    uint8_t uv;
} Weather_Data_t;

extern volatile uint8_t people_count;
extern volatile bool flag_count_changed;
extern volatile Weather_Data_t current_weather;
extern volatile bool flag_weather_changed;

void ESP8266_MQTT_Init(void);
void ESP8266_MQTT_Update(void);
void LCD_Draw_Static_Header(void);
void LCD_Update_Relays_If_Changed(void);
void LCD_Update_People_If_Changed(void);
void LCD_Update_Weather_If_Changed(void);

#endif /* ESP8266_MQTT_H_ */
