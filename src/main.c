/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "../inc/sapi_lcd.h"
#include "../inc/adc.h"


/*=====[Definition macros of private constants]==============================*/

#define V_CANT_DIG 3
#define I_CANT_DIG 4

#define WELCOME_1  "+------------------+"
#define WELCOME_2  "|  Trabajo  Final  |"
#define WELCOME_3  "|       PCSE       |"
#define WELCOME_4  "+------------------+"

#define PPAL_1  "Caracterizando trafo"
#define PPAL_2  " Midiendo Primario  "
#define PPAL_3  "Vp=    V  Ip=     mA"
#define PPAL_4  "Vs=    V  Is=     mA"

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

char ConvertBuff[10];
uint32_t Vp = 0, Vs = 0, Ip = 10, Is = 0;

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/


void print_task(void *arg)
{
	while(1) {
		// Tension Primaria
		lcdGoToXY(4,2);
		lcdSendIntFixedDigit( rms/10, V_CANT_DIG, 0 );
		// Corriente Primaria
		lcdGoToXY(14,2);
		lcdSendIntFixedDigit( Ip, I_CANT_DIG, 0 );
		// Tension Secundaria
		lcdGoToXY(3,3);
		lcdSendIntFixedDigit( Vs, V_CANT_DIG, 1 );
		// Corriente Secundaria
		lcdGoToXY(14,3);
		lcdSendIntFixedDigit( Is, I_CANT_DIG, 0 );

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void app_main()
{
	// Initialize NVS
	lcd_config_t lcd_config;

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	// Inicializar LCD de 20x2 (caracteres x lineas) con cada caracter de 5x8 pixeles
	lcd_config.lineWidth = 20 ;
	lcd_config.amountOfLines = 4 ;
	lcd_config.lcd_set = LCD_SET_4BITMODE_2LINE;
	lcd_config.lcd_control = LCD_CONTROL_CURSOROFF_BLINKOFF;
	lcd_config.lcd_entry_mode = LCD_ENTRY_MODE_LEFT;

	lcdInit( &lcd_config );
	lcdSendString(WELCOME_1);
	lcdSendString(WELCOME_2);
	lcdSendString(WELCOME_3);
	lcdSendString(WELCOME_4);

	vTaskDelay(3000 / portTICK_PERIOD_MS);

	lcdGoToXY(0,0);
	lcdSendString(PPAL_1);
	lcdSendString(PPAL_2);
	lcdSendString(PPAL_3);
	lcdSendString(PPAL_4);

	adc_init( );

	xTaskCreate(print_task, "print_task", 2048, NULL, 5, NULL);
}
