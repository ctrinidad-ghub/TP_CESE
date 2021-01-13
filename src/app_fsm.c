/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "../inc/app_lcd.h"
#include "../inc/app_gpio.h"
#include "../inc/app_fsm.h"
#include "../inc/app_adc.h"
#include "../inc/app_printer.h"
#include "../inc/app_WiFi.h"
#include "../inc/app_Comm.h"
#include "../inc/http_client.h"

/*=====[Definition of private macros, constants or data types]===============*/

#define ADC_ITERATION 10

typedef enum {
	STARTUP,
	WIFI_CONNECTION,
    WAIT_TEST,
	CONFIGURATING,
	ASK_FOR_CONFIGURATION,
	POWER_UP_PRIMARY,
	MEASURE_PRIMARY,
	POWER_DOWN_PRIMARY,
	CHARACTERIZE_PRIMARY,
	POWER_UP_SECONDARY,
	MEASURE_SECONDARY,
	POWER_DOWN_SECONDARY,
	CHARACTERIZE_SECONDARY,
	REPORT,
	CANCEL_FSM
} test_state_t;

typedef enum {
    WAIT_CONFIG,
	REQUEST_CONFIG_PARAMETERS,

} config_state_t;

typedef struct {
	int32_t max;
	int32_t min;
} parametersRange_t;

typedef struct {
	parametersRange_t Vp;
	parametersRange_t Vs;
	parametersRange_t Ip;
	parametersRange_t Is;
} trafoParameters_t;

typedef struct {
	test_state_t test_state;
	bool configurated;
	printer_msg_t printer_msg;
	trafoParameters_t trafoParameters;
	uint8_t fsm_timer;
	configData_t configData;
} deviceControl_t;

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

SemaphoreHandle_t checkTafo_semphr;
SemaphoreHandle_t checkTafoInProgress_semphr;

deviceControl_t deviceControl;
#define MAX_HTTP_RECV_BUFFER 2048
char rxHttp [MAX_HTTP_RECV_BUFFER] = "{ \"id_Dispositivo\": 1, \"lote_partida\": \"20200201-1\", \"test_Numero\": 4, \"tension_linea\": 220, \"corriente_vacio\": 40 }";

/*=====[Definitions of internal functions]===================================*/

bool isConfigurated(void)
{
	return deviceControl.configurated;
}
bool configurate(void)
{
	esp_err_t err;

	err = get_http_config(rxHttp, MAX_HTTP_RECV_BUFFER);

	if (err == ESP_OK){
		processRxData(rxHttp, &deviceControl.configData);
		deviceControl.configurated = 1;
	} else deviceControl.configurated = 0;

	return (deviceControl.configurated);
}

void checkTafo_task (void*arg)
{
	rms_t rms;

	while(1) {
		xSemaphoreTake(checkTafo_semphr, portMAX_DELAY);

		for (int i=0; i<ADC_ITERATION; i++) {
			appAdcStart(&rms);

			// TODO: add mutex to protect deviceControl.test_state
			if (deviceControl.test_state == CANCEL_FSM) break;
			else if (deviceControl.test_state == MEASURE_PRIMARY) appLcdSend(MEASURING_PRIMARY, &rms);
			else if (deviceControl.test_state == MEASURE_SECONDARY) appLcdSend(MEASURING_SECONDARY, &rms);
		}

		xSemaphoreGive(checkTafoInProgress_semphr);
	}
}

void fsm_task (void*arg)
{
	esp_err_t err;
	wifi_state_t wifi_state = WIFI_FAIL;

	disconnectPrimarySecondary();

	deviceControl.test_state = STARTUP;

	// Create semaphores
	checkTafo_semphr = xSemaphoreCreateBinary();
	checkTafoInProgress_semphr = xSemaphoreCreateBinary();

	deviceControl.configurated = 0;
	deviceControl.printer_msg = TEST_FAIL;
	deviceControl.fsm_timer = 0;

	xTaskCreate(checkTafo_task, "checkTafo_task", 1024 * 2, NULL, 5, NULL);

	// Wait until the LCD has initiated
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	while (1) {
		switch(deviceControl.test_state) {
		case STARTUP:
			disconnectPrimarySecondary();

			appLcdSend(WELCOME, NULL);

			vTaskDelay(3000 / portTICK_PERIOD_MS);
			appAdcDisable();
			deviceControl.test_state = WIFI_CONNECTION;
			break;
		case WIFI_CONNECTION:
			appLcdSend(WIFI_CONNECTING, NULL);
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			err = app_WiFiInit();
			if (err != ESP_OK) {
				appLcdSend(WIFI_NO_SSID_AND_PASS, NULL);
				vTaskDelay(3000 / portTICK_PERIOD_MS);
			}
			else {
				wifi_state = app_WiFiConnect();
				if (wifi_state != WIFI_CONNECTED) {
					appLcdSend(WIFI_NO_SSID_AND_PASS, NULL);
					vTaskDelay(3000 / portTICK_PERIOD_MS);
				}
			}
			while (wifi_state != WIFI_CONNECTED) {
				appLcdSend(WIFI_SMARTCONFIG, NULL);
				wifi_state = app_WiFiConnect();
				if (wifi_state != WIFI_CONNECTED) {
					appLcdSend(WIFI_SMARTCONFIG_FAIL, NULL);
					vTaskDelay(3000 / portTICK_PERIOD_MS);
				}
			}
			appLcdSend(WIFI_SUCCESSFULLY_CONNECTED, NULL);
			vTaskDelay(3000 / portTICK_PERIOD_MS);

			appLcdSend(WAITING, NULL);
			deviceControl.test_state = WAIT_TEST;
			break;
		case WAIT_TEST:
			disconnectPrimarySecondary();

			if ( isTestPressed( ) ) {
				if ( isConfigurated() ) {
					deviceControl.test_state = POWER_UP_PRIMARY;
					app_WiFiDisconnect();
					vTaskDelay(500 / portTICK_PERIOD_MS);
					appAdcEnable();
					connectPrimary();
					deviceControl.fsm_timer = 0;
				}
				else {
					deviceControl.test_state = ASK_FOR_CONFIGURATION;
					appLcdSend(NOT_CONFIGURATED, NULL);
				}
			}
			if ( isConfigPressed( ) ) {
				appLcdSend(CONFIGURATING_LCD, NULL);
				deviceControl.test_state = CONFIGURATING;
			}
			break;
		case CONFIGURATING:
			if( configurate() ){
				vTaskDelay(3000 / portTICK_PERIOD_MS);
				appLcdSend(CONFIGURATION_OK, NULL);
			}
			else {
				appLcdSend(CONFIGURATION_FAIL, NULL);
			}
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			appLcdSend(WAITING, NULL);
			deviceControl.test_state = WAIT_TEST;
			break;
		case ASK_FOR_CONFIGURATION:
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			appLcdSend(WAITING, NULL);
			deviceControl.test_state = WAIT_TEST;
			break;
		case POWER_UP_PRIMARY:
			if ( isCancelPressed( ) ) {
				deviceControl.test_state = CANCEL_FSM;
			}
			else if (deviceControl.fsm_timer == 4) {
				deviceControl.test_state = MEASURE_PRIMARY;
				xSemaphoreGive(checkTafo_semphr);
			} else deviceControl.fsm_timer++;
			break;
		case MEASURE_PRIMARY:
			if ( isCancelPressed( ) ) {
				deviceControl.test_state = CANCEL_FSM;
			}
			// Wait until the primary characterization is finished
			else if (uxSemaphoreGetCount(checkTafoInProgress_semphr) == pdTRUE) {
				xSemaphoreTake(checkTafoInProgress_semphr, portMAX_DELAY);
				disconnectPrimarySecondary();
				deviceControl.fsm_timer = 0;
				deviceControl.test_state = POWER_DOWN_PRIMARY;
			}
			break;
		case POWER_DOWN_PRIMARY:
			if ( isCancelPressed( ) ) {
				deviceControl.test_state = CANCEL_FSM;
			}
			else if (deviceControl.fsm_timer == 4) {
				deviceControl.fsm_timer = 0;
				connectSecondary();
				deviceControl.test_state = POWER_UP_SECONDARY;
			} else deviceControl.fsm_timer++;
			break;
		case POWER_UP_SECONDARY:
			if ( isCancelPressed( ) ) {
				deviceControl.test_state = CANCEL_FSM;
			}
			else if (deviceControl.fsm_timer == 4) {
				deviceControl.test_state = MEASURE_SECONDARY;
				xSemaphoreGive(checkTafo_semphr);
			} else deviceControl.fsm_timer++;
			break;
		case MEASURE_SECONDARY:
			if ( isCancelPressed( ) ) {
				deviceControl.test_state = CANCEL_FSM;
			}
			// Wait until the secondary characterization is finished
			else if (uxSemaphoreGetCount(checkTafoInProgress_semphr) == pdTRUE) {
				xSemaphoreTake(checkTafoInProgress_semphr, portMAX_DELAY);
				disconnectPrimarySecondary();
				deviceControl.fsm_timer = 0;
				deviceControl.test_state = POWER_DOWN_SECONDARY;
			}
			break;
		case POWER_DOWN_SECONDARY:
			disconnectPrimarySecondary();
			deviceControl.test_state = REPORT;
			break;
		case REPORT:
			appLcdSend(REPORT_LCD, NULL);
			appAdcDisable();
			vTaskDelay(300 / portTICK_PERIOD_MS);
			app_WiFiConnect();
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			appLcdSend(WAITING, NULL);
			deviceControl.test_state = WAIT_TEST;
			print(deviceControl.printer_msg);
			break;
		case CANCEL_FSM:
			appLcdSend(CANCEL_LCD, NULL);
			disconnectPrimarySecondary();

			// Wait until the ADC conversion finishes,
			// if we are in the middle of an ADC conversion
			xSemaphoreTake(checkTafoInProgress_semphr, 1000 / portTICK_PERIOD_MS);

			if (appAdcStatus() == ENABLE) appAdcDisable();
			app_WiFiConnect();
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			appLcdSend(WAITING, NULL);
			deviceControl.test_state = WAIT_TEST;
			break;
		default:
			vTaskDelay(500 / portTICK_PERIOD_MS);
			break;
		}
		vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/*=====[Definitions of external functions]===================================*/

void appFsmInit( void )
{
	xTaskCreate(fsm_task, "fsm_task", 1024 * 4, NULL, 5, NULL);
}
