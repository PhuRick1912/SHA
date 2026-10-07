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

// Định nghĩa Timeout (ms) cho từng loại lệnh
#define AT_CMD_TIMEOUT_MS       2000
#define WIFI_CONN_TIMEOUT_MS    10000
#define MQTT_CONN_TIMEOUT_MS    10000

typedef enum {
    ESP_STATE_INIT = 0,
    ESP_STATE_WAIT_INIT,

    ESP_STATE_WIFI_MODE,
    ESP_STATE_WAIT_WIFI_MODE,

    ESP_STATE_WIFI_CONN,
    ESP_STATE_WAIT_WIFI_CONN,

    ESP_STATE_MQTT_CFG,
    ESP_STATE_WAIT_MQTT_CFG,

    ESP_STATE_MQTT_CONN,
    ESP_STATE_WAIT_MQTT_CONN,

    ESP_STATE_MQTT_SUB,
    ESP_STATE_WAIT_MQTT_SUB,

    ESP_STATE_RUNNING, // Trạng thái hoạt động bình thường, chờ lệnh từ App
    ESP_STATE_ERROR    // Trạng thái lỗi, cần Reset
} ESP_State_t;

void ESP8266_MQTT_Init(void);
void ESP8266_MQTT_Update(void);
void UART_Process_MQTT_Publish(void);
#endif /* ESP8266_MQTT_H_ */
