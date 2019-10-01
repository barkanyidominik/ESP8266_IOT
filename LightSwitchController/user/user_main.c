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
#include "../include/driver/uart.h"
#include "../include/mqtt_handle.h"
#include "mem.h"
#include "stdlib.h"

static uint8_t relay_state;
static os_timer_t relay_timer;
static uint8_t button_flag;
static os_timer_t interrupt_timer;

static char printbuf[32];

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

void ICACHE_FLASH_ATTR
set_relay(uint8_t flag)
{
    if((flag ^ relay_state) || flag == 2)
    {
        relay_state ^= 1u;
        GPIO_OUTPUT_SET(2, relay_state);
        os_timer_disarm(&relay_timer);
        os_printf("Lámpa kapcsolva\n");
    }
    os_sprintf(printbuf, "%d", relay_state);
    mqttPublish(MQTT_SWITCH_TOPIC, printbuf);
}

void ICACHE_FLASH_ATTR
relay_timer_callback()
{
    os_printf("Lámpa lekapcsolva az időzítő által\n");
    set_relay(2);
}

void ICACHE_FLASH_ATTR
set_relay_timer(uint32_t time)
{
    os_printf("Lámpakapcsolás időzétése elindult...\n");
    os_timer_disarm(&relay_timer);
    os_timer_arm(&relay_timer, time, 0);
}

static void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
    char *topicBuf = (char*)os_zalloc(topic_len+1);
    char *dataBuf = (char*)os_zalloc(data_len+1);

    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    if(!strcmp(topicBuf, MQTT_CONTROL_SWITCH_TOPIC))
    {
        if(!strcmp(dataBuf, "1") && !relay_state)
        {
            set_relay(1);
        }
        else if(!strcmp(dataBuf, "0") && relay_state)
        {
            set_relay(0);
        }
    }
    else if(!strcmp(topicBuf, MQTT_CONTROL_TIMER_TOPIC))
    {
        uint32_t time = strtol(dataBuf, NULL, 10) * 1000;
        set_relay_timer(time);
    }

    os_free(topicBuf);
    os_free(dataBuf);
}

static void set_button_interrupt()
{
    ETS_GPIO_INTR_DISABLE();

    if(button_flag)
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_HILEVEL);
    }
    else
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_LOLEVEL);
    }

    ETS_GPIO_INTR_ENABLE();
}

static void gpio_intr_handler()
{
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    ETS_GPIO_INTR_DISABLE();

    if( gpio_status & BIT(0u) ) // 0u - 0 Unsigned, because operand with binary bitwise operator
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_DISABLE);
        button_flag ^= 1u;
        if(button_flag){
            set_relay(2);
        }
        os_timer_arm(&interrupt_timer, 50, 0);
    }

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    ETS_GPIO_INTR_ENABLE();
}

static void set_gpio_intr()
{
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(0));

    ETS_GPIO_INTR_DISABLE();

    ETS_GPIO_INTR_ATTACH(&gpio_intr_handler, NULL);
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_LOLEVEL);

    ETS_GPIO_INTR_ENABLE();
}

void ICACHE_FLASH_ATTR
user_init(void)
{
    uart_init(115200, 115200);

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

    relay_state = 0;
    button_flag = 0;

    os_timer_setfn(&interrupt_timer, (os_timer_func_t*)set_button_interrupt, 0);
    os_timer_setfn(&relay_timer, (os_timer_func_t*)relay_timer_callback, 0);

    set_gpio_intr();

    mqttInit(mqttDataCb);
}

