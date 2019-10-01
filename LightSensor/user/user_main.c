/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "../include/detector.h"
#include "../include/driver/uart.h"
#include "../include/mqtt_handle.h"
#include "stdlib.h"

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

static os_timer_t measureTaskTimer;
static char printbuf[32];

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

void detector_callback(uint8_t lightDetect){
    os_sprintf(printbuf, "%d", lightDetect);
    if(lightDetect){
        os_printf("MQTT: Fény VAN!\n");
        mqttPublishLight(printbuf);
    } else {
        os_printf("MQTT: VÉGE!\n");
        mqttPublishLight(printbuf);
    }
}

typedef enum {
    STAGNATE,   //Beállt a fényerő
    MODULATE,   //Változik a fényerő (gyors változás, pl. villany kapcsolás)
    INDEFINITE  //Határozatlan
} measureState;

measureState state;

static void ICACHE_FLASH_ATTR
measureTask(os_event_t *events)
{
    static const uint16_t measure_number = 100; // Egy körben 100 mérést végzünk
    static uint16_t lastValue;

    uint32_t average = 0;
    uint16_t min = 1024;  //Max value 1024
    uint16_t max = 0;    //Min value 0

    int i;
    for(i = 0; i < measure_number; i++){
        uint16_t measurement = system_adc_read();

        average += measurement;

        if(measurement < min)
        {
            min = measurement;
        }
        if(measurement > max)
        {
            max = measurement;
        }
    }

    average = average / measure_number;     // Átlagoljuk a méréseket

    uint16_t value = (uint16_t)((1024-average)/10.24);  // Átalakítjuk %-os formára.

    if(state == STAGNATE){
        if(max - min > 10 || abs(lastValue - value) > 10) {
            state = INDEFINITE;
            os_printf("STAGNATE -> INDEFINITE\n");
        }
    }
    else if(state == MODULATE)
    {
        if(max - min < 10)
        {
            state = INDEFINITE;
            os_printf("MODULATE -> INDEFINITE\n");
        }
    }
    else if(state == INDEFINITE)
    {
        if(max - min < 10)
        {
            state = STAGNATE;
            os_printf("INDEFINITE -> STAGNATE\n");
            if(lastValue != value){
                os_printf("\tValue: %d\n", value);
                os_sprintf(printbuf, "%d", value);
                mqttPublishLight(printbuf);
            }
            lastValue = value;
        }
        else
        {
            os_printf("INDEFINITE -> MODULATE\n");
            state = MODULATE;
        }
    }
}

void ICACHE_FLASH_ATTR
initMeasureTask()
{
    //init_detector(&detector_callback);    // Detector a digitális méréshez (GPIO port).

    state = INDEFINITE;

    os_timer_disarm(&measureTaskTimer);     //Ha reconnect történne a brókerre, akkor újra meg lesz hívva az initMeasureTask, ezért disarmoljuk az előző MeasureTask-ot.
    os_timer_setfn(&measureTaskTimer, (os_timer_func_t*) measureTask, 0);
    os_timer_arm(&measureTaskTimer, 50, 1);

}

void ICACHE_FLASH_ATTR
user_init(void)
{
    uart_init(115200, 115200);
    mqttInit();
    setMqttConnectedCb(&initMeasureTask);
}