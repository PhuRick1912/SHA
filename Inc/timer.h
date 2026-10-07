/*
 * timer.h
 *
 *  Created on: Oct 4, 2026
 *      Author: PHU
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "stm32f1xx.h"

void SysTick_Init(void);
uint32_t Get_System_Tick(void);

#endif /* TIMER_H_ */
