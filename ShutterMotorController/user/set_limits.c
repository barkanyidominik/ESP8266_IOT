#include "osapi.h"
#include "ets_sys.h"
#include "user_interface.h"
#include "../include/set_limits.h"
#include "../include/control.h"

typedef enum
{
    IDLE,    //Tétlen
    PREP,    //Előkészítés a programozásra
    UPLIMIT, //Felső végállás beállítás alatt
    LOLIMIT, //Alsó végállás beállítás alatt
    COMPL,  //Kész
    ERR     //Hiba
} ProgrammingState;

static ButtonsStatus buttons;
static ProgrammingState programmingState;
static int32_t limit;
static void(*setLimitsCb)();

static void programming_mode_close()
{
    setLimitsCb();
}

static void set_upButton_intr()
{
    ETS_GPIO_INTR_DISABLE();

    if(buttons.upButton_flag)
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_HILEVEL);
    }
    else
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_LOLEVEL);
    }

    ETS_GPIO_INTR_ENABLE();
}

static void set_downButton_intr()
{
    ETS_GPIO_INTR_DISABLE();

    if(buttons.downButton_flag)
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_HILEVEL);
    }
    else
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_LOLEVEL);
    }

    ETS_GPIO_INTR_ENABLE();
}

static void upButton_intr_handler()
{
    switch (programmingState) {
        case UPLIMIT:
            DEBUG("UpButton case: UPLIMIT!\n");
            DEBUG("\tflag = %d!\n", buttons.upButton_flag);
            if (!buttons.upButton_flag) {
                buttons.upButton_flag = 1;
            } else {
                buttons.upButton_flag = 0;
            }
            break;

        case LOLIMIT:
            if (!buttons.upButton_flag) {
                DEBUG("\t\tUP START\n");
                buttons.upButton_flag = 1;
                buttons.upButton_offset = WDEV_NOW();
            } else {
                DEBUG("\t\tUP END\n");
                buttons.upButton_flag = 0;
                limit += WDEV_NOW() - buttons.upButton_offset;

                float num = (float) (WDEV_NOW() - buttons.upButton_offset) / 1000000.0;
                DEBUG("\tUp: + %d.%d seconds\n\r", (int) num, (int) ((num - (int) num) * 100));
            }
            break;

        default:
            DEBUG("Up button default state\n");
            break;
    }
}

static void downButton_intr_handler()
{
    switch (programmingState) {
        case UPLIMIT:
            DEBUG("DownButton case: UPLIMIT!\n");
            DEBUG("\tflag = %d!\n", buttons.downButton_flag);
            if (!buttons.downButton_flag) {
                buttons.downButton_flag = 1;
            } else {
                buttons.downButton_flag = 0;
            }
            break;

        case LOLIMIT:
            if (!buttons.downButton_flag) {
                DEBUG("\t\tDOWN START\n");
                buttons.downButton_flag = 1;
                buttons.downButton_offset = WDEV_NOW();
            } else {
                DEBUG("\t\tDOWN END\n");
                buttons.downButton_flag = 0;
                limit -= WDEV_NOW() - buttons.downButton_offset;

                float num = (float) (WDEV_NOW() - buttons.downButton_offset) / 1000000.0;
                DEBUG("\tDown: - %d.%d seconds\n\r", (int) num, (int) ((num - (int) num) * 100));
            }
            break;

        default:
            DEBUG("Down button default state\n");
            break;
    }

}

static void double_intr_handler()
{
    DEBUG("#DOUBLE INTERRUPT!\n\r");

    switch (programmingState)
    {
        case UPLIMIT:
            programmingState = LOLIMIT;
            break;
        case LOLIMIT:
            programmingState = COMPL;
            DEBUG("\tLimit: %d.%d seconds\n\r", (int) (limit / 1000000.0),
                      (int) (((limit / 1000000.0) - (int) (limit / 1000000.0)) * 100));
            programming_mode_close();
            break;
    }

    buttons.upButton_flag = 0;
    buttons.downButton_flag = 0;
}

static void gpio_intr_handler()
{
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    ETS_GPIO_INTR_DISABLE();

    if( gpio_status & BIT(0) )
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_DISABLE);
        upButton_intr_handler();
        os_timer_arm(&buttons.upButton_timer, 50, 0);
    }
    else if( gpio_status & BIT(2) )
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_DISABLE);
        downButton_intr_handler();
        os_timer_arm(&buttons.downButton_timer, 50, 0);
    }

    if(buttons.upButton_flag && buttons.downButton_flag)
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_DISABLE);
        gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_DISABLE);

        double_intr_handler();

        os_timer_arm(&buttons.upButton_timer, 1000, 0);
        os_timer_arm(&buttons.downButton_timer, 1000, 0);
    }

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    ETS_GPIO_INTR_ENABLE();
}

ICACHE_FLASH_ATTR static void set_gpio_intr()
{
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(0));

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2));

    ETS_GPIO_INTR_DISABLE();

    ETS_GPIO_INTR_ATTACH(gpio_intr_handler, NULL);
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_LOLEVEL);
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_LOLEVEL);

    programmingState = UPLIMIT;

    ETS_GPIO_INTR_ENABLE();
}

ICACHE_FLASH_ATTR static void programming_mode_init()
{
    programmingState = PREP;

    buttons.downButton_flag = 0;
    buttons.downButton_flag = 0;

    os_timer_setfn(&buttons.upButton_timer, (os_timer_func_t*) set_upButton_intr, 0);
    os_timer_setfn(&buttons.downButton_timer, (os_timer_func_t*) set_downButton_intr, 0);

    limit = 0;

    set_gpio_intr();
}

ICACHE_FLASH_ATTR void set_limits(void(*callback)())
{
    programmingState = IDLE;
    setLimitsCb = callback;
    programming_mode_init();

}