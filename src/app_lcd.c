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
#define LCD_WIDTH				20
#define LCD_LINE     			4

#define VP_AMOUNT_OF_DIG 		3
#define VS_AMOUNT_OF_DIG 		4 // Include the decimal point
#define VS_DECIMAL_POINT_POS 	1
#define DECIMAL_POINT_NOT_USED 	0
#define I_CANT_DIG 				4

typedef struct {
	char msg_1[LCD_WIDTH+1];
	char msg_2[LCD_WIDTH+1];
	char msg_3[LCD_WIDTH+1];
	char msg_4[LCD_WIDTH+1];
} lcd_msg;

//  WELCOME
#define WELCOME_1  "+------------------+\0"
#define WELCOME_2  "|  Trabajo  Final  |\0"
#define WELCOME_3  "|       CESE       |\0"
#define WELCOME_4  "+------------------+\0"

//  WIFI_CONNECTING
#define WIFI_CONNECTING_1  "+------------------+\0"
#define WIFI_CONNECTING_2  "|  Estableciendo   |\0"
#define WIFI_CONNECTING_3  "|  conexion WiFi   |\0"
#define WIFI_CONNECTING_4  "+------------------+\0"

//  WIFI_NO_SSID_AND_PASS
#define WIFI_NO_SSID_AND_PASS_1  "+------------------+\0"
#define WIFI_NO_SSID_AND_PASS_2  "| Fallo al intentar|\0"
#define WIFI_NO_SSID_AND_PASS_3  "|  conectar a WiFi |\0"
#define WIFI_NO_SSID_AND_PASS_4  "+------------------+\0"

//  WIFI_SMARTCONFIG
#define WIFI_SMARTCONFIG_1  "+------------------+\0"
#define WIFI_SMARTCONFIG_2  "|  Configure WiFi  |\0"
#define WIFI_SMARTCONFIG_3  "|  por SmartConfig |\0"
#define WIFI_SMARTCONFIG_4  "+------------------+\0"

//  WIFI_SMARTCONFIG_FAIL
#define WIFI_SMARTCONFIG_FAIL_1  "+------------------+\0"
#define WIFI_SMARTCONFIG_FAIL_2  "| Fallo al conectar|\0"
#define WIFI_SMARTCONFIG_FAIL_3  "|  por SmartConfig |\0"
#define WIFI_SMARTCONFIG_FAIL_4  "+------------------+\0"

//  WIFI_SUCCESSFULLY_CONNECTED
#define WIFI_SUCCESSFULLY_CONNECTED_1  "+------------------+\0"
#define WIFI_SUCCESSFULLY_CONNECTED_2  "| Conectado a red  |\0"
#define WIFI_SUCCESSFULLY_CONNECTED_3  "|       WiFi       |\0"
#define WIFI_SUCCESSFULLY_CONNECTED_4  "+------------------+\0"

//	WAITING
#define WAITING_1  "+------------------+\0"
#define WAITING_2  "|   Esperando...   |\0"
#define WAITING_3  "|  Test o Config   |\0"
#define WAITING_4  "+------------------+\0"

// CONFIGURATING_LCD
#define CONFIGURATING_LCD_1  "+------------------+\0"
#define CONFIGURATING_LCD_2  "|   Configurando   |\0"
#define CONFIGURATING_LCD_3  "|     Espere...    |\0"
#define CONFIGURATING_LCD_4  "+------------------+\0"

//  NOT_CONFIGURED
#define NOT_CONFIGURED_1  "+------------------+\0"
#define NOT_CONFIGURED_2  "|    Equipo no     |\0"
#define NOT_CONFIGURED_3  "|   configurado    |\0"
#define NOT_CONFIGURED_4  "+------------------+\0"

//	CONFIGURATION_OK
#define CONFIGURATION_OK_1  "+------------------+\0"
#define CONFIGURATION_OK_2  "|      Equipo      |\0"
#define CONFIGURATION_OK_3  "|  configurado OK  |\0"
#define CONFIGURATION_OK_4  "+------------------+\0"

//	CONFIGURATION_FAIL
#define CONFIGURATION_FAIL_1  "+------------------+\0"
#define CONFIGURATION_FAIL_2  "|    Falla en      |\0"
#define CONFIGURATION_FAIL_3  "|  configuracion   |\0"
#define CONFIGURATION_FAIL_4  "+------------------+\0"

//	MEASURING_PRIMARY
#define MEASURING_PRIMARY_1  "Caracterizando trafo\0"
#define MEASURING_PRIMARY_2  " Midiendo Primario  \0"
#define MEASURING_PRIMARY_3  "Vp=    V  Ip=     mA\0"
#define MEASURING_PRIMARY_4  "Vs=    V            \0"

//	MEASURING_SECONDARY
#define MEASURING_SECONDARY_1  "Caracterizando trafo\0"
#define MEASURING_SECONDARY_2  "Midiendo Secundario \0"
#define MEASURING_SECONDARY_3  "Vp=    V            \0"
#define MEASURING_SECONDARY_4  "Vs=    V  Is=     mA\0"

//	REPORT_LCD
#define REPORT_LCD_1  "+------------------+\0"
#define REPORT_LCD_2  "|    Enviando      |\0"
#define REPORT_LCD_3  "|     reporte      |\0"
#define REPORT_LCD_4  "+------------------+\0"

//	CANCEL_LCD
#define CANCEL_LCD_1  "+------------------+\0"
#define CANCEL_LCD_2  "|   Cancelando     |\0"
#define CANCEL_LCD_3  "|    espere..      |\0"
#define CANCEL_LCD_4  "+------------------+\0"

const lcd_msg lcd_text [] = {
	{WELCOME_1, WELCOME_2, WELCOME_3, WELCOME_4},
	{WIFI_CONNECTING_1, WIFI_CONNECTING_2, WIFI_CONNECTING_3, WIFI_CONNECTING_4},
	{WIFI_NO_SSID_AND_PASS_1, WIFI_NO_SSID_AND_PASS_2, WIFI_NO_SSID_AND_PASS_3, WIFI_NO_SSID_AND_PASS_4},
	{WIFI_SMARTCONFIG_1, WIFI_SMARTCONFIG_2, WIFI_SMARTCONFIG_3, WIFI_SMARTCONFIG_4},
	{WIFI_SMARTCONFIG_FAIL_1, WIFI_SMARTCONFIG_FAIL_2, WIFI_SMARTCONFIG_FAIL_3, WIFI_SMARTCONFIG_FAIL_4},
	{WIFI_SUCCESSFULLY_CONNECTED_1, WIFI_SUCCESSFULLY_CONNECTED_2, WIFI_SUCCESSFULLY_CONNECTED_3, WIFI_SUCCESSFULLY_CONNECTED_4},
	{WAITING_1, WAITING_2, WAITING_3, WAITING_4},
	{CONFIGURATING_LCD_1, CONFIGURATING_LCD_2, CONFIGURATING_LCD_3, CONFIGURATING_LCD_4},
	{NOT_CONFIGURED_1, NOT_CONFIGURED_2, NOT_CONFIGURED_3, NOT_CONFIGURED_4},
	{CONFIGURATION_OK_1, CONFIGURATION_OK_2, CONFIGURATION_OK_3, CONFIGURATION_OK_4},
	{CONFIGURATION_FAIL_1, CONFIGURATION_FAIL_2, CONFIGURATION_FAIL_3, CONFIGURATION_FAIL_4},
	{MEASURING_PRIMARY_1, MEASURING_PRIMARY_2, MEASURING_PRIMARY_3, MEASURING_PRIMARY_4},
	{MEASURING_SECONDARY_1, MEASURING_SECONDARY_2, MEASURING_SECONDARY_3, MEASURING_SECONDARY_4},
	{REPORT_LCD_1, REPORT_LCD_2, REPORT_LCD_3, REPORT_LCD_4},
	{CANCEL_LCD_1, CANCEL_LCD_2, CANCEL_LCD_3, CANCEL_LCD_4}
};

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

	// Initializing LCD of LCD_WIDTH x LCD_LINE with 5x8 pixels
	lcd_config.lineWidth = LCD_WIDTH;
	lcd_config.amountOfLines = LCD_LINE;
	lcd_config.lcd_set = LCD_SET_4BITMODE_2LINE;
	lcd_config.lcd_control = LCD_CONTROL_CURSOROFF_BLINKOFF;
	lcd_config.lcd_entry_mode = LCD_ENTRY_MODE_LEFT;

	lcdInit( &lcd_config );

	while(1) {
		xQueueReceive(lcd_queue, &lcd_msg, portMAX_DELAY);
		lcdGoToXY(0,0);
		lcdSendString(lcd_text[lcd_msg.lcd_msg_id].msg_1);
		lcdSendString(lcd_text[lcd_msg.lcd_msg_id].msg_2);
		lcdSendString(lcd_text[lcd_msg.lcd_msg_id].msg_3);
		lcdSendString(lcd_text[lcd_msg.lcd_msg_id].msg_4);

		switch(lcd_msg.lcd_msg_id) {
		case MEASURING_PRIMARY:
			// Primary Voltage
			lcdGoToXY(4,2);
			lcdSendIntFixedDigit( lcd_msg.rms.Vp, VP_AMOUNT_OF_DIG, DECIMAL_POINT_NOT_USED );
			// Primary Current
			lcdGoToXY(14,2);
			lcdSendIntFixedDigit( lcd_msg.rms.Ip, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
			// Secondary Voltage
			lcdGoToXY(3,3);
			lcdSendIntFixedDigit( lcd_msg.rms.Vs/10, VS_AMOUNT_OF_DIG, VS_DECIMAL_POINT_POS );
			break;
		case MEASURING_SECONDARY:
			// Primary Voltage
			lcdGoToXY(4,2);
			lcdSendIntFixedDigit( lcd_msg.rms.Vp, VP_AMOUNT_OF_DIG, DECIMAL_POINT_NOT_USED );
			// Secondary Voltage
			lcdGoToXY(3,3);
			lcdSendIntFixedDigit( lcd_msg.rms.Vs/10, VS_AMOUNT_OF_DIG, VS_DECIMAL_POINT_POS );
			// Secondary Current
			lcdGoToXY(14,3);
			lcdSendIntFixedDigit( lcd_msg.rms.Is, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
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
	if(lcd_queue == NULL)
	{
		// TODO: Define error policy
		while(1);
	}
	BaseType_t res = xTaskCreate(appLcd_task, "appLcd_task", 2048, NULL, 5, NULL);
	if (res != pdPASS)
	{
		// TODO: Define error policy
		while(1);
	}
}
