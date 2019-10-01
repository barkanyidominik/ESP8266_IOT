//
// Created by barkanyi on 2018.12.01..
//

#ifndef ESP8266_NONOS_DEV_MQTT_HANDLE_H
#define ESP8266_NONOS_DEV_MQTT_HANDLE_H

#define MQTT_LWT_TOPIC          "sensor/motion.status"
#define MQTT_LWT_MESSAGE        "Offline"
#define MQTT_CONNECT_MESSAGE    "Online"

#define MQTT_MOTION_TOPIC   "sensor/motion.alert"

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
void mqttPublishMotion();
void setMqttConnectedCb(void*);

#endif //ESP8266_NONOS_DEV_MQTT_HANDLE_H
