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
#include "../inc/app_Comm.h"
#include "../inc/test_status.h"

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

//  BLOCK_DEVICE_LCD
#define BLOCK_DEVICE_1  "+------------------+\0"
#define BLOCK_DEVICE_2  "|Sin conexion WiFi |\0"
#define BLOCK_DEVICE_3  "| Equipo bloqueado |\0"
#define BLOCK_DEVICE_4  "+------------------+\0"

//  WIFI_NO_SSID_AND_PASS
#define WIFI_NO_SSID_AND_PASS_1  "+------------------+\0"
#define WIFI_NO_SSID_AND_PASS_2  "| Fallo al intentar|\0"
#define WIFI_NO_SSID_AND_PASS_3  "|  conectar a WiFi |\0"
#define WIFI_NO_SSID_AND_PASS_4  "+------------------+\0"

//  USE_WIFI_CONFIG
#define USE_WIFI_CONFIG_1  "+------------------+\0"
#define USE_WIFI_CONFIG_2  "| Desea configurar |\0"
#define USE_WIFI_CONFIG_3  "| WiFi nuevamente? |\0"
#define USE_WIFI_CONFIG_4  "+------------------+\0"

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

//	WAITING_TEST
#define WAITING_TEST_1  "LOTE:               \0"
#define WAITING_TEST_2  "VsHi=     Lo=      V\0"
#define WAITING_TEST_3  "IpHi=     Lo=     mA\0"
#define WAITING_TEST_4  "IsHi=     Lo=     mA\0"

//	WAIT_CONFIG
#define WAIT_CONFIG_1  "+------------------+\0"
#define WAIT_CONFIG_2  "|   Esperando...   |\0"
#define WAIT_CONFIG_3  "|  Configuracion   |\0"
#define WAIT_CONFIG_4  "+------------------+\0"

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
#define REPORT_LCD_1  "                    \0"
#define REPORT_LCD_2  "Vs=     V           \0"
#define REPORT_LCD_3  "Ip=     mA          \0"
#define REPORT_LCD_4  "Is=     mA          \0"

//  FAILED_REPORT_LCD
#define FAILED_REPORT_LCD_1  "+------------------+\0"
#define FAILED_REPORT_LCD_2  "| Fallo al enviar  |\0"
#define FAILED_REPORT_LCD_3  "|reporte al webserv|\0"
#define FAILED_REPORT_LCD_4  "+------------------+\0"

// BLOCK_DEVICE_WEB_SERVER_LCD
#define BLOCK_DEVICE_WEB_SERVER_LCD_1  "+------------------+\0"
#define BLOCK_DEVICE_WEB_SERVER_LCD_2  "| Falla Web Server |\0"
#define BLOCK_DEVICE_WEB_SERVER_LCD_3  "| Equipo bloqueado |\0"
#define BLOCK_DEVICE_WEB_SERVER_LCD_4  "+------------------+\0"

//	CANCEL_LCD
#define CANCEL_LCD_1  "+------------------+\0"
#define CANCEL_LCD_2  "|    Cancelando    |\0"
#define CANCEL_LCD_3  "|     espere..     |\0"
#define CANCEL_LCD_4  "+------------------+\0"

//  FAILED_PRINTER_COM
#define FAILED_PRINTER_COM_1  "+------------------+\0"
#define FAILED_PRINTER_COM_2  "|   Fallo de COM   |\0"
#define FAILED_PRINTER_COM_3  "|   con impresora  |\0"
#define FAILED_PRINTER_COM_4  "+------------------+\0"

const lcd_msg lcd_text [] = {
	{WELCOME_1, WELCOME_2, WELCOME_3, WELCOME_4},
	{WIFI_CONNECTING_1, WIFI_CONNECTING_2, WIFI_CONNECTING_3, WIFI_CONNECTING_4},
	{WIFI_NO_SSID_AND_PASS_1, WIFI_NO_SSID_AND_PASS_2, WIFI_NO_SSID_AND_PASS_3, WIFI_NO_SSID_AND_PASS_4},
	{USE_WIFI_CONFIG_1, USE_WIFI_CONFIG_2, USE_WIFI_CONFIG_3, USE_WIFI_CONFIG_4},
	{WIFI_SMARTCONFIG_1, WIFI_SMARTCONFIG_2, WIFI_SMARTCONFIG_3, WIFI_SMARTCONFIG_4},
	{WIFI_SMARTCONFIG_FAIL_1, WIFI_SMARTCONFIG_FAIL_2, WIFI_SMARTCONFIG_FAIL_3, WIFI_SMARTCONFIG_FAIL_4},
	{WIFI_SUCCESSFULLY_CONNECTED_1, WIFI_SUCCESSFULLY_CONNECTED_2, WIFI_SUCCESSFULLY_CONNECTED_3, WIFI_SUCCESSFULLY_CONNECTED_4},
	{WAITING_TEST_1, WAITING_TEST_2, WAITING_TEST_3, WAITING_TEST_4},
	{WAIT_CONFIG_1, WAIT_CONFIG_2, WAIT_CONFIG_3, WAIT_CONFIG_4},
	{CONFIGURATING_LCD_1, CONFIGURATING_LCD_2, CONFIGURATING_LCD_3, CONFIGURATING_LCD_4},
	{NOT_CONFIGURED_1, NOT_CONFIGURED_2, NOT_CONFIGURED_3, NOT_CONFIGURED_4},
	{CONFIGURATION_OK_1, CONFIGURATION_OK_2, CONFIGURATION_OK_3, CONFIGURATION_OK_4},
	{CONFIGURATION_FAIL_1, CONFIGURATION_FAIL_2, CONFIGURATION_FAIL_3, CONFIGURATION_FAIL_4},
	{MEASURING_PRIMARY_1, MEASURING_PRIMARY_2, MEASURING_PRIMARY_3, MEASURING_PRIMARY_4},
	{MEASURING_SECONDARY_1, MEASURING_SECONDARY_2, MEASURING_SECONDARY_3, MEASURING_SECONDARY_4},
	{REPORT_LCD_1, REPORT_LCD_2, REPORT_LCD_3, REPORT_LCD_4},
	{FAILED_REPORT_LCD_1, FAILED_REPORT_LCD_2, FAILED_REPORT_LCD_3, FAILED_REPORT_LCD_4},
	{CANCEL_LCD_1, CANCEL_LCD_2, CANCEL_LCD_3, CANCEL_LCD_4},
	{BLOCK_DEVICE_1, BLOCK_DEVICE_2, BLOCK_DEVICE_3, BLOCK_DEVICE_4},
	{BLOCK_DEVICE_WEB_SERVER_LCD_1, BLOCK_DEVICE_WEB_SERVER_LCD_2, BLOCK_DEVICE_WEB_SERVER_LCD_3, BLOCK_DEVICE_WEB_SERVER_LCD_4},
	{FAILED_PRINTER_COM_1, FAILED_PRINTER_COM_2, FAILED_PRINTER_COM_3, FAILED_PRINTER_COM_4}
};

typedef struct {
	lcd_msg_id_t lcd_msg_id;
	rms_t rms;
	configData_t *configData;
	test_status_t *test_status;
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
		case WAITING_TEST:
			lcdGoToXY(6,0);
			lcdSendString( (const char*) lcd_msg.configData->lote );
			lcdGoToXY(5,1);
			lcdSendIntFixedDigit( lcd_msg.configData->trafoParameters.Vs.max/10, VS_AMOUNT_OF_DIG, VS_DECIMAL_POINT_POS );
			lcdGoToXY(13,1);
			lcdSendIntFixedDigit( lcd_msg.configData->trafoParameters.Vs.min/10, VS_AMOUNT_OF_DIG, VS_DECIMAL_POINT_POS );
			lcdGoToXY(5,2);
			lcdSendIntFixedDigit( lcd_msg.configData->trafoParameters.Ip.max, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
			lcdGoToXY(13,2);
			lcdSendIntFixedDigit( lcd_msg.configData->trafoParameters.Ip.min, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
			lcdGoToXY(5,3);
			lcdSendIntFixedDigit( lcd_msg.configData->trafoParameters.Is.max, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
			lcdGoToXY(13,3);
			lcdSendIntFixedDigit( lcd_msg.configData->trafoParameters.Is.min, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
			break;
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
		case REPORT_LCD:
			lcdGoToXY(0,0);
			if (lcd_msg.test_status->test_result == TEST_PASS) {
				lcdSendString("      APROBADO      ");
			}
			else {
				lcdSendString("     RECHAZADO      ");
			}

			// Secondary Voltage
			lcdGoToXY(4,1);
			lcdSendIntFixedDigit( lcd_msg.test_status->rms.Vs/10, VS_AMOUNT_OF_DIG, VS_DECIMAL_POINT_POS );
			lcdGoToXY(9,1);
			if ((lcd_msg.test_status->test_result & VS_FAILED) && VS_FAILED)
				lcdSendString("  **FALLO**");
			else
				lcdSendString("     OK    ");
			// Primary Current
			lcdGoToXY(4,2);
			lcdSendIntFixedDigit( lcd_msg.test_status->rms.Ip, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
			lcdGoToXY(10,2);
			if ((lcd_msg.test_status->test_result & IP_FAILED) && IP_FAILED)
				lcdSendString(" **FALLO**");
			else
				lcdSendString("    OK    ");
			// Secondary Current
			lcdGoToXY(4,3);
			lcdSendIntFixedDigit( lcd_msg.test_status->rms.Is, I_CANT_DIG, DECIMAL_POINT_NOT_USED );
			lcdGoToXY(10,3);
			if ((lcd_msg.test_status->test_result & IS_FAILED) && IS_FAILED)
				lcdSendString(" **FALLO**");
			else
				lcdSendString("    OK    ");
			break;
		default:
			break;
		}
	}
}

/*=====[Definitions of external functions]===================================*/

void appLcdSend(lcd_msg_id_t lcd_msg_id, void *param) {
	lcd_msg_t lcd_msg;
	rms_t *rms;

	lcd_msg.lcd_msg_id = lcd_msg_id;

	switch(lcd_msg.lcd_msg_id) {
	case WAITING_TEST:
		lcd_msg.configData = (configData_t *) param;
		break;
	case MEASURING_PRIMARY:
	case MEASURING_SECONDARY:
		rms = (rms_t *) param;

		lcd_msg.rms.Vp = rms->Vp;
		lcd_msg.rms.Ip = rms->Ip;
		lcd_msg.rms.Vs = rms->Vs;
		lcd_msg.rms.Is = rms->Is;
		break;
	case REPORT_LCD:
		lcd_msg.test_status = (test_status_t *) param;
		break;
	default:
		break;
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
