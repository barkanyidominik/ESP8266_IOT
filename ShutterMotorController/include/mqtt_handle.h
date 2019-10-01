//
// Created by barkanyi on 2018.12.01..
//
#ifndef ESP8266_NONOS_DEV_MQTT_HANDLE_H
#define ESP8266_NONOS_DEV_MQTT_HANDLE_H

#define MQTT_LWT_TOPIC          "sensor/shutter.status"
#define MQTT_LWT_MESSAGE        "Offline"
#define MQTT_CONNECT_MESSAGE    "Online"

#define MQTT_POSITION_TOPIC     "sensor/shutter.position"

#define MQTT_CONTROL_MOVE_TOPIC         "control/shutter.move"
#define MQTT_CONTROL_POSITION_TOPIC     "control/shutter.position"

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
