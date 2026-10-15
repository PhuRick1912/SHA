/*
 * esp8266.h
 *
 *  Created on: Oct 6, 2026
 *      Author: PHU
 */

#ifndef ESP8266_MQTT_H_
#define ESP8266_MQTT_H_

#include <stdint.h>
#include <stdbool.h>
extern volatile uint8_t people_count;
extern volatile bool flag_count_changed;
// Định nghĩa Timeout (ms) cho từng loại lệnh
#define AT_CMD_TIMEOUT_MS       2000
#define WIFI_CONN_TIMEOUT_MS    10000
#define MQTT_CONN_TIMEOUT_MS    10000



void ESP8266_MQTT_Init(void);
void ESP8266_MQTT_Update(void);

#endif /* ESP8266_MQTT_H_ */
