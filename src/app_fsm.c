/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "../inc/app_lcd.h"
#include "../inc/app_gpio.h"
#include "../inc/app_fsm.h"
#include "../inc/app_adc.h"

/*=====[Definition of private macros, constants or data types]===============*/

#define ADC_ITERATION 5

/*=====[Definitions of extern global variables]==============================*/

QueueHandle_t printer_queue;

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

SemaphoreHandle_t checkTafo_semphr;
SemaphoreHandle_t checkTafoInProgress_semphr;

typedef enum {
	STARTUP,
    WAIT_TEST, // chequear que haya datos de configuracion
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
} deviceControl_t;

deviceControl_t deviceControl;

/*=====[Definitions of internal functions]===================================*/

bool isConfigurated(void)
{
	return deviceControl.configurated;
}
bool configurate(void)
{
	deviceControl.configurated = 1;
	return 1;
}

void checkTafo_task (void*arg)
{
	rms_t rms;

	while(1) {
		xSemaphoreTake(checkTafo_semphr, portMAX_DELAY);

		if (deviceControl.test_state == MEASURE_PRIMARY)
		{
			for (int i=0; i<ADC_ITERATION; i++) {
				appAdcStart(&rms);

				appLcdSend(MEASURING_PRIMARY, &rms);
			}

			xSemaphoreGive(checkTafoInProgress_semphr);
		}
		else if (deviceControl.test_state == MEASURE_SECONDARY)
		{
			for (int i=0; i<ADC_ITERATION; i++) {
				appAdcStart(&rms);

				appLcdSend(MEASURING_SECONDARY, &rms);
			}

			xSemaphoreGive(checkTafoInProgress_semphr);
		}
	}
}

void fsm_task (void*arg)
{
	disconnectPrimarySecondary();

	deviceControl.test_state = STARTUP;

	// Create semaphores
	checkTafo_semphr = xSemaphoreCreateBinary();
	checkTafoInProgress_semphr = xSemaphoreCreateBinary();

	deviceControl.configurated = 0;
	deviceControl.printer_msg = PRINTER_FAIL;

	printer_queue = xQueueCreate(1, sizeof(printer_msg_t));

	xTaskCreate(checkTafo_task, "checkTafo_task", 1024 * 2, NULL, 5, NULL);

	// Wait until the LCD has initiate
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	while (1) {
		switch(deviceControl.test_state) {
		case STARTUP:
			disconnectPrimarySecondary();

			appLcdSend(WELCOME, NULL);

			vTaskDelay(3000 / portTICK_PERIOD_MS);

			appLcdSend(WAITING, NULL);
			deviceControl.test_state = WAIT_TEST;
			break;
		case WAIT_TEST:
			disconnectPrimarySecondary();

			if ( isTestPressed( ) ) {
				if ( isConfigurated() ) {
					deviceControl.test_state = POWER_UP_PRIMARY;
				}
				else {
					deviceControl.test_state = ASK_FOR_CONFIGURATION;
					appLcdSend(NOT_CONFIGURATED, NULL);
				}
			}
			if ( isConfigPressed( ) ) {
				if( configurate() ){
					appLcdSend(CONFIGURATION_OK, NULL);
					vTaskDelay(3000 / portTICK_PERIOD_MS);
					appLcdSend(WAITING, NULL);
				}
				else {
					appLcdSend(CONFIGURATION_FAIL, NULL);
					vTaskDelay(3000 / portTICK_PERIOD_MS);
					appLcdSend(WAITING, NULL);
				}
			}
			vTaskDelay(500 / portTICK_PERIOD_MS);
			break;
		case ASK_FOR_CONFIGURATION:
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			appLcdSend(WAITING, NULL);
			deviceControl.test_state = WAIT_TEST;
			break;
		case POWER_UP_PRIMARY:
			disconnectPrimarySecondary();
			vTaskDelay(200 / portTICK_PERIOD_MS);
			connectPrimary();
			deviceControl.test_state = MEASURE_PRIMARY;
			xSemaphoreGive(checkTafo_semphr);
			break;
		case MEASURE_PRIMARY:
			// Wait until the primary characterization is finished
			xSemaphoreTake(checkTafoInProgress_semphr, portMAX_DELAY);
			deviceControl.test_state = POWER_DOWN_PRIMARY;
			break;
		case POWER_DOWN_PRIMARY:
			disconnectPrimarySecondary();
			vTaskDelay(200 / portTICK_PERIOD_MS);
			deviceControl.test_state = POWER_UP_SECONDARY;
			break;
		case POWER_UP_SECONDARY:
			disconnectPrimarySecondary();
			vTaskDelay(200 / portTICK_PERIOD_MS);
			connectSecondary();
			deviceControl.test_state = MEASURE_SECONDARY;
			xSemaphoreGive(checkTafo_semphr);
			break;
		case MEASURE_SECONDARY:
			// Wait until the secondary characterization is finished
			xSemaphoreTake(checkTafoInProgress_semphr, portMAX_DELAY);
			deviceControl.test_state = POWER_DOWN_SECONDARY;
			break;
		case POWER_DOWN_SECONDARY:
			disconnectPrimarySecondary();
			vTaskDelay(200 / portTICK_PERIOD_MS);
			deviceControl.test_state = REPORT;
			break;
		case REPORT:
			appLcdSend(REPORT_LCD, NULL);
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			deviceControl.test_state = WAIT_TEST;
			appLcdSend(WAITING, NULL);
			xQueueSend( printer_queue, ( void * ) &deviceControl.printer_msg, ( TickType_t ) 0 );
			break;
		default:
			vTaskDelay(500 / portTICK_PERIOD_MS);
			break;
		}
    }

    vTaskDelete(NULL);
}

/*=====[Definitions of external functions]===================================*/

void appFsmInit( void )
{
	xTaskCreate(fsm_task, "fsm_task", 1024 * 2, NULL, 5, NULL);
}
