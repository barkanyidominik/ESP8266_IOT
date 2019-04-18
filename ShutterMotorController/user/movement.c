//
// Created by barkanyi on 03/04/19.
//
#include "osapi.h"
#include "../include/movement.h"
#include "gpio.h"

static MovementState movementState;

extern int32_t debugPosition;
extern int32_t debugLimit;

MovementState get_movementState()
{
    return movementState;
}

void up_movement_start()
{
    if(movementState == IDLE)
    {
        movementState = UP;
        GPIO_OUTPUT_SET(5,1);
        DEBUG("\t\t\t\tRELAY1: ON\n");
    }
}

void down_movement_start()
{
    if(movementState == IDLE)
    {
        movementState = DOWN;
        GPIO_OUTPUT_SET(12,1);
        DEBUG("\t\t\t\tRELAY2: ON\n");
    }
}

void movement_stop()
{
    DEBUG("\t\t\t\tRELAY1: OFF\n");
    DEBUG("\t\t\t\tRELAY2: OFF\n");
    GPIO_OUTPUT_SET(5,0);
    GPIO_OUTPUT_SET(12,0);
    movementState = IDLE;
}

ICACHE_FLASH_ATTR void init_movement()
{
    movementState = IDLE;

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
}