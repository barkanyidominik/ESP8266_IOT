//
// Created by barkanyi on 06/03/19.
//

#ifndef ESP8266_IOT_CONTROL_H
#define ESP8266_IOT_CONTROL_H

typedef struct
{
    uint8_t upButton_flag;
    uint8_t downButton_flag;
    int32_t upButton_offset;
    int32_t downButton_offset;
    os_timer_t upButton_timer;
    os_timer_t downButton_timer;

} ButtonsStatus;

static ButtonsStatus buttons;

void control_mode_init(int32_t limit_us);

#define REG_READ(_r)     (*(volatile uint32 *)(_r))
#define WDEV_NOW()    REG_READ(0x3ff20c00)



#define DEBUG_EN 1
#if DEBUG_EN == 1
#define DEBUG(format, ...) os_printf(format, ## __VA_ARGS__)
#else
#define DEBUG(format,...)
#endif


#endif //ESP8266_IOT_CONTROL_H
