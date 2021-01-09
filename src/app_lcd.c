/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "../inc/app_adc.h"
#include "../inc/sapi_lcd.h"
#include "../inc/app_lcd.h"

/*=====[Definition of private macros, constants or data types]===============*/

#define VP_AMOUNT_OF_DIG 		3
#define VS_AMOUNT_OF_DIG 		4 // Include the decimal point
#define VS_DECIMAL_POINT_POS 	1
#define DECIMAL_POINT_NOT_USED 	0
#define I_CANT_DIG 				4

//  WELCOME
#define WELCOME_1  "+------------------+"
#define WELCOME_2  "|  Trabajo  Final  |"
#define WELCOME_3  "|       CESE       |"
#define WELCOME_4  "+------------------+"

//  WIFI_CONNECTING
#define WIFI_CONNECTING_1  "+------------------+"
#define WIFI_CONNECTING_2  "|  Estableciendo   |"
#define WIFI_CONNECTING_3  "|  conexion WiFi   |"
#define WIFI_CONNECTING_4  "+------------------+"

//  WIFI_NO_SSID_AND_PASS
#define WIFI_NO_SSID_AND_PASS_1  "+------------------+"
#define WIFI_NO_SSID_AND_PASS_2  "| Fallo al intentar|"
#define WIFI_NO_SSID_AND_PASS_3  "|  conectar a WiFi |"
#define WIFI_NO_SSID_AND_PASS_4  "+------------------+"

//  WIFI_SMARTCONFIG
#define WIFI_SMARTCONFIG_1  "+------------------+"
#define WIFI_SMARTCONFIG_2  "|  Configure WiFi  |"
#define WIFI_SMARTCONFIG_3  "|  por SmartConfig |"
#define WIFI_SMARTCONFIG_4  "+------------------+"

//  WIFI_SMARTCONFIG_FAIL
#define WIFI_SMARTCONFIG_FAIL_1  "+------------------+"
#define WIFI_SMARTCONFIG_FAIL_2  "| Fallo al conectar|"
#define WIFI_SMARTCONFIG_FAIL_3  "|  por SmartConfig |"
#define WIFI_SMARTCONFIG_FAIL_4  "+------------------+"

//  WIFI_SUCCESSFULLY_CONNECTED
#define WIFI_SUCCESSFULLY_CONNECTED_1  "+------------------+"
#define WIFI_SUCCESSFULLY_CONNECTED_2  "| Conectado a red  |"
#define WIFI_SUCCESSFULLY_CONNECTED_3  "|       WiFi       |"
#define WIFI_SUCCESSFULLY_CONNECTED_4  "+------------------+"

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

//	CANCEL_LCD
#define CANCEL_LCD_1  "+------------------+"
#define CANCEL_LCD_2  "|   Cancelando     |"
#define CANCEL_LCD_3  "|    espere..      |"
#define CANCEL_LCD_4  "+------------------+"

typedef struct {
	lcd_msg_id_t lcd_msg_id;
	rms_t rms;
} lcd_msg_t;

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static QueueHandle_t lcd_queue;

/*=====[Definitions of internal functions]===================================*/

void appLcd_task(void *arg)
{
	lcd_config_t lcd_config;
	lcd_msg_t lcd_msg;

	vTaskDelay(200 / portTICK_PERIOD_MS);

	// Inicializar LCD de 20x2 (caracteres x lineas) con cada caracter de 5x8 pixeles
	lcd_config.lineWidth = 20 ;
	lcd_config.amountOfLines = 4 ;
	lcd_config.lcd_set = LCD_SET_4BITMODE_2LINE;
	lcd_config.lcd_control = LCD_CONTROL_CURSOROFF_BLINKOFF;
	lcd_config.lcd_entry_mode = LCD_ENTRY_MODE_LEFT;

	lcdInit( &lcd_config );

	while(1) {
		xQueueReceive(lcd_queue, &lcd_msg, portMAX_DELAY);
		switch(lcd_msg.lcd_msg_id) {
		case WELCOME:
			lcdGoToXY(0,0);
			lcdSendString(WELCOME_1);
			lcdSendString(WELCOME_2);
			lcdSendString(WELCOME_3);
			lcdSendString(WELCOME_4);
			break;
		case WIFI_CONNECTING:
			lcdGoToXY(0,0);
			lcdSendString(WIFI_CONNECTING_1);
			lcdSendString(WIFI_CONNECTING_2);
			lcdSendString(WIFI_CONNECTING_3);
			lcdSendString(WIFI_CONNECTING_4);
			break;
		case WIFI_NO_SSID_AND_PASS:
			lcdGoToXY(0,0);
			lcdSendString(WIFI_NO_SSID_AND_PASS_1);
			lcdSendString(WIFI_NO_SSID_AND_PASS_2);
			lcdSendString(WIFI_NO_SSID_AND_PASS_3);
			lcdSendString(WIFI_NO_SSID_AND_PASS_4);
			break;
		case WIFI_SMARTCONFIG_FAIL:
			lcdGoToXY(0,0);
			lcdSendString(WIFI_SMARTCONFIG_FAIL_1);
			lcdSendString(WIFI_SMARTCONFIG_FAIL_2);
			lcdSendString(WIFI_SMARTCONFIG_FAIL_3);
			lcdSendString(WIFI_SMARTCONFIG_FAIL_4);
			break;
		case WIFI_SMARTCONFIG:
			lcdGoToXY(0,0);
			lcdSendString(WIFI_SMARTCONFIG_1);
			lcdSendString(WIFI_SMARTCONFIG_2);
			lcdSendString(WIFI_SMARTCONFIG_3);
			lcdSendString(WIFI_SMARTCONFIG_4);
			break;
		case WIFI_SUCCESSFULLY_CONNECTED:
			lcdGoToXY(0,0);
			lcdSendString(WIFI_SUCCESSFULLY_CONNECTED_1);
			lcdSendString(WIFI_SUCCESSFULLY_CONNECTED_2);
			lcdSendString(WIFI_SUCCESSFULLY_CONNECTED_3);
			lcdSendString(WIFI_SUCCESSFULLY_CONNECTED_4);
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
			// Tension Primaria
			lcdGoToXY(4,2);
			lcdSendIntFixedDigit( lcd_msg.rms.Vp, VP_AMOUNT_OF_DIG, DECIMAL_POINT_NOT_USED );
			// Corriente Primaria
			lcdGoToXY(14,2);
			lcdSendIntFixedDigit( lcd_msg.rms.Ip, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
			// Tension Secundaria
			lcdGoToXY(3,3);
			lcdSendIntFixedDigit( lcd_msg.rms.Vs/10, VS_AMOUNT_OF_DIG, VS_DECIMAL_POINT_POS );
			break;
		case MEASURING_SECONDARY:
			lcdGoToXY(0,0);
			lcdSendString(MEASURING_SECONDARY_1);
			lcdSendString(MEASURING_SECONDARY_2);
			lcdSendString(MEASURING_SECONDARY_3);
			lcdSendString(MEASURING_SECONDARY_4);
			// Tension Primaria
			lcdGoToXY(4,2);
			lcdSendIntFixedDigit( lcd_msg.rms.Vp, VP_AMOUNT_OF_DIG, DECIMAL_POINT_NOT_USED );
			// Tension Secundaria
			lcdGoToXY(3,3);
			lcdSendIntFixedDigit( lcd_msg.rms.Vs/10, VS_AMOUNT_OF_DIG, VS_DECIMAL_POINT_POS );
			// Corriente Secundaria
			lcdGoToXY(14,3);
			lcdSendIntFixedDigit( lcd_msg.rms.Is, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
			break;
		case REPORT_LCD:
			lcdGoToXY(0,0);
			lcdSendString(REPORT_LCD_1);
			lcdSendString(REPORT_LCD_2);
			lcdSendString(REPORT_LCD_3);
			lcdSendString(REPORT_LCD_4);
			break;
		case CANCEL_LCD:
			lcdGoToXY(0,0);
			lcdSendString(CANCEL_LCD_1);
			lcdSendString(CANCEL_LCD_2);
			lcdSendString(CANCEL_LCD_3);
			lcdSendString(CANCEL_LCD_4);
			break;
		default:
			break;
		}
	}
}

/*=====[Definitions of external functions]===================================*/

void appLcdSend(lcd_msg_id_t lcd_msg_id, rms_t *rms) {
	lcd_msg_t lcd_msg;

	lcd_msg.lcd_msg_id = lcd_msg_id;

	if(rms != NULL) {
		lcd_msg.rms.Vp = rms->Vp;
		lcd_msg.rms.Ip = rms->Ip;
		lcd_msg.rms.Vs = rms->Vs;
		lcd_msg.rms.Is = rms->Is;
	}

	xQueueSend( lcd_queue, ( void * ) &lcd_msg, ( TickType_t ) 0 );
}

void appLcdInit(void) {
	lcd_queue = xQueueCreate(1, sizeof(lcd_msg_t));
	xTaskCreate(appLcd_task, "appLcd_task", 2048, NULL, 5, NULL);
}
