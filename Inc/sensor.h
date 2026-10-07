
#ifndef SENSOR_H_
#define SENSOR_H_

#include <stdbool.h>
#include "stm32f1xx.h"

#define AFIOEN		(1U<<0)
#define GPIOAEN 	(1U<<2)


void sensor_exti_init(void);
bool Is_Door_Open(void);
bool Is_IR_Triggered(void);

#endif /* SENSOR_H_ */
