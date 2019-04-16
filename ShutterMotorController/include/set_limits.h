//
// Created by barkanyi on 06/03/19.
//

#ifndef ESP8266_IOT_SET_LIMITS_H
#define ESP8266_IOT_SET_LIMITS_H
#include "osapi.h"

ICACHE_FLASH_ATTR void set_limits(void(*callback)());


#define REG_READ(_r)     (*(volatile uint32 *)(_r))
#define WDEV_NOW()    REG_READ(0x3ff20c00)

#endif //ESP8266_IOT_SET_LIMITS_H
