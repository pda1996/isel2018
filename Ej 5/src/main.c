#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
#define PERIOD_TICK 100 / portTICK_RATE_MS
int tiempo_pulsado = 0;

enum fsm_state
{
    OFF,
    ON
};
void led_on(fsm_t *this)
{
    GPIO_OUTPUT_SET(2, 0);
    tiempo_pulsado = xTaskGetTickCount() * portTICK_RATE_MS + 60000;
}

void led_off(fsm_t *this)
{
    GPIO_OUTPUT_SET(2, 1);
}

int pressed(fsm_t *this)
{
   PIN_FUNC_SELECT(GPIO_PIN_REG_0, FUNC_GPIO0);
  PIN_FUNC_SELECT(GPIO_PIN_REG_15, FUNC_GPIO15);
  return ((!GPIO_INPUT_GET(0)) || (GPIO_INPUT_GET(15)));
}

int time_passed(fsm_t *this)
{
    if (xTaskGetTickCount() * portTICK_RATE_MS >= tiempo_pulsado)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

fsm_trans_t interruptor[] = {
    {OFF, pressed, ON, led_on},
    {ON, pressed, ON, led_on},    
    {ON, time_passed, OFF, led_off},
    {-1, NULL, -1, NULL},
};

uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map)
    {
    case FLASH_SIZE_4M_MAP_256_256:
        rf_cal_sec = 128 - 5;
        break;

    case FLASH_SIZE_8M_MAP_512_512:
        rf_cal_sec = 256 - 5;
        break;

    case FLASH_SIZE_16M_MAP_512_512:
    case FLASH_SIZE_16M_MAP_1024_1024:
        rf_cal_sec = 512 - 5;
        break;

    case FLASH_SIZE_32M_MAP_512_512:
    case FLASH_SIZE_32M_MAP_1024_1024:
        rf_cal_sec = 1024 - 5;
        break;

    default:
        rf_cal_sec = 0;
        break;
    }

    return rf_cal_sec;
}

void lamp_t(void *ignore)
{
    fsm_t *fsm = fsm_new(interruptor);
    led_off(fsm);
    portTickType xLastWakeTime;

    while (true)
    {
        xLastWakeTime = xTaskGetTickCount();
        fsm_fire(fsm);
        vTaskDelayUntil(&xLastWakeTime, PERIOD_TICK);
    }
    vTaskDelete(NULL);
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    xTaskCreate(&lamp_t, "startup", 2048, NULL, 1, NULL);
}