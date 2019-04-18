#include "osapi.h"
//#include "ets_sys.h"
#include "user_interface.h"
#include "../include/control.h"
#include "../include/movement.h"

#define BUTTON_HOLD_MS 1000 // Eddig kell nyomva tartani a gombot a FOLYAMATOS MOZGÁSHOZ
#define MAX_LIMIT_DIFFERENCE_US 100000 // Ha ezt a számot nem éri el a LIMIT és a POZÍCIÓ különbsége, akkor már nem indítjuk el a mozgást.

static ButtonsStatus buttons;
static int32_t limit;
static int32_t position;

int32_t debugLimit;
int32_t debugPosition;

static os_timer_t limit_timer;

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

static void reach_the_limit() {

    MovementState movementState = get_movementState();
    movement_stop();
    os_timer_disarm(&limit_timer);

    if (movementState == UP)
    {
        position -= WDEV_NOW() - buttons.upButton_offset;
        debugPosition -= WDEV_NOW() - buttons.upButton_offset;
    }
    else if(movementState == DOWN)
    {
        position += WDEV_NOW() - buttons.downButton_offset;
        debugPosition += WDEV_NOW() - buttons.downButton_offset;
    }

    DEBUG("\t\t\t\tPosition = %d\n", debugPosition);

}

static void upButton_intr_handler()
{
    buttons.upButton_flag ^= 1u;

    if (buttons.upButton_flag)
    { // Gomb lenyomás történt
        DEBUG("UP BUTTON LENYOMAS\n");

        MovementState movementState = get_movementState();
        if(movementState == IDLE) //Ha nincs mozgásban a motor
        {
            buttons.upButton_offset = WDEV_NOW();
            if(position - MAX_LIMIT_DIFFERENCE_US > 0){
                DEBUG("\t\tUP START\n");
                os_timer_arm(&limit_timer, position / 1000, 0);
                up_movement_start();
            }
        }
        else //Ha mozgásban van a motor
        {
            reach_the_limit();
        }
    }
    else
    { // Gomb felengedés történt
        DEBUG("UP BUTTON FELENGED\n");
    }
}

static void downButton_intr_handler()
{
    buttons.downButton_flag ^= 1u;

    if (buttons.downButton_flag)
    { // Gomb lenyomás történt
        DEBUG("DOWN BUTTON LENYOMAS\n");

        MovementState movementState = get_movementState();
        if(movementState == IDLE) //Ha nincs mozgásban a motor
        {
            buttons.downButton_offset = WDEV_NOW();
            if(position + MAX_LIMIT_DIFFERENCE_US < limit){
                DEBUG("\t\tDOWN START\n");
                os_timer_arm(&limit_timer, (limit - position) / 1000, 0);
                down_movement_start();
            }
        }
        else //Ha mozgásban van a motor
        {
            reach_the_limit();
        }
    }
    else
    { // Gomb felengedés történt
        DEBUG("DOWN BUTTON FELENGED\n");
    }
}

static void gpio_intr_handler()
{
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    ETS_GPIO_INTR_DISABLE();

    if( gpio_status & BIT(0u) ) // 0u - 0 Unsigned, because operand with binary bitwise operator
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_DISABLE);
        upButton_intr_handler();
        os_timer_arm(&buttons.upButton_timer, 50, 0);
    }
    else if( gpio_status & BIT(2u) )
    {
        gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_DISABLE);
        downButton_intr_handler();
        os_timer_arm(&buttons.downButton_timer, 50, 0);
    }

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    ETS_GPIO_INTR_ENABLE();
}

static void set_gpio_intr()
{
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(0));

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2));

    ETS_GPIO_INTR_DISABLE();

    ETS_GPIO_INTR_ATTACH(&gpio_intr_handler, NULL);
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_LOLEVEL);
    gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_LOLEVEL);

    ETS_GPIO_INTR_ENABLE();
}

ICACHE_FLASH_ATTR void control_mode_init(int32_t limit_us)
{
    DEBUG("Initialing control mode...\n\n");

    buttons.downButton_flag = 0;
    buttons.downButton_flag = 0;

    os_timer_setfn(&buttons.upButton_timer, (os_timer_func_t*) set_upButton_intr, 0);
    os_timer_setfn(&buttons.downButton_timer, (os_timer_func_t*) set_downButton_intr, 0);

    os_timer_setfn(&limit_timer, (os_timer_func_t*) reach_the_limit, 0);

    limit = limit_us;
    position = limit_us;

    debugLimit = limit_us;
    debugPosition = limit_us;

    set_gpio_intr();
}