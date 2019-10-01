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
#include <stdlib.h>
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "../include/driver/uart.h"
#include "../include/mqtt_handle.h"
#include "mem.h"

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

static void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len);

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


#include "pwm.h"
#include "os_type.h"

#define NB_PWM_CHANNELS  3u          // Nb of PWM channel to be active

static os_timer_t duty_timer;
static uint8_t r;
static uint8_t g;
static uint8_t b;
static uint8_t brightness;

void set_rgb()
{
    pwm_set_duty(r * brightness, 0);
    pwm_set_duty(g * brightness, 1);
    pwm_set_duty(b * brightness, 2);
    pwm_start();
}

void set_brightness(uint8_t percent)
{
    if(percent >= 0 && percent <= 100)
    {
        brightness = (uint8_t)(0.87*percent);
        set_rgb();
    }
}

void set_hex(char* hexstring)
{
    long number = (long) strtol( &hexstring[0], NULL, 16);
    r = number >> 16;
    g = number >> 8 & 0xFF;
    b = number & 0xFF;

    os_printf("r = %d , g = %d, b = %d \n\r", r,g,b);
    set_rgb();
}

void ICACHE_FLASH_ATTR user_init()
{
    uart_init(115200, 115200);

    uint32 period = 1000;                      // PWM period in microsec (mini is 1000, max 10000)
    uint32 nbChannels = NB_PWM_CHANNELS;       // Nb of PWM channel to be active
    uint32 duty[NB_PWM_CHANNELS] = {r, g, b};    // Duty cycle, in number of step of 45ns
    uint32 pinParams[NB_PWM_CHANNELS][3] = {{PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2, 2u}, {PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0, 0u}, {PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4, 4u}};

    pwm_init(period, duty, nbChannels, pinParams);
    pwm_start();

    brightness = 100;

    mqttInit(mqttDataCb);
}


static void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
    char *topicBuf = (char*)os_zalloc(topic_len+1);
    char *dataBuf = (char*)os_zalloc(data_len+1);

    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    os_printf("%s\t\n", topicBuf);

    if(!strcmp(topicBuf, "control/led.switch"))
    {
        if(!strcmp(dataBuf, "1"))
        {
            set_brightness(100);
        }
        else if(!strcmp(dataBuf, "0"))
        {
            set_brightness(0);
        }
    }
    else if(!strcmp(topicBuf, "control/led.brightness"))
    {
        int16_t num = strtol(dataBuf, NULL, 10);
        os_printf("%d\n", num);
        set_brightness(num);
    }
    else if(!strcmp(topicBuf, "control/led.hex"))
    {
        char* hex = dataBuf + 1; //A # törléséhez növeljük eggyel a pointert
        set_hex(hex);
    }


    os_free(topicBuf);
    os_free(dataBuf);
}


