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
#include "freertos/queue.h"
#include "../inc/sapi_lcd.h"
#include "../inc/app_lcd.h"
#include "../inc/app_adc.h"
#include "../inc/app_fsm.h"

/*=====[Definition macros of private constants]==============================*/

#define V_CANT_DIG 3
#define I_CANT_DIG 4

//  WELCOME
#define WELCOME_1  "+------------------+"
#define WELCOME_2  "|  Trabajo  Final  |"
#define WELCOME_3  "|       CESE       |"
#define WELCOME_4  "+------------------+"

//	WAITING
#define WAITING_1  "+------------------+"
#define WAITING_2  "|   Esperando...   |"
#define WAITING_3  "|  Test o Config   |"
#define WAITING_4  "+------------------+"

//  NOT_CONFIGURATED
#define NOT_CONFIGURATED_1  "+------------------+"
#define NOT_CONFIGURATED_2  "|    Equipo no     |"
#define NOT_CONFIGURATED_3  "|   configurado    |"
#define NOT_CONFIGURATED_4  "+------------------+"

//	CONFIGURATION_OK
#define CONFIGURATION_OK_1  "+------------------+"
#define CONFIGURATION_OK_2  "|      Equipo      |"
#define CONFIGURATION_OK_3  "|  configurado OK  |"
#define CONFIGURATION_OK_4  "+------------------+"

//	CONFIGURATION_FAIL
#define CONFIGURATION_FAIL_1  "+------------------+"
#define CONFIGURATION_FAIL_2  "|    Falla en      |"
#define CONFIGURATION_FAIL_3  "|  configuracion   |"
#define CONFIGURATION_FAIL_4  "+------------------+"

//	MEASURING_PRIMARY
#define MEASURING_PRIMARY_1  "Caracterizando trafo"
#define MEASURING_PRIMARY_2  " Midiendo Primario  "
#define MEASURING_PRIMARY_3  "Vp=    V  Ip=     mA"
#define MEASURING_PRIMARY_4  "Vs=    V            "

//	MEASURING_SECONDARY
#define MEASURING_SECONDARY_1  "Caracterizando trafo"
#define MEASURING_SECONDARY_2  "Midiendo Secundario "
#define MEASURING_SECONDARY_3  "Vp=    V            "
#define MEASURING_SECONDARY_4  "Vs=    V  Is=     mA"

//	REPORT_LCD
#define REPORT_LCD_1  "+------------------+"
#define REPORT_LCD_2  "|    Enviando      |"
#define REPORT_LCD_3  "|     reporte      |"
#define REPORT_LCD_4  "+------------------+"

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

TaskHandle_t xHandle;

/*=====[Definitions of internal functions]===================================*/

void appLcd_task(void *arg)
{
	lcd_config_t lcd_config;
	rms_t rms;
	lcd_msg_t lcd_msg;
	lcd_msg_t msg;

	vTaskDelay(200 / portTICK_PERIOD_MS);

	// Inicializar LCD de 20x2 (caracteres x lineas) con cada caracter de 5x8 pixeles
	lcd_config.lineWidth = 20 ;
	lcd_config.amountOfLines = 4 ;
	lcd_config.lcd_set = LCD_SET_4BITMODE_2LINE;
	lcd_config.lcd_control = LCD_CONTROL_CURSOROFF_BLINKOFF;
	lcd_config.lcd_entry_mode = LCD_ENTRY_MODE_LEFT;

	lcdInit( &lcd_config );

	msg = WELCOME;

	while(1) {
		if(xQueueReceive(lcd_queue, &lcd_msg, 0) == pdTRUE) {
			msg = lcd_msg;
			switch(msg) {
			case WELCOME:
				lcdGoToXY(0,0);
				lcdSendString(WELCOME_1);
				lcdSendString(WELCOME_2);
				lcdSendString(WELCOME_3);
				lcdSendString(WELCOME_4);
				break;
			case WAITING:
				lcdGoToXY(0,0);
				lcdSendString(WAITING_1);
				lcdSendString(WAITING_2);
				lcdSendString(WAITING_3);
				lcdSendString(WAITING_4);
				break;
			case CONFIGURATION_OK:
				lcdGoToXY(0,0);
				lcdSendString(CONFIGURATION_OK_1);
				lcdSendString(CONFIGURATION_OK_2);
				lcdSendString(CONFIGURATION_OK_3);
				lcdSendString(CONFIGURATION_OK_4);
				break;
			case NOT_CONFIGURATED:
				lcdGoToXY(0,0);
				lcdSendString(NOT_CONFIGURATED_1);
				lcdSendString(NOT_CONFIGURATED_2);
				lcdSendString(NOT_CONFIGURATED_3);
				lcdSendString(NOT_CONFIGURATED_4);
				break;
			case MEASURING_PRIMARY:
				lcdGoToXY(0,0);
				lcdSendString(MEASURING_PRIMARY_1);
				lcdSendString(MEASURING_PRIMARY_2);
				lcdSendString(MEASURING_PRIMARY_3);
				lcdSendString(MEASURING_PRIMARY_4);
				break;
			case MEASURING_SECONDARY:
				lcdGoToXY(0,0);
				lcdSendString(MEASURING_SECONDARY_1);
				lcdSendString(MEASURING_SECONDARY_2);
				lcdSendString(MEASURING_SECONDARY_3);
				lcdSendString(MEASURING_SECONDARY_4);
				break;
			case REPORT_LCD:
				lcdGoToXY(0,0);
				lcdSendString(REPORT_LCD_1);
				lcdSendString(REPORT_LCD_2);
				lcdSendString(REPORT_LCD_3);
				lcdSendString(REPORT_LCD_4);
				break;
			default:
				break;
			}
		}

		switch(msg) {
		case MEASURING_PRIMARY:
			xQueueReceive(rms_queue, &rms, portMAX_DELAY);

			// Tension Primaria
			lcdGoToXY(4,2);
			lcdSendIntFixedDigit( rms.Vp, V_CANT_DIG, 0 );
			// Corriente Primaria
			lcdGoToXY(14,2);
			lcdSendIntFixedDigit( rms.Ip, I_CANT_DIG, 0 );
			// Tension Secundaria
			lcdGoToXY(3,3);
			lcdSendIntFixedDigit( rms.Vs/10, V_CANT_DIG, 1 );
			break;
		case MEASURING_SECONDARY:
			xQueueReceive(rms_queue, &rms, portMAX_DELAY);

			// Tension Primaria
			lcdGoToXY(4,2);
			lcdSendIntFixedDigit( rms.Vp, V_CANT_DIG, 0 );
			// Tension Secundaria
			lcdGoToXY(3,3);
			lcdSendIntFixedDigit( rms.Vs/10, V_CANT_DIG, 1 );
			// Corriente Secundaria
			lcdGoToXY(14,3);
			lcdSendIntFixedDigit( rms.Is, I_CANT_DIG, 0 );
			break;
		default:
			vTaskDelay(500 / portTICK_PERIOD_MS);
			break;
		}
	}
}

/*=====[Definitions of external functions]===================================*/

void appLcdInit(void)
{
	xTaskCreate(appLcd_task, "appLcd_task", 2048, NULL, 5, &xHandle);
}
