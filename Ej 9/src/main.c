#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"
#define PERIOD_TICK 100/portTICK_RATE_MS
#define ETS_GPIO_INTR_ENABLE() \
  _xt_isr_unmask(1<< ETS_GPIO_INUM)
#define ETS_GPIO_INTR_DISABLE() \
  _xt_isr_mask(1<< ETS_GPIO_INUM)

int codigo[3];
int contrasena[3] = {1, 1, 1};
int code_index, code_inserted;
int tiempo_pulsado=0;
portTickType REBOUND_TICK, xWakeTime;
volatile int done0;
volatile int done15;
volatile int codigo_listo;
volatile int nextTimeout;
enum fsm_state {
  DESACTIVAR,
  ACTIVAR,
  CODIGO,
  ON,
  OFF,
};
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
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
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

void apagar(fsm_t *self){
  GPIO_OUTPUT_SET(2,1);
  codigo_listo=0;
}

void led_on(fsm_t *self){
  done15 = 0;
  GPIO_OUTPUT_SET(2,0);
}


void isr_gpio(void* arg) {
  /*PIN_FUNC_SELECT(GPIO_PIN_REG_15, FUNC_GPIO15);
  PIN_FUNC_SELECT(GPIO_PIN_REG_0, FUNC_GPIO0);*/
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


int mirar_flag(fsm_t *self){
  if((code_index > 2) || (codigo[code_index]> 10)) {
    return 0;
  }
  return done0;
}

int timeout(fsm_t *self){
portTickType now = xTaskGetTickCount();
if(nextTimeout > now){
  return 1;
}
return 0;
}

int codigo_correcto(fsm_t *self){
  int correcto=0;
  if(code_index==3){
    for(int i=0; i<3; i++){
      if(codigo[i] == contrasena[i]){
        correcto++;
      }else{
        return 0;
      }
    }
    if(correcto==3){
      printf("codigo correcto/n");
      codigo_listo = 1;
      return 1;
    }
  }
  return 0;
}

int codigo_incorrecto(fsm_t *self){
  int correcto=0;
  if(code_index==3){
    for(int i=0; i<3; i++){
      if(codigo[i] == contrasena[i]){
        correcto++;
      }else{
        return 1;
      }
    }
  }
  return 0;
}

void update_code(fsm_t *self){
  done0=0;
  code_inserted++;
  printf("nueva pulsacion/n");
  nextTimeout = xTaskGetTickCount() + (60*1000)/portTICK_RATE_MS;
}

void next_index(fsm_t *self){
  code_index ++;
  printf("nuevo indice/n");
}

void limpiar_flag(fsm_t *self){
  code_index=0;
  code_inserted=0;
}

int code_ready(fsm_t *self){
  return codigo_listo;
}

void limpiar(fsm_t *self){
  codigo_listo=0;
}

int presencia(fsm_t *self){
  return done15;
}


static fsm_trans_t codigo_fsm[] = {
  { CODIGO, mirar_flag, CODIGO, update_code},
  { CODIGO, timeout, CODIGO, next_index},
  { CODIGO, codigo_correcto, CODIGO, limpiar_flag},
  { CODIGO, codigo_incorrecto, CODIGO, limpiar_flag},
  {-1, NULL, -1, NULL },
};

static fsm_trans_t alarma_fsm[] = {
  { DESACTIVAR, code_ready, ACTIVAR, limpiar},
  { ACTIVAR, code_ready, DESACTIVAR, apagar},
  { ACTIVAR, presencia, ACTIVAR, led_on},
  {-1, NULL, -1, NULL },
};


fsm_trans_t interruptor[] = {
    {OFF, pressed, ON, led_on},
    {ON, pressed, ON, led_on},    
    {ON, time_passed, OFF, apagar},
    {-1, NULL, -1, NULL},
};

void alarm(void* ignore)
{
  fsm_t* fsm_1 = fsm_new(codigo_fsm);
  fsm_t* fsm_2 = fsm_new(alarma_fsm);
  fsm_t* fsm_3 = fsm_new(interruptor);
  portTickType xLastWakeTime;

  PIN_FUNC_SELECT(GPIO_PIN_REG_15, FUNC_GPIO15);
  PIN_FUNC_SELECT(GPIO_PIN_REG_0, FUNC_GPIO0);

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

  while(true) {
    xLastWakeTime = xTaskGetTickCount();
    fsm_fire(fsm_1);
    fsm_fire(fsm_2);
    fsm_fire(fsm_3);
    vTaskDelayUntil(&xLastWakeTime, PERIOD_TICK);
  }
}
/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    xTaskCreate(&alarm, "startup", 2048, NULL, 1, NULL);
}
