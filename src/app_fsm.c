/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <stdio.h>

#include "../inc/app_adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "../inc/app_fsm.h"
#include "../inc/teclas.h"
#include "../inc/app_lcd.h"

/*=====[Definition macros of private constants]==============================*/

// GPIO
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<CPV | 1ULL<<CSV)
#define GPIO_INPUT_PIN_SEL   (1ULL<<pCan | 1ULL<<pConf | 1ULL<<pTest)

/*=====[Definitions of extern global variables]==============================*/

tTecla tecla_test, tecla_Can, tecla_Conf;

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

bool test_pushed;
bool config_pushed;
bool cancel_pushed;

/*=====[Definitions of internal functions]===================================*/

static void trafoPinInit( void )
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;

    gpio_set_level(CPV, 1);
	gpio_set_level(CSV, 1);

    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;

    gpio_config(&io_conf);
}

void connect_primary(void)
{
	gpio_set_level(CPV, 0);
	gpio_set_level(CSV, 1);
}

void connect_secondary(void) // connect secondary
{
	gpio_set_level(CPV, 1);
	gpio_set_level(CSV, 0);
}

void disconnect_primary_secondary(void) // disconnect primary secondary
{
	gpio_set_level(CPV, 1);
	gpio_set_level(CSV, 1);
}

void check_semaphores(void) {
	if (xSemaphoreTake(tecla_test.request, 0) == pdTRUE)
		test_pushed = 1;
	else
		test_pushed = 0;
	if (xSemaphoreTake(tecla_Conf.request, 0) == pdTRUE)
		config_pushed = 1;
	else
		config_pushed = 0;
	if (xSemaphoreTake(tecla_Can.request, 0) == pdTRUE)
		cancel_pushed = 1;
	else
		cancel_pushed = 0;
}

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

				appLcdSendRms(&rms);
			}

			xSemaphoreGive(checkTafoInProgress_semphr);
		}
		else if (deviceControl.test_state == MEASURE_SECONDARY)
		{
			for (int i=0; i<ADC_ITERATION; i++) {
				appAdcStart(&rms);

				appLcdSendRms(&rms);
			}

			xSemaphoreGive(checkTafoInProgress_semphr);
		}
	}
}

void tarea_tecla( void* taskParmPtr )
{

	fsmButtonInit( &tecla_Can, pCan );
	fsmButtonInit( &tecla_Conf, pConf );
	fsmButtonInit( &tecla_test, pTest );

	while( 1 )
	{
		fsmButtonUpdate( &tecla_Can );
		fsmButtonUpdate( &tecla_Conf );
		fsmButtonUpdate( &tecla_test );

		vTaskDelay( BUTTON_RATE );
	}
}

void fsm_task (void*arg)
{
	disconnect_primary_secondary();

	deviceControl.test_state = STARTUP;

	// Create semaphores
	checkTafo_semphr = xSemaphoreCreateBinary();
	checkTafoInProgress_semphr = xSemaphoreCreateBinary();

	test_pushed = 0;
	config_pushed = 0;
	cancel_pushed = 0;
	deviceControl.configurated = 0;
	deviceControl.printer_msg = PRINTER_FAIL;

	printer_queue = xQueueCreate(1, sizeof(printer_msg_t));

	xTaskCreate(checkTafo_task, "checkTafo_task", 1024 * 2, NULL, 5, NULL);

	// Wait until the LCD has initiate
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	while (1) {
		check_semaphores();

		switch(deviceControl.test_state) {
		case STARTUP:
			disconnect_primary_secondary();

			appLcdSend(WELCOME);

			vTaskDelay(3000 / portTICK_PERIOD_MS);

			appLcdSend(WAITING);
			deviceControl.test_state = WAIT_TEST;
			break;
		case WAIT_TEST:
			disconnect_primary_secondary();

			if (test_pushed){
				if (isConfigurated()) {
					deviceControl.test_state = POWER_UP_PRIMARY;
					appLcdSend(MEASURING_PRIMARY);
				}
				else {
					deviceControl.test_state = ASK_FOR_CONFIGURATION;
					appLcdSend(NOT_CONFIGURATED);
				}
			}
			if (config_pushed){
				if(configurate()){
					appLcdSend(CONFIGURATION_OK);
					vTaskDelay(3000 / portTICK_PERIOD_MS);
					appLcdSend(WAITING);
				}
				else {
					appLcdSend(CONFIGURATION_FAIL);
					vTaskDelay(3000 / portTICK_PERIOD_MS);
					appLcdSend(WAITING);
				}
			}
			vTaskDelay(500 / portTICK_PERIOD_MS);
			break;
		case ASK_FOR_CONFIGURATION:
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			appLcdSend(WAITING);
			deviceControl.test_state = WAIT_TEST;
			break;
		case POWER_UP_PRIMARY:
			disconnect_primary_secondary();
			vTaskDelay(200 / portTICK_PERIOD_MS);
			connect_primary();
			deviceControl.test_state = MEASURE_PRIMARY;
			xSemaphoreGive(checkTafo_semphr);
			break;
		case MEASURE_PRIMARY:
			// Wait until the primary caracterization is finished
			xSemaphoreTake(checkTafoInProgress_semphr, portMAX_DELAY);
			deviceControl.test_state = POWER_DOWN_PRIMARY;
			break;
		case POWER_DOWN_PRIMARY:
			disconnect_primary_secondary();
			vTaskDelay(200 / portTICK_PERIOD_MS);
			deviceControl.test_state = POWER_UP_SECONDARY;
			break;
		case POWER_UP_SECONDARY:
			disconnect_primary_secondary();
			vTaskDelay(200 / portTICK_PERIOD_MS);
			connect_secondary();
			deviceControl.test_state = MEASURE_SECONDARY;
			appLcdSend(MEASURING_SECONDARY);
			xSemaphoreGive(checkTafo_semphr);
			break;
		case MEASURE_SECONDARY:
			// Wait until the secondary caracterization is finished
			xSemaphoreTake(checkTafoInProgress_semphr, portMAX_DELAY);
			deviceControl.test_state = POWER_DOWN_SECONDARY;
			break;
		case POWER_DOWN_SECONDARY:
			disconnect_primary_secondary();
			vTaskDelay(200 / portTICK_PERIOD_MS);
			deviceControl.test_state = REPORT;
			break;
		case REPORT:
			appLcdSend(REPORT_LCD);
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			deviceControl.test_state = WAIT_TEST;
			appLcdSend(WAITING);
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
	trafoPinInit( );

	xTaskCreate(fsm_task, "fsm_task", 1024 * 2, NULL, 5, NULL);
	xTaskCreate(tarea_tecla, "tarea_tecla", 1024 * 2, NULL, 5, NULL);
}
