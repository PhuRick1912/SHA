#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

const char* mqtt_server = "test.mosquitto.org";

// --- CẤU HÌNH TOPIC ---
const char* topic_sub       = "bk_phu/lab/commands";
const char* topic_pub_relay = "bk_phu/lab/status";
const char* topic_pub_count = "bk_phu/lab/count";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_DELAY = 5000;

const uint8_t MAX_UART_LEN = 32;
char uartBuffer[MAX_UART_LEN];
uint8_t uartIndex = 0;

boolean reconnectMQTT() {
  char clientId[20];
  snprintf(clientId, sizeof(clientId), "ESP-%04X", random(0xffff));
  
  if (client.connect(clientId)) {
    client.subscribe(topic_sub);
    return true;
  }
  return false;
}

void callback(char* topic, byte* payload, unsigned int length) {
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';
  
  if (strcmp(msg, "ON") == 0) {
    Serial.print(F("CMD:ON\n"));
  } 
  else if (strcmp(msg, "OFF") == 0) {
    Serial.print(F("CMD:OFF\n"));
  }
  else if (strncmp(msg, "SET_CNT:", 8) == 0) {
    Serial.print(F("CMD:"));
    Serial.print(msg);
    Serial.print(F("\n"));
  }
}

void processUART() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (uartIndex > 0) {
        uartBuffer[uartIndex] = '\0';
        
        if (strcmp(uartBuffer, "STS:ON") == 0) {
          client.publish(topic_pub_relay, "ON");
        } 
        else if (strcmp(uartBuffer, "STS:OFF") == 0) {
          client.publish(topic_pub_relay, "OFF");
        }
        else if (strncmp(uartBuffer, "STS:CNT:", 8) == 0) {
          client.publish(topic_pub_count, uartBuffer + 8);
        }
        
        uartIndex = 0; // Reset buffer
      }
    } else if (uartIndex < MAX_UART_LEN - 1) {
      uartBuffer[uartIndex++] = c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(10); 
  
  WiFiManager wm;
  
  // Xóa cấu hình cũ để test (Bỏ comment khi cần debug)
  // wm.resetSettings();

  wm.setDebugOutput(false);

  bool res = wm.autoConnect("IoT_Product_Setup");

  if(!res) {
    // Timeout hoặc cấu hình lỗi -> Reset để thử lại
    ESP.restart();
  }

  // Khi vượt qua dòng trên -> ESP đã vào được WiFi nhà người dùng
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      unsigned long now = millis();
      if (now - lastReconnectAttempt > RECONNECT_DELAY) {
        lastReconnectAttempt = now;
        if (reconnectMQTT()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      client.loop();
    }
  }
  processUART();
}