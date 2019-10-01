/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#define DEBUG_EN 0
#if DEBUG_EN == 1
#define DEBUG(format, ...) os_printf(format, ## __VA_ARGS__)
#else
#define DEBUG(format,...)
#endif

#include "osapi.h"
#include "user_interface.h"
#include "../include/driver/uart.h"

#include "utils.h"
#include "../include/dht.h"

#include "../include/mqtt_handle.h"

//Main task definition zone
#define MAIN_TASK_PRIO      0
#define MAIN_TASK_Q_SIZE    2


static os_event_t mainTaskQ[MAIN_TASK_Q_SIZE];
static os_timer_t mainTaskInitTimer;
static int16_t lastHum;
static int16_t lastTemp;
static char printbuf[32];
//extern ICACHE_FLASH_ATTR void socketStarter();
extern void dhtStart();
MqttHandler mqttHandler;


#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0xfd000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#else
#error "The flash map is not supported"
#endif

static const partition_item_t at_partition_table[] = {
        { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
        { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
        { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
        { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
        { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
        { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
        os_printf("system_partition_table_regist fail\r\n");
        while(1);
    }
}

void mainTask(os_event_t *ev) // sig: 0 - Normál állapot, 1 - Hiba, újrainicializálás
{
    DhtHandler* Dht = (DhtHandler*)ev->par;
    if(ev->par == 0)
    {
        return;
    }
    switch(Dht->state)
    {
        case DHT_ERR:
            DEBUG("Error in dht reading\n\r");
            break;
        case DHT_COMPL:
            DEBUG("DhtReading complete: %d %d\n\r", Dht->DhtHum, Dht->DhtTemp);

            //HUMIDITY
            os_sprintf(printbuf,"%d", (int)(Dht->DhtHum));
            DEBUG("DhtHum: %s\n\r", printbuf);
            if(lastHum != Dht->DhtHum)
            {
                os_printf("Mqtt send to [dht.hum]: %s\n", printbuf);
                mqttPublish(MQTT_HUM_TOPIC, printbuf);
                mqttPublish(MQTT_HUM_VALUE_TOPIC, printbuf);
                lastHum = Dht->DhtHum;
            }

            //TEMPERATURE
            os_sprintf(printbuf,"%d", (int)(Dht->DhtTemp));
            DEBUG("DhtTemp: %s\n\r", printbuf);
            if(lastTemp != Dht->DhtTemp)
            {
                os_printf("Mqtt send to [dht.temp]: %s\n", printbuf);
                mqttPublish(MQTT_TEMP_TOPIC, printbuf);
                mqttPublish(MQTT_TEMP_VALUE_TOPIC, printbuf);
                lastTemp = Dht->DhtTemp;
            }
            break;
        default:
            DEBUG("Default state\n\r");
    }
    dhtStart();
}

void mainTaskInit()
{
    DEBUG("MainTaskInit %d\n\r", mqttHandler.mqttState);
    if(mqttHandler.mqttState == MQTT_CONNECTED)
    {
        DEBUG("CONNECTED call mainTask");

        os_timer_disarm(&mainTaskInitTimer);
        dhtStart();
    }
}


user_init(void)
{
    uart_div_modify(UART0, UART_CLK_FREQ/115200);
    mqttInit();
    system_os_task(mainTask, MAIN_TASK_PRIO, mainTaskQ,MAIN_TASK_Q_SIZE);

    os_timer_setfn(&mainTaskInitTimer, (os_timer_func_t*)mainTaskInit, (void*)0);
    os_timer_arm(&mainTaskInitTimer, 1000, 1);

}




