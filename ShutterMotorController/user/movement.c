//
// Created by barkanyi on 03/04/19.
//
#include "osapi.h"
#include "../include/movement.h"

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
        DEBUG("\t\t\t\tRELAY1: ON\n");
    }
}

void down_movement_start()
{
    if(movementState == IDLE)
    {
        movementState = DOWN;
        DEBUG("\t\t\t\tRELAY2: ON\n");
    }
}

void movement_stop()
{
    DEBUG("\t\t\t\tRELAY1: OFF\n");
    DEBUG("\t\t\t\tRELAY2: OFF\n");
    movementState = IDLE;
}

ICACHE_FLASH_ATTR void init_movement()
{
    movementState = IDLE;


}