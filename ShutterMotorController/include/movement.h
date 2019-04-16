//
// Created by barkanyi on 03/04/19.
//

#ifndef ESP8266_IOT_MOVEMENT_H
#define ESP8266_IOT_MOVEMENT_H

typedef enum {
    IDLE,
    UP,
    DOWN
} MovementState;

MovementState get_movementState();
void up_movement_start();
void down_movement_start();
void movement_stop();
ICACHE_FLASH_ATTR void init_movement();

#define DEBUG_EN 1
#if DEBUG_EN == 1
#define DEBUG(format, ...) os_printf(format, ## __VA_ARGS__)
#else
#define DEBUG(format,...)
#endif

#endif //ESP8266_IOT_MOVEMENT_H
