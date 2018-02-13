#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#define GPIO_OUTPUT_SET(gpio_no, bit_value)
char* buf;

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

/* Devuelve la correspondencia en Morse del carácter c */
const char* morse (char c) {
  static const char* morse_ch [] = {
    ". -", /* A */
    "- . . .", /* B */
    "- . - .", /* C */
    "- . .", /* D */
    ".", /* E */
    ". . -.", /* F */
    "- - .", /* G */
    ". . . .", /* H */
    ". .", /* I */
    ". - - -", /* J */
    "- . -", /* K */
    ". - . .", /* L */
    "- -", /* M */
    "- .", /* N */
    "- - -", /* O */
    ". - -.", /* P */
    "- - . -", /* Q */
    ". - .", /* R */
    ". . .", /* S */
    "-", /* T */
    ". . -", /* U */
    ". . . -", /* V */
    ". - -", /* W */
    "- . . -", /* X */
    "- . - -", /* Y */
    "- - . .", /* Z */
    "- - - - -", /* 0 */
    ". - - - -", /* 1 */
    ". . - - -", /* 2 */
    ". . . - -", /* 3 */
    ". . . . -", /* 4 */
    ". . . . .", /* 5 */
    "- . . . .", /* 6 */
    "- - . . .", /* 7 */
    "- - - . .", /* 8 */
    "- - - - .", /* 9 */
    ". . . . . .", /* . */
    ". - . - . -", /* , */
    ". . - - . .", /* ? */
    "- - . . - -" /* ! */
  };

  return morse_ch[c - 'a'];
}

/* Copia en buf la versión en Morse del mensaje str, con un límite de n caracteres */
int str2morse (char* buf , int n , const char* str) {

  for (size_t i = 0; i < n && str[i] != '\0'; i++) {
    if (str[i] != ' ') {
      char letra = str[i];
      buf = strcat(buf, morse(letra));
      //añadir separación entre símbolos (eq 1 punto) -> hecho en morse
      //añadir separación entre letras (eq 3 puntos)
      buf = strcat(buf, "   ");
    } else{
      //añadir separación entre palabras (eq 5 puntos)
      buf = strcat(buf, "     ");
    }
  }
}

/* Envía el mensaje msg, ya codificado en Morse, encendiendo y apagando el LED */
void morse_send (const char* msg) {
  switch (*msg ) {
      case '.':   // enviar un punto: 250 ms
        gpio_output_set(0, BIT2, BIT2, 0);
        vTaskDelay(250/portTICK_RATE_MS);
        gpio_output_set(BIT2, 0, BIT2, 0);
        break ;
      case '-':   // enviar una raya: 3 * 250ms = 750 ms
        gpio_output_set(0, BIT2, BIT2, 0);
        vTaskDelay(750/portTICK_RATE_MS);
        gpio_output_set(BIT2, 0, BIT2, 0);
        break ;
      case ' ':   // pausa del tamaño de un punto: 250 ms
        gpio_output_set(BIT2, 0, BIT2, 0);
        vTaskDelay(250/portTICK_RATE_MS);
        gpio_output_set(BIT2, 0, BIT2, 0);
        break ;
    case '\0':
      return ;
    }
    morse_send (++msg ) ;
}

void ppal(void* ignore)
{
  char buf[256];
  str2morse (buf, 256, "hola- mundo ");
  while (true) {
    /* code */
    morse_send (buf);

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
    xTaskCreate(&ppal, "startup", 2048, NULL, 1, NULL);
}
