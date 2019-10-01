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

#define DHT_PIN     2

#define DHT11       1
#define DHT22       2

#define DHT_TYPE    DHT11

#define DHT_TIMEOUT 10

typedef enum
{
   DHT_IDLE,    //Az API készen áll az olvasásra
    DHT_PREP,
    DHT_MEAS,
    DHT_COMPL,   //Van már mért adat, de még nem olvasták el
    DHT_ERR      //

} DhtState;

//extern static DhtHandler DhtHandle;

typedef struct
{
    DhtState state;
#if DHT_TYPE == DHT11
    int16_t DhtTemp;
    int16_t DhtHum;
#else
    float DhtTemp;
    float DhtHum;
#endif
    uint8_t TimeoutCnt;
}DhtHandler;

#define DHT_PIN_ON()    GPIO_OUTPUT_SET(DHT_PIN, 1)
#define DHT_PIN_OFF()   GPIO_OUTPUT_SET(DHT_PIN, 0)

#define DHT_PIN_DIS()   GPIO_DIS_OUTPUT(DHT_PIN)

#define DHT_READ_PIN()  GPIO_INPUT_GET(DHT_PIN)

#define READ_REG(_r)    (*(volatile uint32_t *)(_r))
#define WDEW_NOW()      READ_REG(0x3ff20c00)


#endif //ESP8266_NONOS_DEV_DHT_H
