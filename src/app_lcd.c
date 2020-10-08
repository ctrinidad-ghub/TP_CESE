/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "../inc/sapi_lcd.h"
#include "../inc/app_lcd.h"
#include "../inc/app_adc.h"

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

/*=====[Definitions of private global variables]=============================*/

TaskHandle_t xHandle;

/*=====[Definitions of internal functions]===================================*/

void print_task(void *arg)
{
	while(1) {
		// Tension Primaria
		lcdGoToXY(4,2);
		lcdSendIntFixedDigit( Vp/10, V_CANT_DIG, 0 );
		// Corriente Primaria
		lcdGoToXY(14,2);
		lcdSendIntFixedDigit( Ip, I_CANT_DIG, 0 );
		// Tension Secundaria
		lcdGoToXY(3,3);
		lcdSendIntFixedDigit( Vs/10, V_CANT_DIG, 1 );
		// Corriente Secundaria
		lcdGoToXY(14,3);
		lcdSendIntFixedDigit( Is, I_CANT_DIG, 0 );

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void appLcdInit_task(void *arg)
{
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

	xTaskCreate(print_task, "print_task", 2048, NULL, 5, NULL);

	vTaskDelete( xHandle );
}

/*=====[Definitions of external functions]===================================*/

void appLcdInit(void)
{
	// Initialize NVS
	lcd_config_t lcd_config;

	// Inicializar LCD de 20x2 (caracteres x lineas) con cada caracter de 5x8 pixeles
	lcd_config.lineWidth = 20 ;
	lcd_config.amountOfLines = 4 ;
	lcd_config.lcd_set = LCD_SET_4BITMODE_2LINE;
	lcd_config.lcd_control = LCD_CONTROL_CURSOROFF_BLINKOFF;
	lcd_config.lcd_entry_mode = LCD_ENTRY_MODE_LEFT;

	lcdInit( &lcd_config );

	xTaskCreate(appLcdInit_task, "appLcdInit_task", 2048, NULL, 5, &xHandle);
}
