//
// Created by barkanyi on 2018.12.01..
//

#ifndef ESP8266_NONOS_DEV_MQTT_HANDLE_H
#define ESP8266_NONOS_DEV_MQTT_HANDLE_H

#define MQTT_LWT_TOPIC          "sensor/dht.status"
#define MQTT_DISCONNECT_MESSAGE "Offline"
#define MQTT_CONNECT_MESSAGE    "Online"

#define MQTT_HUM_TOPIC          "sensor/hum"
#define MQTT_TEMP_TOPIC         "sensor/temp"
#define MQTT_HUM_VALUE_TOPIC    "sensor/hum.value"
#define MQTT_TEMP_VALUE_TOPIC   "sensor/temp.value"

typedef enum
{
    MQTT_INIT,
    MQTT_CONNECTED,
    MQTT_DISCONNECTED
} MqttState;

typedef struct {
    MqttState mqttState;
} MqttHandler;

extern MqttHandler mqttHandler;

void mqttInit();
void mqttPublish(char* topic, char* data);
void setMqttConnectedCb(void*);

#endif //ESP8266_NONOS_DEV_MQTT_HANDLE_H
