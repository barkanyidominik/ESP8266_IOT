//
// Created by barkanyi on 2018.10.29..
//

#define DEBUG_EN 0
#if DEBUG_EN == 1
#define DEBUG(format, ...) os_printf(format, ## __VA_ARGS__)
#else
#define DEBUG(format,...)
#endif

#include "../include/motion.h"
#include "../include/mqtt_handle.h"

static char printbuf[32];

void motionStopCb()
{
    os_printf("MOTION STOP!!!\n\r");
    os_sprintf(printbuf, "%d", 0);
    mqttPublishMotion(printbuf);
    ETS_GPIO_INTR_DISABLE();
    motionSensorInit();
}

ICACHE_FLASH_ATTR void setupMotionStopCb()
{
    ETS_GPIO_INTR_DISABLE(); // Interrupt tiltása, amíg elvégezzük a szükséges beállításokat
    ETS_GPIO_INTR_ATTACH(&motionStopCb, NULL);
    gpio_register_set(GPIO_PIN_ADDR(MOTION_PIN), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                                                 | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                                                 | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(MOTION_PIN));
    gpio_pin_intr_state_set(GPIO_ID_PIN(MOTION_PIN), GPIO_PIN_INTR_NEGEDGE);
    ETS_GPIO_INTR_ENABLE();
}

void motionStartCb()
{
    os_printf("MOTION!!!\n\r");
    os_sprintf(printbuf, "%d", 1);
    mqttPublishMotion(printbuf);
    setupMotionStopCb();
}

ICACHE_FLASH_ATTR void setupMotionStartCb()
{
    ETS_GPIO_INTR_DISABLE(); // Interrupt tiltása, amíg elvégezzük a szükséges beállításokat
    ETS_GPIO_INTR_ATTACH(&motionStartCb, NULL);
    gpio_register_set(GPIO_PIN_ADDR(MOTION_PIN), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                                              | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                                              | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(MOTION_PIN));
    gpio_pin_intr_state_set(GPIO_ID_PIN(MOTION_PIN), GPIO_PIN_INTR_POSEDGE);
    ETS_GPIO_INTR_ENABLE();
}

ICACHE_FLASH_ATTR void motionSensorInit()
{
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    GPIO_DIS_OUTPUT(MOTION_PIN);   // GPIO kimenet tiltása
    setupMotionStartCb();
}