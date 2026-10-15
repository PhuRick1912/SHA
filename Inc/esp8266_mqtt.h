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

void ESP8266_MQTT_Init(void);
void ESP8266_MQTT_Update(void);

#endif /* ESP8266_MQTT_H_ */
