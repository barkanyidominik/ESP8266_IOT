//
// Created by barkanyi on 06/04/19.
//
#include "../include/set_limits.h"
#include "../include/control.h"
#include "../include/movement.h"

static void control_mode_stop_cb(int32_t position)
{
    DEBUG("control_mode_stop_cb(%d)", position);
}

static void control_mode_down_cb()
{
    DEBUG("control_mode_down_cb()");
}

static void control_mode_up_cb()
{
    DEBUG("control_mode_up_cb()");
}

ICACHE_FLASH_ATTR static void control_mode()
{
    control_mode_init(10000000);
}

static void programming_mode_cb()
{
    DEBUG("programming_mode_cb()");
}

ICACHE_FLASH_ATTR static void programming_mode()
{
    set_limits(&programming_mode_cb);
}

ICACHE_FLASH_ATTR void controller_init()
{
    init_movement();
    //set_limits(&programming_mode_cb);
    control_mode();
}