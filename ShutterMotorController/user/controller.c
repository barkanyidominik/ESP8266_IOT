//
// Created by barkanyi on 06/04/19.
//

#include "stdlib.h"
#include "../include/set_limits.h"
#include "../include/control.h"
#include "../include/movement.h"
#include "spi_flash.h"
#include "../include/mqtt_handle.h"
#include "mem.h"

static void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len);
static void position_callback(uint8_t percent);

static char printbuf[32];
static int32_t limit;


static void control_mode_stop_cb(int32_t position)
{
    DEBUG("control_mode_stop_cb(%d)", position);
}

static void control_mode_down_cb()
{
    DEBUG("control_mode_down_cb()");
}

static void control_mode_up_cb()
{
    DEBUG("control_mode_up_cb()");
}

ICACHE_FLASH_ATTR static void control_mode()
{
    control_mode_init(limit);
    set_position_callback(position_callback);
/*
    uint32_t _data = 123456;
    uint32_t _size = sizeof(_data);
    uint32_t _sector = 0x8C;
    if(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK) {
        if(spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, &_data, _size) == SPI_FLASH_RESULT_OK) {
            os_printf("SIKER");
        }
    }
*/
/*
    uint32_t _sector = 0x8C;
    uint32_t _data;
    uint32_t _size = sizeof(_data);
    spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, &_data, _size);
    os_printf("Data = %d", _data);
*/

}

static void programming_mode_cb(int32_t new_limit)
{
    DEBUG("programming_mode_cb()");
    limit = new_limit;
    control_mode();
}

ICACHE_FLASH_ATTR static void programming_mode()
{
    set_limits(&programming_mode_cb);
}

ICACHE_FLASH_ATTR void controller_init()
{
    mqttInit(mqttDataCb);
    init_movement();
    programming_mode();
}

static void position_callback(uint8_t percent)
{
    os_sprintf(printbuf, "%d", percent);
    mqttPublish(MQTT_POSITION_TOPIC, printbuf);
}

static void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
    char *topicBuf = (char*)os_zalloc(topic_len+1);
    char *dataBuf = (char*)os_zalloc(data_len+1);

    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    os_printf("%s\t", topicBuf);

    if(!strcmp(topicBuf, MQTT_CONTROL_MOVE_TOPIC))
    {
        if(!strcmp(dataBuf, "0"))
        {
            up_mqtt_handler();
        }
        else if(!strcmp(dataBuf, "1"))
        {
            down_mqtt_handler();
        }
    }
    else if(!strcmp(topicBuf, MQTT_CONTROL_POSITION_TOPIC))
    {
        int16_t num = strtol(dataBuf, NULL, 10);
        os_printf("%d\n", num);
        set_position_mqtt_handler(num);
    }


    os_free(topicBuf);
    os_free(dataBuf);
}