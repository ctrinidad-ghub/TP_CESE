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
#define MAX_HTTP_RECV_BUFFER 2048

typedef enum {
	STARTUP,
	WIFI_CONNECTION,
	WIFI_CONFIG,
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
	CANCEL_FSM,
	BLOCK_DEVICE
} test_state_t;

typedef struct {
	test_state_t test_state;
	bool configurated;
	printer_msg_t printer_msg;
	uint8_t fsm_timer;
	configData_t configData;
	SemaphoreHandle_t checkTafo_semphr;
	SemaphoreHandle_t checkTafoInProgress_semphr;
	SemaphoreHandle_t test_state_mutex;
	wifi_state_t wifi_state;
} deviceControl_t;

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static deviceControl_t deviceControl;

static char buffHttp [MAX_HTTP_RECV_BUFFER];

/*=====[Definitions of internal functions]===================================*/

bool isConfigurated(void)
{
	return deviceControl.configurated;
}

void cleanConfiguration(void){
	deviceControl.configData.id = 0;
	for(int i=0; i<LOTE_LENGTH; i++) deviceControl.configData.lote[i]=0;
	deviceControl.configData.test_num = 0;
	deviceControl.configData.trafoParameters.Vp.max = 0;
	deviceControl.configData.trafoParameters.Vp.min = 0;
	deviceControl.configData.trafoParameters.Ip.max = 0;
	deviceControl.configData.trafoParameters.Ip.min = 0;
	deviceControl.configData.trafoParameters.Vs.max = 0;
	deviceControl.configData.trafoParameters.Vs.min = 0;
	deviceControl.configData.trafoParameters.Is.max = 0;
	deviceControl.configData.trafoParameters.Is.min = 0;
	deviceControl.configurated = 0;
}

bool configurate(void)
{
	esp_err_t err;

	for(int i=0; i<sizeof(buffHttp);i++) buffHttp[i] = 0;

	cleanConfiguration();

	err = get_http_config(buffHttp, MAX_HTTP_RECV_BUFFER);

	if (err == ESP_OK){
		processRxData(buffHttp, &deviceControl.configData);
		deviceControl.configurated = 1;
	} else deviceControl.configurated = 0;

	return (deviceControl.configurated);
}

void checkTafo_task (void*arg)
{
	rms_t rms;

	while(1) {
		xSemaphoreTake(deviceControl.checkTafo_semphr, portMAX_DELAY);

		for (int i=0; i<ADC_ITERATION; i++) {
			appAdcStart(&rms);

			xSemaphoreTake(deviceControl.test_state_mutex, portMAX_DELAY);
			if (deviceControl.test_state == CANCEL_FSM){
				xSemaphoreGive(deviceControl.test_state_mutex);
				break;
			}
			else if (deviceControl.test_state == MEASURE_PRIMARY) appLcdSend(MEASURING_PRIMARY, &rms);
			else if (deviceControl.test_state == MEASURE_SECONDARY) appLcdSend(MEASURING_SECONDARY, &rms);
			xSemaphoreGive(deviceControl.test_state_mutex);
		}

		xSemaphoreGive(deviceControl.checkTafoInProgress_semphr);
	}
}

void fsm_task (void*arg)
{
	esp_err_t err;
	uint8_t countWiFiAttempt = 0;

	disconnectPrimarySecondary();

	deviceControl.test_state = STARTUP;
	deviceControl.wifi_state = WIFI_NO_INIT;

	// Create semaphores and mutex
	deviceControl.checkTafo_semphr = xSemaphoreCreateBinary();
	deviceControl.checkTafoInProgress_semphr = xSemaphoreCreateBinary();
	deviceControl.test_state_mutex = xSemaphoreCreateMutex();
	if( deviceControl.checkTafo_semphr == NULL || deviceControl.checkTafoInProgress_semphr == NULL ||
		deviceControl.test_state_mutex == NULL )
	{
		// TODO: Define error policy
		while(1);
	}
	deviceControl.configurated = 0;
	deviceControl.printer_msg = TEST_FAIL;
	deviceControl.fsm_timer = 0;
	cleanConfiguration();

	BaseType_t res = xTaskCreate(checkTafo_task, "checkTafo_task", 1024 * 2, NULL, 5, NULL);
	if (res != pdPASS)
	{
		// TODO: Define error policy
		while(1);
	}

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
				deviceControl.test_state = WIFI_CONFIG;
				break;
			}
			else {
				deviceControl.wifi_state = app_WiFiConnect();
				if (deviceControl.wifi_state != WIFI_CONNECTED) {
					appLcdSend(WIFI_NO_SSID_AND_PASS, NULL);
					vTaskDelay(3000 / portTICK_PERIOD_MS);
					deviceControl.test_state = WIFI_CONFIG;
					break;
				}
			}

			appLcdSend(WIFI_SUCCESSFULLY_CONNECTED, NULL);
			vTaskDelay(3000 / portTICK_PERIOD_MS);

			appLcdSend(WAIT_CONFIG, NULL);
			deviceControl.test_state = WAIT_TEST;
			break;

		case WIFI_CONFIG:
			appLcdSend(USE_WIFI_CONFIG, NULL);
			if ( isConfigPressed( ) ) {
				countWiFiAttempt = 0;
				while (deviceControl.wifi_state != WIFI_CONNECTED) {
					appLcdSend(WIFI_SMARTCONFIG, NULL);
					deviceControl.wifi_state = app_WiFiConnect();
					if (deviceControl.wifi_state != WIFI_CONNECTED) {
						appLcdSend(WIFI_SMARTCONFIG_FAIL, NULL);
						vTaskDelay(3000 / portTICK_PERIOD_MS);
						countWiFiAttempt++;
						if (countWiFiAttempt == 3) {
							appLcdSend(BLOCK_DEVICE_LCD, NULL);
							deviceControl.test_state = BLOCK_DEVICE;
							break;
						}
					}
				}
				if (deviceControl.wifi_state == WIFI_CONNECTED) {
					appLcdSend(WIFI_SUCCESSFULLY_CONNECTED, NULL);
					vTaskDelay(3000 / portTICK_PERIOD_MS);

					appLcdSend(WAIT_CONFIG, NULL);
					deviceControl.test_state = WAIT_TEST;
				}
			}
			if ( isCancelPressed( ) ) {
				appLcdSend(BLOCK_DEVICE_LCD, NULL);
				deviceControl.test_state = BLOCK_DEVICE;
			}
			break;
		case WAIT_TEST:
			disconnectPrimarySecondary();

			if ( isTestPressed( ) ) {
				if ( isConfigurated() ) {
					deviceControl.test_state = POWER_UP_PRIMARY;
					deviceControl.wifi_state = app_WiFiDisconnect();
					vTaskDelay(500 / portTICK_PERIOD_MS);
					appAdcEnable();
					connectPrimary();
					deviceControl.fsm_timer = 0;
				}
				else {
					deviceControl.test_state = ASK_FOR_CONFIGURATION;
					appLcdSend(NOT_CONFIGURED, NULL);
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
				vTaskDelay(3000 / portTICK_PERIOD_MS);
			    appLcdSend(WAITING_TEST, &deviceControl.configData);
			}
			else {
				appLcdSend(CONFIGURATION_FAIL, NULL);
			    vTaskDelay(3000 / portTICK_PERIOD_MS);
			    appLcdSend(WAIT_CONFIG, NULL);
			} 
			deviceControl.test_state = WAIT_TEST;
			break;
		case ASK_FOR_CONFIGURATION:
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			appLcdSend(WAIT_CONFIG, NULL);
			deviceControl.test_state = WAIT_TEST;
			break;
		case POWER_UP_PRIMARY:
			if ( isCancelPressed( ) ) {
				xSemaphoreTake(deviceControl.test_state_mutex, portMAX_DELAY);
				deviceControl.test_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_state_mutex);
			}
			else if (deviceControl.fsm_timer == 4) {
				deviceControl.test_state = MEASURE_PRIMARY;
				xSemaphoreGive(deviceControl.checkTafo_semphr);
			} else deviceControl.fsm_timer++;
			break;
		case MEASURE_PRIMARY:
			if ( isCancelPressed( ) ) {
				xSemaphoreTake(deviceControl.test_state_mutex, portMAX_DELAY);
				deviceControl.test_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_state_mutex);
			}
			// Wait until the primary characterization is finished
			else if (uxSemaphoreGetCount(deviceControl.checkTafoInProgress_semphr) == pdTRUE) {
				xSemaphoreTake(deviceControl.checkTafoInProgress_semphr, portMAX_DELAY);
				disconnectPrimarySecondary();
				deviceControl.fsm_timer = 0;
				deviceControl.test_state = POWER_DOWN_PRIMARY;
			}
			break;
		case POWER_DOWN_PRIMARY:
			if ( isCancelPressed( ) ) {
				xSemaphoreTake(deviceControl.test_state_mutex, portMAX_DELAY);
				deviceControl.test_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_state_mutex);
			}
			else if (deviceControl.fsm_timer == 4) {
				deviceControl.fsm_timer = 0;
				connectSecondary();
				deviceControl.test_state = POWER_UP_SECONDARY;
			} else deviceControl.fsm_timer++;
			break;
		case POWER_UP_SECONDARY:
			if ( isCancelPressed( ) ) {
				xSemaphoreTake(deviceControl.test_state_mutex, portMAX_DELAY);
				deviceControl.test_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_state_mutex);
			}
			else if (deviceControl.fsm_timer == 4) {
				deviceControl.test_state = MEASURE_SECONDARY;
				xSemaphoreGive(deviceControl.checkTafo_semphr);
			} else deviceControl.fsm_timer++;
			break;
		case MEASURE_SECONDARY:
			if ( isCancelPressed( ) ) {
				xSemaphoreTake(deviceControl.test_state_mutex, portMAX_DELAY);
				deviceControl.test_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_state_mutex);
			}
			// Wait until the secondary characterization is finished
			else if (uxSemaphoreGetCount(deviceControl.checkTafoInProgress_semphr) == pdTRUE) {
				xSemaphoreTake(deviceControl.checkTafoInProgress_semphr, portMAX_DELAY);
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

			// Reconnect to WiFi
			deviceControl.wifi_state = app_WiFiConnect();
			if (deviceControl.wifi_state != WIFI_CONNECTED) {
				appLcdSend(FAILED_REPORT_LCD, NULL);
				vTaskDelay(3000 / portTICK_PERIOD_MS);
				appLcdSend(BLOCK_DEVICE_LCD, NULL);
				deviceControl.test_state = BLOCK_DEVICE;
				break;
			}

			// Write test results to the web server
			processTxData(buffHttp, &deviceControl.configData);
			err = post_http_results(buffHttp);
			if (err != ESP_OK) {
				appLcdSend(FAILED_REPORT_LCD, NULL);
				vTaskDelay(3000 / portTICK_PERIOD_MS);
				appLcdSend(BLOCK_DEVICE_WEB_SERVER_LCD, NULL);
				deviceControl.test_state = BLOCK_DEVICE;
				break;
			}

			// Send data to the printer
			printerStatus_t printerStatus = print(deviceControl.printer_msg, deviceControl.configData.lote);
			if (printerStatus == PRINTER_NO_COMM) {
				appLcdSend(FAILED_PRINTER_COM, NULL);
				vTaskDelay(3000 / portTICK_PERIOD_MS);
			}

			appLcdSend(WAITING_TEST, &deviceControl.configData);
			deviceControl.test_state = WAIT_TEST;
			break;
		case CANCEL_FSM:
			appLcdSend(CANCEL_LCD, NULL);
			disconnectPrimarySecondary();

			// Wait until the ADC conversion finishes,
			// if we are in the middle of an ADC conversion
			xSemaphoreTake(deviceControl.checkTafoInProgress_semphr, 1000 / portTICK_PERIOD_MS);

			if (appAdcStatus() == ENABLE) appAdcDisable();

			// Reconnect to WiFi
			deviceControl.wifi_state = app_WiFiConnect();
			if (deviceControl.wifi_state != WIFI_CONNECTED) {
				appLcdSend(FAILED_REPORT_LCD, NULL);
				vTaskDelay(3000 / portTICK_PERIOD_MS);
				appLcdSend(BLOCK_DEVICE_LCD, NULL);
				deviceControl.test_state = BLOCK_DEVICE;
				break;
			}
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			appLcdSend(WAITING_TEST, &deviceControl.configData);
			deviceControl.test_state = WAIT_TEST;
			break;
		default:

			vTaskDelay(3000 / portTICK_PERIOD_MS);
			break;
		}
		vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/*=====[Definitions of external functions]===================================*/

void appFsmInit( void )
{
	BaseType_t res = xTaskCreate(fsm_task, "fsm_task", 1024 * 4, NULL, 5, NULL);
	if (res != pdPASS)
	{
		// TODO: Define error policy
		while(1);
	}
}
