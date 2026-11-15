#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

const char* topic_sub = "bk_phu/lab/commands";
const char* topic_pub_relay = "bk_phu/lab/status";
const char* topic_pub_count = "bk_phu/lab/count";
const char* topic_pub_weather = "bk_phu/lab/weather";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_DELAY = 5000;

const uint8_t MAX_UART_LEN = 48;
char uartBuffer[MAX_UART_LEN];
uint8_t uartIndex = 0;

boolean reconnectMQTT() {
  char clientId[20];
  snprintf(clientId, sizeof(clientId), "ESP-%04X", random(0xffff));
  if (client.connect(clientId)) {
    client.subscribe(topic_sub);
    client.subscribe(topic_pub_weather);
    return true;
  }
  return false;
}

void callback(char* topic, byte* payload, unsigned int length) {
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';
  if (strcmp(topic, topic_pub_weather) == 0) {
    Serial.print(F("API:"));
    Serial.print(msg);
    Serial.print(F("\n"));
    return;
  }
  if (strncmp(msg, "R1:", 3) == 0 || strncmp(msg, "R2:", 3) == 0 || strncmp(msg, "R3:", 3) == 0 || strncmp(msg, "R4:", 3) == 0 || strncmp(msg, "ALL:", 4) == 0 || strncmp(msg, "SET_CNT:", 8) == 0 || strcmp(msg, "ON") == 0 || strcmp(msg, "OFF") == 0) {
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
        if (strncmp(uartBuffer, "STS:REL:", 8) == 0) {
          client.publish(topic_pub_relay, uartBuffer + 8, true);
        } else if (strcmp(uartBuffer, "STS:ON") == 0) {
          client.publish(topic_pub_relay, "ON", true);
        } else if (strcmp(uartBuffer, "STS:OFF") == 0) {
          client.publish(topic_pub_relay, "OFF", true);
        } else if (strncmp(uartBuffer, "STS:CNT:", 8) == 0) {
          client.publish(topic_pub_count, uartBuffer + 8, true);
        }
        uartIndex = 0;
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
  wm.setDebugOutput(false);
  bool res = wm.autoConnect("IoT_Product_Setup");
  if (!res) {
    ESP.restart();
  }
  client.setServer(mqtt_server, mqtt_port);
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
