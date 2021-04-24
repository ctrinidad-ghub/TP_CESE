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
#include "../inc/test_status.h"

/*=====[Definition of private macros, constants or data types]===============*/

#define ADC_ITERATION 			5
#define MAX_HTTP_RECV_BUFFER 	2048
#define POST_RETRIES			2

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
} test_fsm_state_t;

typedef enum {
	FAST_TEST_MODE,
	DETAILED_TEST_MODE
} test_mode_t;

typedef struct {
	test_fsm_state_t test_fsm_state;
	bool configurated;
	uint8_t fsm_timer;
	configData_t configData;
	SemaphoreHandle_t checkTafo_semphr;
	SemaphoreHandle_t checkTafoInProgress_semphr;
	SemaphoreHandle_t test_fsm_state_mutex;
	wifi_state_t wifi_state;
	test_status_t test_status;
	test_mode_t test_mode;
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
	for(int i=0; i<BATCHID_LENGTH; i++) deviceControl.configData.batchId[i]=0;
	for(int i=0; i<CODE_LENGTH; i++) deviceControl.configData.code[i]=0;
	deviceControl.configData.trafoParameters.Vinp.max = 0;
	deviceControl.configData.trafoParameters.Vinp.min = 0;
	deviceControl.configData.trafoParameters.Voutp.max = 0;
	deviceControl.configData.trafoParameters.Voutp.min = 0;
	deviceControl.configData.trafoParameters.Ip.max = 0;
	deviceControl.configData.trafoParameters.Ip.min = 0;
	deviceControl.configData.trafoParameters.Vins.max = 0;
	deviceControl.configData.trafoParameters.Vins.min = 0;
	deviceControl.configData.trafoParameters.Vouts.max = 0;
	deviceControl.configData.trafoParameters.Vouts.min = 0;
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
	uint8_t adc_iteration;
	test_fsm_state_t test_fsm_state;

	while(1) {
		xSemaphoreTake(deviceControl.checkTafo_semphr, portMAX_DELAY);

		// In DETAILED_TEST_MODE the measured values should be shown
		// so it should iterate for a little while
		if (deviceControl.test_mode == DETAILED_TEST_MODE) adc_iteration = ADC_ITERATION;
		else adc_iteration = 1;

		for (int i=0; i<adc_iteration; i++) {
			appAdcStart(&rms);

			xSemaphoreTake(deviceControl.test_fsm_state_mutex, portMAX_DELAY);
			test_fsm_state = deviceControl.test_fsm_state;
			xSemaphoreGive(deviceControl.test_fsm_state_mutex);

			if (test_fsm_state == CANCEL_FSM) {
				break;
			}
			else if (test_fsm_state == MEASURE_PRIMARY) {
				deviceControl.test_status.Ip = rms.Ip;
				deviceControl.test_status.Vouts = rms.Vs;
				deviceControl.test_status.Vinp = rms.Vp;
				if (rms.Ip > deviceControl.configData.trafoParameters.Ip.max ||
					rms.Ip < deviceControl.configData.trafoParameters.Ip.min) {
					deviceControl.test_status.test_result |= IP_FAILED;
				}
				if (rms.Vs > deviceControl.configData.trafoParameters.Vouts.max ||
					rms.Vs < deviceControl.configData.trafoParameters.Vouts.min) {
					deviceControl.test_status.test_result |= VS_FAILED;
				}

				// In DETAILED_TEST_MODE the measured values should be shown
				if(deviceControl.test_mode == DETAILED_TEST_MODE) appLcdSend(MEASURING_PRIMARY, &rms);
			}
			else if (test_fsm_state == MEASURE_SECONDARY) {
				deviceControl.test_status.Is = rms.Is;
				deviceControl.test_status.Voutp = rms.Vp;
				deviceControl.test_status.Vins = rms.Vs;
				if (rms.Is > deviceControl.configData.trafoParameters.Is.max ||
					rms.Is < deviceControl.configData.trafoParameters.Is.min) {
					deviceControl.test_status.test_result |= IS_FAILED;
				}

				// In DETAILED_TEST_MODE the measured values should be shown
				if(deviceControl.test_mode == DETAILED_TEST_MODE) appLcdSend(MEASURING_SECONDARY, &rms);
			}
		}
		xSemaphoreGive(deviceControl.checkTafoInProgress_semphr);
	}
}

void fsm_task (void*arg)
{
	esp_err_t err;
	uint8_t countWiFiAttempt = 0;
	TickType_t xLastWakeTime;

	disconnectPrimarySecondary();

	deviceControl.test_fsm_state = STARTUP;
	deviceControl.wifi_state = WIFI_NO_INIT;

	// Create semaphores and mutex
	deviceControl.checkTafo_semphr = xSemaphoreCreateBinary();
	deviceControl.checkTafoInProgress_semphr = xSemaphoreCreateBinary();
	deviceControl.test_fsm_state_mutex = xSemaphoreCreateMutex();
	if( deviceControl.checkTafo_semphr == NULL || deviceControl.checkTafoInProgress_semphr == NULL ||
		deviceControl.test_fsm_state_mutex == NULL )
	{
		// TODO: Define error policy
		while(1);
	}
	deviceControl.configurated = 0;
	deviceControl.test_status.test_result = TEST_PASS;
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
		switch(deviceControl.test_fsm_state) {
		case STARTUP:
			disconnectPrimarySecondary();

			if ( isTestPressed( ) ) {
				deviceControl.test_mode = DETAILED_TEST_MODE;
			}
			else{
				deviceControl.test_mode = FAST_TEST_MODE;
			}

			appLcdSend(WELCOME, NULL);

			vTaskDelay(LCD_MSG_WAIT);
			appAdcDisable();
			deviceControl.test_fsm_state = WIFI_CONNECTION;
			break;
		case WIFI_CONNECTION:
			appLcdSend(WIFI_CONNECTING, NULL);
			vTaskDelay(LCD_MSG_WAIT);
			err = app_WiFiInit(&deviceControl.wifi_state);
			if (err != ESP_OK) {
				appLcdSend(WIFI_NO_SSID_AND_PASS, NULL);
				vTaskDelay(LCD_MSG_WAIT);
				deviceControl.test_fsm_state = WIFI_CONFIG;
				break;
			}
			else {
				app_WiFiConnect(&deviceControl.wifi_state);
				if (deviceControl.wifi_state != WIFI_CONNECTED) {
					appLcdSend(WIFI_NO_SSID_AND_PASS, NULL);
					vTaskDelay(LCD_MSG_WAIT);
					deviceControl.test_fsm_state = WIFI_CONFIG;
					break;
				}
			}

			// Only print this information in detailed mode
			if (deviceControl.test_mode == DETAILED_TEST_MODE) {
				appLcdSend(WIFI_SUCCESSFULLY_CONNECTED, NULL);
				vTaskDelay(LCD_MSG_WAIT);
			}

			appLcdSend(WAIT_CONFIG, NULL);
			deviceControl.test_fsm_state = WAIT_TEST;
			break;

		case WIFI_CONFIG:
			appLcdSend(USE_WIFI_CONFIG, NULL);
			if ( isConfigPressed( ) ) {
				countWiFiAttempt = 0;
				while (deviceControl.wifi_state != WIFI_CONNECTED) {
					appLcdSend(WIFI_SMARTCONFIG, NULL);
					app_WiFiConnect(&deviceControl.wifi_state);
					if (deviceControl.wifi_state != WIFI_CONNECTED) {
						appLcdSend(WIFI_SMARTCONFIG_FAIL, NULL);
						vTaskDelay(LCD_MSG_WAIT);
						countWiFiAttempt++;
						if (countWiFiAttempt == 3) {
							appLcdSend(BLOCK_DEVICE_LCD, NULL);
							deviceControl.test_fsm_state = BLOCK_DEVICE;
							break;
						}
					}
				}
				if (deviceControl.wifi_state == WIFI_CONNECTED) {
					appLcdSend(WIFI_SUCCESSFULLY_CONNECTED, NULL);
					vTaskDelay(LCD_MSG_WAIT);

					appLcdSend(WAIT_CONFIG, NULL);
					deviceControl.test_fsm_state = WAIT_TEST;
				}
			}
			if ( isCancelPressed( ) ) {
				appLcdSend(BLOCK_DEVICE_LCD, NULL);
				deviceControl.test_fsm_state = BLOCK_DEVICE;
			}
			break;
		case WAIT_TEST:
			disconnectPrimarySecondary();

			if ( isTestPressed( ) ) {
				if ( isConfigurated() ) {
					if ( isSafetySwitchOpen( ) ) {
					    appLcdSend(SAFETY_SWITCH_OPEN, NULL);
					    vTaskDelay(LCD_MSG_WAIT);
					    appLcdSend(WAITING_TEST, &deviceControl.configData);
					}
					else {
						deviceControl.test_fsm_state = POWER_UP_PRIMARY;
						app_WiFiDisconnect(&deviceControl.wifi_state);
						vTaskDelay(500 / portTICK_PERIOD_MS);
						appAdcEnable();
						connectPrimary();
						deviceControl.fsm_timer = 0;
						deviceControl.test_status.test_result = TEST_PASS;
					}
				}
				else {
					deviceControl.test_fsm_state = ASK_FOR_CONFIGURATION;
					appLcdSend(NOT_CONFIGURED, NULL);
				}
			}
			if ( isConfigPressed( ) ) {
				appLcdSend(CONFIGURATING_LCD, NULL);
				deviceControl.test_fsm_state = CONFIGURATING;
			}
			break;
		case CONFIGURATING:
			// Initialize the xLastWakeTime variable with the current time.
			xLastWakeTime = xTaskGetTickCount ();

			if( configurate() ){
				// Use vTaskDelayUntil to avoid waiting too much time
				vTaskDelayUntil(&xLastWakeTime, LCD_MSG_WAIT);

			    appLcdSend(WAITING_TEST, &deviceControl.configData);
			}
			else {
				appLcdSend(CONFIGURATION_FAIL, NULL);
			    vTaskDelay(LCD_MSG_WAIT);
			    appLcdSend(WAIT_CONFIG, NULL);
			} 
			deviceControl.test_fsm_state = WAIT_TEST;
			break;
		case ASK_FOR_CONFIGURATION:
			vTaskDelay(LCD_MSG_WAIT);
			appLcdSend(WAIT_CONFIG, NULL);
			deviceControl.test_fsm_state = WAIT_TEST;
			break;
		case POWER_UP_PRIMARY:
			if ( isCancelPressed( ) || isSafetySwitchOpen( ) ) {
				xSemaphoreTake(deviceControl.test_fsm_state_mutex, portMAX_DELAY);
				deviceControl.test_fsm_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_fsm_state_mutex);
			}
			else if (deviceControl.fsm_timer == 4) {
				deviceControl.test_fsm_state = MEASURE_PRIMARY;
				xSemaphoreGive(deviceControl.checkTafo_semphr);
			} else deviceControl.fsm_timer++;
			break;
		case MEASURE_PRIMARY:
			if ( isCancelPressed( ) || isSafetySwitchOpen( ) ) {
				xSemaphoreTake(deviceControl.test_fsm_state_mutex, portMAX_DELAY);
				deviceControl.test_fsm_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_fsm_state_mutex);
			}
			// Wait until the primary characterization is finished
			else if (uxSemaphoreGetCount(deviceControl.checkTafoInProgress_semphr) == pdTRUE) {
				xSemaphoreTake(deviceControl.checkTafoInProgress_semphr, portMAX_DELAY);
				disconnectPrimarySecondary();
				deviceControl.fsm_timer = 0;
				deviceControl.test_fsm_state = POWER_DOWN_PRIMARY;
			}
			break;
		case POWER_DOWN_PRIMARY:
			if ( isCancelPressed( ) || isSafetySwitchOpen( ) ) {
				xSemaphoreTake(deviceControl.test_fsm_state_mutex, portMAX_DELAY);
				deviceControl.test_fsm_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_fsm_state_mutex);
			}
			else if (deviceControl.fsm_timer == 4) {
				deviceControl.fsm_timer = 0;
				connectSecondary();
				deviceControl.test_fsm_state = POWER_UP_SECONDARY;
			} else deviceControl.fsm_timer++;
			break;
		case POWER_UP_SECONDARY:
			if ( isCancelPressed( ) || isSafetySwitchOpen( ) ) {
				xSemaphoreTake(deviceControl.test_fsm_state_mutex, portMAX_DELAY);
				deviceControl.test_fsm_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_fsm_state_mutex);
			}
			else if (deviceControl.fsm_timer == 4) {
				deviceControl.test_fsm_state = MEASURE_SECONDARY;
				xSemaphoreGive(deviceControl.checkTafo_semphr);
			} else deviceControl.fsm_timer++;
			break;
		case MEASURE_SECONDARY:
			if ( isCancelPressed( ) || isSafetySwitchOpen( ) ) {
				xSemaphoreTake(deviceControl.test_fsm_state_mutex, portMAX_DELAY);
				deviceControl.test_fsm_state = CANCEL_FSM;
				xSemaphoreGive(deviceControl.test_fsm_state_mutex);
			}
			// Wait until the secondary characterization is finished
			else if (uxSemaphoreGetCount(deviceControl.checkTafoInProgress_semphr) == pdTRUE) {
				xSemaphoreTake(deviceControl.checkTafoInProgress_semphr, portMAX_DELAY);
				disconnectPrimarySecondary();
				deviceControl.fsm_timer = 0;
				deviceControl.test_fsm_state = POWER_DOWN_SECONDARY;
			}
			break;
		case POWER_DOWN_SECONDARY:
			if (deviceControl.fsm_timer == 4) {
				deviceControl.test_fsm_state = REPORT;
			} else deviceControl.fsm_timer++;
			disconnectPrimarySecondary();
			break;
		case REPORT:
			appLcdSend(REPORT_LCD, &deviceControl.test_status);

			triggerBuzzer(deviceControl.test_status.test_result);

			appAdcDisable();
			vTaskDelay(300 / portTICK_PERIOD_MS);

			// Reconnect to WiFi
			app_WiFiConnect(&deviceControl.wifi_state);
			if (deviceControl.wifi_state != WIFI_CONNECTED) {
				appLcdSend(FAILED_REPORT_LCD, NULL);
				vTaskDelay(LCD_MSG_WAIT);
				appLcdSend(BLOCK_DEVICE_LCD, NULL);
				deviceControl.test_fsm_state = BLOCK_DEVICE;
				break;
			}

			// Write test results to the web server
			processTxData(buffHttp, &deviceControl.configData, &deviceControl.test_status);

			uint8_t postRetries = 0;
			do {
				err = post_http_results(buffHttp, MAX_HTTP_RECV_BUFFER);
				postRetries++;
			} while (err!= ESP_OK && postRetries < POST_RETRIES);

			if (err != ESP_OK) {
				appLcdSend(FAILED_REPORT_LCD, NULL);
				vTaskDelay(LCD_MSG_WAIT);
				appLcdSend(BLOCK_DEVICE_WEB_SERVER_LCD, NULL);
				deviceControl.test_fsm_state = BLOCK_DEVICE;
				break;
			}

			// Send data to the printer
			printerStatus_t printerStatus = print(&deviceControl.test_status, deviceControl.configData.batchId);
			if (printerStatus == PRINTER_NO_COMM) {
				appLcdSend(FAILED_PRINTER_COM, NULL);
				vTaskDelay(LCD_MSG_WAIT);
			}

			appLcdSend(WAITING_TEST, &deviceControl.configData);
			deviceControl.test_fsm_state = WAIT_TEST;
			break;
		case CANCEL_FSM:
			disconnectPrimarySecondary();

			// Initialize the xLastWakeTime variable with the current time.
			xLastWakeTime = xTaskGetTickCount ();

			if ( isSafetySwitchOpen( ) )
				appLcdSend(SAFETY_SWITCH_OPEN, NULL);
			else
				appLcdSend(CANCEL_LCD, NULL);

			// Wait until the ADC conversion finishes,
			// if we are in the middle of an ADC conversion
			xSemaphoreTake(deviceControl.checkTafoInProgress_semphr, 1800 / portTICK_PERIOD_MS);

			if (appAdcStatus() == ENABLE) appAdcDisable();

			// Reconnect to WiFi
			app_WiFiConnect(&deviceControl.wifi_state);
			if (deviceControl.wifi_state != WIFI_CONNECTED) {
				appLcdSend(BLOCK_DEVICE_LCD, NULL);
				deviceControl.test_fsm_state = BLOCK_DEVICE;
				break;
			}

			// Use vTaskDelayUntil to avoid waiting too much time
			vTaskDelayUntil(&xLastWakeTime, LCD_MSG_WAIT);
			appLcdSend(WAITING_TEST, &deviceControl.configData);
			deviceControl.test_fsm_state = WAIT_TEST;
			break;
		default:

			vTaskDelay(LCD_MSG_WAIT);
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
