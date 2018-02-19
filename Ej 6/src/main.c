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
volatile int done0;
volatile int done15;
portTickType xWakeTime;
portTickType REBOUND_TICK;


enum fsm_state
{
    ACTIVATED,
    DESACTIVATED
};

void isr_gpio(void* arg) {
  static portTickType xLastISRTick0 = 0;
  static portTickType xLastISRTick15 = 0;
  uint32 status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
  portTickType now = xTaskGetTickCount ();

  if (status & BIT(0)) {
    if (now > xLastISRTick0) {
      xLastISRTick0 = now + REBOUND_TICK;
      done0 = 1;
    }
 //xLastISRTick0 = now + REBOUND_TICK;
  }

  if (status & BIT(15)) {
    if (now > xLastISRTick15) {
      xLastISRTick15 = now + REBOUND_TICK;
      done15 = 1;
    }
 //xLastISRTick15 = now + REBOUND_TICK;
  }
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status);
}

void led_on(fsm_t *this)
{
    done15=0;
    GPIO_OUTPUT_SET(2, 0);
}

void presencia_off(fsm_t *this)
{
    done0=0;
    GPIO_OUTPUT_SET(2, 1);
}

int pulsed(fsm_t *this)
{
    return done0;
}

int presencia_on(fsm_t* this){
    //que ?
     done0=0;

}

int presencia(fsm_t *this)
{
    return done15;
}

fsm_trans_t interruptor[] = {
    {DESACTIVATED, pulsed, ACTIVATED, presencia_on},
    {ACTIVATED, presencia, ACTIVATED, led_on},
    {ACTIVATED, pulsed, DESACTIVATED, presencia_off},
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

void alarma(void *ignore)
{
    fsm_t *fsm = fsm_new(interruptor);
    led_off(fsm);
    portTickType xLastWakeTime;

     PIN_FUNC_SELECT(GPIO_PIN_REG_15, FUNC_GPIO15);
     PIN_FUNC_SELECT(GPIO_PIN_REG_0, FUNC_GPIO0);
     GPIO_ConfigTypeDef io_conf_0;
    io_conf_0.GPIO_IntrType = GPIO_PIN_INTR_NEGEDGE;
    io_conf_0.GPIO_Mode = GPIO_Mode_Input;
    io_conf_0.GPIO_Pin = BIT(0);
    io_conf_0.GPIO_Pullup = GPIO_PullUp_EN;
    gpio_config(&io_conf_0);
    GPIO_ConfigTypeDef io_conf_15;
    io_conf_15.GPIO_IntrType = GPIO_PIN_INTR_NEGEDGE;
    io_conf_15.GPIO_Mode = GPIO_Mode_Input;
    io_conf_15.GPIO_Pin = BIT(15);
    io_conf_15.GPIO_Pullup = GPIO_PullUp_EN;
    gpio_config(&io_conf_15);
    gpio_intr_handler_register((void*)isr_gpio, NULL);
    gpio_pin_intr_state_set(0, GPIO_PIN_INTR_NEGEDGE);
    gpio_pin_intr_state_set(15, GPIO_PIN_INTR_POSEDGE);
    ETS_GPIO_INTR_ENABLE();

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
    xTaskCreate(&alarma, "startup", 2048, NULL, 1, NULL);
}