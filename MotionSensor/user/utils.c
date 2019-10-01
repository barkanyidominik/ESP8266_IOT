
#include "utils.h"

static uint8_t led_state;
static os_timer_t blinkTimer;

void heartBeat(){
    GPIO_OUTPUT_SET(2, led_state);
    led_state ^= 1;
    os_printf("heartBeat!\n\r");
}

void initBlinky()
{
    led_state = 0;
    //PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
    os_timer_setfn(&blinkTimer, (os_timer_func_t*)heartBeat, 0);
    os_timer_arm(&blinkTimer, LED_BLINKY_TIME, 1);
}

void initConnecton()
{
    wifi_set_opmode(0x01); //Station mode
    wifi_softap_dhcps_stop();
    char ssid[] = "UTILSWIFI2";
    char password[] = "nyuszika";
    struct station_config stationConf;
    stationConf.bssid_set = 0;
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);


}

void initUtils()
{
    initBlinky();
    initConnecton();
}
