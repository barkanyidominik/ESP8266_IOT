//
// Created by barkanyi on 06/04/19.
//

#include "../include/set_limits.h"
#include "../include/control.h"
#include "../include/movement.h"
#include "spi_flash.h"

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
/*
    uint32_t _data = 123456;
    uint32_t _size = sizeof(_data);
    uint32_t _sector = 0x3D;
    if(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK) {
        if(spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, &_data, _size) == SPI_FLASH_RESULT_OK) {
            os_printf("SIKER");
        }
    }

    uint32_t _sector = 0x3D;
    uint32_t _data;
    uint32_t _size = sizeof(_data);
    spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, &_data, _size);
    os_printf("Data = %d", _data);
*/
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