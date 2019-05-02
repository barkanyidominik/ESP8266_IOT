//
// Created by barkanyi on 2018.12.01..
//
#ifndef ESP8266_NONOS_DEV_MQTT_HANDLE_H
#define ESP8266_NONOS_DEV_MQTT_HANDLE_H

typedef enum
{
    MQTT_INIT,
    MQTT_CONNECTED,
    MQTT_DISCONNECTED
} MqttState;

typedef struct {
    MqttState mqttState;
} MqttHandler;

void mqttInit();
void mqttPublishLight();
void setMqttConnectedCb(void*);

#endif //ESP8266_NONOS_DEV_MQTT_HANDLE_H
