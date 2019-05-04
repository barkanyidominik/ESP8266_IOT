#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "../include/detector.h"

static uint8_t intr_state = 0;
static void(*detectorCb)(uint8_t);

static void set_gpio_intr()
{
    if(!intr_state)
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_LOLEVEL);
    }
    else
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_HILEVEL);
    }
}

static void gpio_intr_handler()
{
    ETS_GPIO_INTR_DISABLE();
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    intr_state ^= 1u;
    if( gpio_status & BIT(2) )
    {
        detectorCb(intr_state);
        set_gpio_intr();
    }

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);


    ETS_GPIO_INTR_ENABLE();
}

ICACHE_FLASH_ATTR
void init_detector(void(*callback)(uint8_t))
{
    detectorCb = callback;
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U); // Beépített felhúzó ellenállás engedélyezése
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2)); // Beállítás bemenetként

    ETS_GPIO_INTR_DISABLE();

    ETS_GPIO_INTR_ATTACH(&gpio_intr_handler, NULL);

    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_LOLEVEL);

    intr_state = 0;

    ETS_GPIO_INTR_ENABLE();
}