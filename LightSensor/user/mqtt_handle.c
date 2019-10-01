//
// Created by barkanyi on 2018.12.01..
//

#include "../include/mqtt_handle.h"
#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "sntp.h"

MQTT_Client mqttClient;
typedef unsigned long u32_t;
static ETSTimer sntp_timer;
static MqttHandler mqttHandler;

void sntpfn()
{
    u32_t ts = 0;
    ts = sntp_get_current_timestamp();
    os_printf("current time : %s\n", sntp_get_real_time(ts));
    if (ts == 0) {
        //os_printf("did not get a valid time from sntp server\n");
    } else {
        os_timer_disarm(&sntp_timer);
        MQTT_Connect(&mqttClient);
    }
}

void wifiConnectCb(uint8_t status)
{
    if(status == STATION_GOT_IP){
        sntp_setservername(0, "pool.ntp.org");        // set sntp server after got ip address
        sntp_init();
        os_timer_disarm(&sntp_timer);
        os_timer_setfn(&sntp_timer, (os_timer_func_t *)sntpfn, NULL);
        os_timer_arm(&sntp_timer, 1000, 1);//1s
    } else {
        MQTT_Disconnect(&mqttClient);
    }
}

void mqttConnectedCb(uint32_t *args)
{
    INFO("MQTT CONNECTED CB!!!!\n\r");
    mqttHandler.mqttState = MQTT_CONNECTED;
    MQTT_Client* client = (MQTT_Client*)args;

    MQTT_Publish(client, MQTT_LWT_TOPIC, MQTT_CONNECT_MESSAGE, sizeof(MQTT_CONNECT_MESSAGE), 0, 1);

    INFO("MQTT: Connected\r\n");
}

void mqttDisconnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    mqttHandler.mqttState = MQTT_DISCONNECTED;
    INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
    char *topicBuf = (char*)os_zalloc(topic_len+1),
            *dataBuf = (char*)os_zalloc(data_len+1);

    MQTT_Client* client = (MQTT_Client*)args;

    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
    os_printf("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
    os_free(topicBuf);
    os_free(dataBuf);
}

ICACHE_FLASH_ATTR void setMqttConnectedCb(void *cb)
{
    INFO("Set mqtt ConnectedCb");
    MQTT_OnConnected(&mqttClient, cb);
}

ICACHE_FLASH_ATTR void mqttPublishLight(char* data)
{
    INFO("Publish data: %s!!!", data);
    MQTT_Publish(&mqttClient, MQTT_LIGHT_TOPIC, data, strlen(data), 0, 0);
    MQTT_Publish(&mqttClient, MQTT_BRIGHTNESS_TOPIC, data, strlen(data), 0, 0);
}

ICACHE_FLASH_ATTR void mqttInit(void)
{
    os_delay_us(60000);
    mqttHandler.mqttState = MQTT_INIT;

    CFG_Load();

    MQTT_InitConnection(&mqttClient, MQTT_HOST, MQTT_PORT, 0);

    MQTT_InitClient(&mqttClient, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, 60, 1);

    MQTT_InitLWT(&mqttClient, MQTT_LWT_TOPIC, MQTT_LWT_MESSAGE, 0, 1);

    MQTT_OnConnected(&mqttClient, mqttConnectedCb);

    WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);

    MQTT_OnData(&mqttClient, mqttDataCb);

    INFO("\r\nSystem started ...\r\n");
}
