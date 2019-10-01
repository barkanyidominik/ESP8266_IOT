//
// Created by barkanyi on 2018.10.29..
//

#ifndef ESP8266_NONOS_DEV_DHT_H
#define ESP8266_NONOS_DEV_DHT_H

#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "gpio.h"
#include "c_types.h"

#define MOTION_PIN     2

ICACHE_FLASH_ATTR void motionSensorInit();

#define MOTION_PIN_ON()    GPIO_OUTPUT_SET(MOTION_PIN, 1)
#define MOTION_PIN_OFF()   GPIO_OUTPUT_SET(MOTION_PIN, 0)

#define MOTION_PIN_DIS()   GPIO_DIS_OUTPUT(MOTION_PIN)

#define MOTION_READ_PIN()  GPIO_INPUT_GET(MOTION_PIN)

#define READ_REG(_r)    (*(volatile uint32_t *)(_r))
#define WDEW_NOW()      READ_REG(0x3ff20c00)


#endif //ESP8266_NONOS_DEV_DHT_H
