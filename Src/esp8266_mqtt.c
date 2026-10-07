#include "esp8266_mqtt.h"
#include "uart.h"
#include "timer.h"
#include "relay.h"
#include <string.h>

static ESP_State_t current_esp_state;
static uint32_t command_tick;
static uint8_t retry_count = 0;
#define MAX_RETRY 3

void ESP8266_MQTT_Init(void) {
    current_esp_state = ESP_STATE_INIT;
    command_tick = 0;
    retry_count = 0;
}

// Hàm phụ trợ để gửi lệnh và chuyển trạng thái
static void Send_AT_Command(const char* cmd, ESP_State_t next_wait_state) {
    UART1_Clear_Rx_Buffer();
    UART1_Clear_Data_Flag();
    UART1_DMA_Transmit((uint8_t*)cmd, strlen(cmd));
    command_tick = Get_System_Tick();
    current_esp_state = next_wait_state;
}

void ESP8266_MQTT_Update(void) {
    uint32_t current_tick = Get_System_Tick();

    switch (current_esp_state) {

        // --- 1. KIỂM TRA GIAO TIẾP VẬT LÝ ---
        case ESP_STATE_INIT:
            Send_AT_Command("AT\r\n", ESP_STATE_WAIT_INIT);
            break;

        case ESP_STATE_WAIT_INIT:
            if (UART1_Is_Data_Ready()) {
                if (strstr((char*)UART1_Get_Rx_Buffer(), "OK")) {
                    current_esp_state = ESP_STATE_WIFI_MODE;
                    retry_count = 0;
                }
                UART1_Clear_Data_Flag();
            } else if ((current_tick - command_tick) > AT_CMD_TIMEOUT_MS) {
                current_esp_state = ESP_STATE_ERROR;
            }
            break;

        // --- 2. CẤU HÌNH STATION MODE ---
        case ESP_STATE_WIFI_MODE:
            Send_AT_Command("AT+CWMODE=1\r\n", ESP_STATE_WAIT_WIFI_MODE);
            break;

        case ESP_STATE_WAIT_WIFI_MODE:
            if (UART1_Is_Data_Ready()) {
                if (strstr((char*)UART1_Get_Rx_Buffer(), "OK")) {
                    current_esp_state = ESP_STATE_WIFI_CONN;
                }
                UART1_Clear_Data_Flag();
            } else if ((current_tick - command_tick) > AT_CMD_TIMEOUT_MS) {
                current_esp_state = ESP_STATE_ERROR;
            }
            break;

        // --- 3. KẾT NỐI WI-FI ---
        case ESP_STATE_WIFI_CONN:
            Send_AT_Command("AT+CWJAP=\"WIFI_SSID\",\"WIFI_PASSWORD\"\r\n", ESP_STATE_WAIT_WIFI_CONN);
            break;

        case ESP_STATE_WAIT_WIFI_CONN:
            if (UART1_Is_Data_Ready()) {
                char* rx_buf = (char*)UART1_Get_Rx_Buffer();
                if (strstr(rx_buf, "WIFI CONNECTED") && strstr(rx_buf, "OK")) {
                    current_esp_state = ESP_STATE_MQTT_CFG;
                } else if (strstr(rx_buf, "FAIL") || strstr(rx_buf, "ERROR")) {
                    current_esp_state = ESP_STATE_ERROR;
                }
                UART1_Clear_Data_Flag();
            } else if ((current_tick - command_tick) > WIFI_CONN_TIMEOUT_MS) {
                current_esp_state = ESP_STATE_ERROR;
            }
            break;
            // --- 4. CẤU HÌNH THÔNG SỐ MQTT CLIENT ---
                    case ESP_STATE_MQTT_CFG:
                        // Cú pháp: AT+MQTTUSERCFG=<LinkID>,<Scheme>,<"ClientID">,<"Username">,<"Password">,<Cert_key_ID>,<CA_ID>,<"Path">
                        // Scheme = 1 (TCP qua port 1883). Thay "user" và "pass" bằng thông tin từ HiveMQ Credentials.
                        Send_AT_Command("AT+MQTTUSERCFG=0,1,\"STM32_Gateway_01\",\"user\",\"pass\",0,0,\"\"\r\n", ESP_STATE_WAIT_MQTT_CFG);
                        break;

                    case ESP_STATE_WAIT_MQTT_CFG:
                        if (UART1_Is_Data_Ready()) {
                            char* rx_buf = (char*)UART1_Get_Rx_Buffer();
                            if (strstr(rx_buf, "OK") != NULL) {
                                current_esp_state = ESP_STATE_MQTT_CONN;
                            } else if (strstr(rx_buf, "ERROR") != NULL) {
                                current_esp_state = ESP_STATE_ERROR;
                            }
                            UART1_Clear_Data_Flag();
                        } else if ((current_tick - command_tick) > AT_CMD_TIMEOUT_MS) {
                            current_esp_state = ESP_STATE_ERROR;
                        }
                        break;

                    // --- 5. KẾT NỐI ĐẾN BROKER HIVEMQ ---
                    case ESP_STATE_MQTT_CONN:
                        // Cú pháp: AT+MQTTCONN=<LinkID>,<"Host">,<Port>,<Reconnect>
                        // Reconnect = 1 để ESP tự động kết nối lại khi rớt mạng.
                        Send_AT_Command("AT+MQTTCONN=0,\"broker.hivemq.com\",1883,1\r\n", ESP_STATE_WAIT_MQTT_CONN);
                        break;

                    case ESP_STATE_WAIT_MQTT_CONN:
                        if (UART1_Is_Data_Ready()) {
                            char* rx_buf = (char*)UART1_Get_Rx_Buffer();
                            // Phản hồi thành công sẽ có chuỗi "+MQTTCONNECTED" và "OK"
                            if (strstr(rx_buf, "OK") != NULL || strstr(rx_buf, "+MQTTCONNECTED") != NULL) {
                                current_esp_state = ESP_STATE_MQTT_SUB;
                            } else if (strstr(rx_buf, "ERROR") != NULL || strstr(rx_buf, "FAIL") != NULL) {
                                current_esp_state = ESP_STATE_ERROR;
                            }
                            UART1_Clear_Data_Flag();
                        } else if ((current_tick - command_tick) > MQTT_CONN_TIMEOUT_MS) { // Dùng timeout dài hơn cho kết nối TCP
                            current_esp_state = ESP_STATE_ERROR;
                        }
                        break;

                    // --- 6. SUBCRIBE TOPIC (LẮNG NGHE LỆNH TỪ APP) ---
                    case ESP_STATE_MQTT_SUB:
                        // Cú pháp: AT+MQTTSUB=<LinkID>,<"Topic">,<QoS>
                        // Đăng ký Topic lệnh điều khiển từ App, QoS = 1.
                        Send_AT_Command("AT+MQTTSUB=0,\"home/app/commands\",1\r\n", ESP_STATE_WAIT_MQTT_SUB);
                        break;

                    case ESP_STATE_WAIT_MQTT_SUB:
                        if (UART1_Is_Data_Ready()) {
                            char* rx_buf = (char*)UART1_Get_Rx_Buffer();
                            if (strstr(rx_buf, "OK") != NULL) {
                                // Hoàn tất chuỗi khởi tạo mạng. Chuyển sang trạng thái hoạt động.
                                current_esp_state = ESP_STATE_RUNNING;
                            } else if (strstr(rx_buf, "ERROR") != NULL) {
                                current_esp_state = ESP_STATE_ERROR;
                            }
                            UART1_Clear_Data_Flag();
                        } else if ((current_tick - command_tick) > AT_CMD_TIMEOUT_MS) {
                            current_esp_state = ESP_STATE_ERROR;
                        }
                        break;
                    case ESP_STATE_RUNNING:
                                if (UART1_Is_Data_Ready()) {
                                    char* rx_buf = (char*)UART1_Get_Rx_Buffer();

                                    // Chuỗi từ ESP nhả ra có định dạng: +MQTTSUBRECV:0,"home/app/commands",... ,ON
                                    if (strstr(rx_buf, "+MQTTSUBRECV") != NULL) {

                                        // Tìm kiếm Payload lệnh thực thi
                                        if (strstr(rx_buf, "ON") != NULL) {
                                            relay_set_state(RELAY_ON); // Bật thiết bị

                                            // Phản hồi trạng thái ngược lên App
                                            const char* pub_cmd = "AT+MQTTPUB=0,\"home/status/relay\",\"ON\",1,0\r\n";
                                            UART1_DMA_Transmit((uint8_t*)pub_cmd, strlen(pub_cmd));
                                        }
                                        else if (strstr(rx_buf, "OFF") != NULL) {
                                            relay_set_state(RELAY_OFF); // Tắt thiết bị

                                            // Phản hồi trạng thái ngược lên App
                                            const char* pub_cmd = "AT+MQTTPUB=0,\"home/status/relay\",\"OFF\",1,0\r\n";
                                            UART1_DMA_Transmit((uint8_t*)pub_cmd, strlen(pub_cmd));
                                        }
                                    }

                                    // Luôn dọn dẹp cờ và buffer sau khi xử lý xong một chuỗi
                                    UART1_Clear_Rx_Buffer();
                                    UART1_Clear_Data_Flag();
                                }
                                break;

                            // --- XỬ LÝ LỖI ---
                            case ESP_STATE_ERROR:
                                if (retry_count < MAX_RETRY) {
                                    retry_count++;
                                    current_esp_state = ESP_STATE_INIT; // Reset quy trình
                                } else {
                                    // Hard Reset ESP8266 thông qua chân EN (Nếu có nối dây)
                                    // Hoặc gửi "AT+RST\r\n"
                                    Send_AT_Command("AT+RST\r\n", ESP_STATE_WAIT_INIT);
                                    retry_count = 0;
                                }
                                break;
                        }
                    }

void UART_Process_MQTT_Publish(void) {
    // Kiểm tra cờ sự kiện từ Relay
    if (flag_relay_changed == true) {

        // Kiểm tra xem luồng DMA TX có đang bận gửi gói tin nào khác không
        // (Tránh ghi đè buffer TX khi ESP8266 đang bận xử lý)
        if ((DMA1_Channel4->CCR & (1U << 0)) == 0) { // DMA_CCR4_EN == 0

            flag_relay_changed = false; // Lập tức xóa cờ để chờ sự kiện tiếp theo

            if (current_relay_state == RELAY_ON) {
                const char* pub_cmd = "AT+MQTTPUB=0,\"home/status/relay\",\"ON\",1,0\r\n";
                UART1_DMA_Transmit((uint8_t*)pub_cmd, strlen(pub_cmd));
            }
            else {
                const char* pub_cmd = "AT+MQTTPUB=0,\"home/status/relay\",\"OFF\",1,0\r\n";
                UART1_DMA_Transmit((uint8_t*)pub_cmd, strlen(pub_cmd));
            }
        }
    }
}
