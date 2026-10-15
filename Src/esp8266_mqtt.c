#include "esp8266_mqtt.h"
#include "uart.h"
#include "relay.h"
#include <string.h>
#include <stdio.h>

volatile uint8_t people_count = 0;
volatile bool flag_count_changed = false;

static char tx_dma_buffer[32];

void ESP8266_MQTT_Init(void) {
    UART1_Clear_Rx_Buffer();
    UART1_Clear_Data_Flag();
}

void ESP8266_MQTT_Update(void) {
    GPIOC->ODR &= ~(1U << 13); // LED Status

    // --- 1. NHẬN LỆNH TỪ ESP8266 ---
    if (UART1_Is_Data_Ready()) {
        char* rx_buf = (char*)UART1_Get_Rx_Buffer();
        char* ptr;

        // Xử lý lệnh RELAY cũ
        if (strstr(rx_buf, "CMD:ON") != NULL) {
            relay_set_state(RELAY_ON);
            UART1_Clear_Rx_Buffer();
        }
        else if (strstr(rx_buf, "CMD:OFF") != NULL) {
            relay_set_state(RELAY_OFF);
            UART1_Clear_Rx_Buffer();
        }

        else if ((ptr = strstr(rx_buf, "CMD:SET_CNT:")) != NULL) {
            ptr += 12; // Dịch con trỏ qua chuỗi "CMD:SET_CNT:"
            uint8_t new_count = 0;
            while (*ptr >= '0' && *ptr <= '9') {
                new_count = new_count * 10 + (*ptr - '0');
                ptr++;
            }
            people_count = new_count;
            flag_count_changed = true; // Kích cờ báo cáo lại lên APP để xác nhận
            UART1_Clear_Rx_Buffer();
        }

        // Chống tràn
        if (strlen(rx_buf) > (RX_RING_SIZE - 20)) {
            UART1_Clear_Rx_Buffer();
        }
    }

    // --- 2. GỬI TRẠNG THÁI LÊN ESP8266 ---
    // Gửi trạng thái Relay
    if (flag_relay_changed == true) {
        if (current_relay_state == RELAY_ON) {
            UART1_DMA_Transmit((uint8_t*)"STS:ON\n", 7);
        } else {
            UART1_DMA_Transmit((uint8_t*)"STS:OFF\n", 8);
        }
        flag_relay_changed = false;
    }

    // Gửi trạng thái Số lượng người
    if (flag_count_changed == true) {
        uint16_t len = snprintf(tx_dma_buffer, sizeof(tx_dma_buffer), "STS:CNT:%u\n", people_count);
        UART1_DMA_Transmit((uint8_t*)tx_dma_buffer, len);
        flag_count_changed = false;
    }
}
