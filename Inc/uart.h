#ifndef UART_H_
#define UART_H_


#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define RX_BUFFER_SIZE 50




void UART1_DMA_Init(void);
void UART1_DMA_Transmit(uint8_t *data, uint16_t len);
void UART_Process_MQTT_Payload(void);

// Các hàm API để tầng Application lấy dữ liệu
bool UART1_Is_Data_Ready(void);
void UART1_Clear_Data_Flag(void);
uint8_t* UART1_Get_Rx_Buffer(void);
void UART1_Clear_Rx_Buffer(void);


#endif /* UART_H_ */
