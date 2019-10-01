//
// Created by barkanyi on 2018.12.01..
//
#ifndef ESP8266_NONOS_DEV_MQTT_HANDLE_H
#define ESP8266_NONOS_DEV_MQTT_HANDLE_H

#define MQTT_LWT_TOPIC          "sensor/fan.status"
#define MQTT_LWT_MESSAGE        "Offline"
#define MQTT_CONNECT_MESSAGE    "Online"

#define MQTT_SWITCH_TOPIC       "sensor/fan.switch"

#define MQTT_CONTROL_SWITCH_TOPIC       "control/fan.switch"
#define MQTT_CONTROL_TIMER_TOPIC     "control/fan.timer"

typedef enum
{
    MQTT_INIT,
    MQTT_CONNECTED,
    MQTT_DISCONNECTED
} MqttState;

typedef struct {
    MqttState mqttState;
} MqttHandler;

void mqttInit(void*);
void mqttPublish();
void setMqttConnectedCb(void*);

#endif //ESP8266_NONOS_DEV_MQTT_HANDLE_H
