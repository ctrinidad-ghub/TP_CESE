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

/*=====[Definition macros of private constants]==============================*/

// GPIO
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<CPV | 1ULL<<CSV)

/*=====[Definitions of extern global variables]==============================*/

SemaphoreHandle_t test_request;
SemaphoreHandle_t config_request;
SemaphoreHandle_t cancel_request;

QueueHandle_t lcd_queue;
QueueHandle_t checkTafo_queue;

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
	lcd_msg_t lcd_msg;
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
	if (xSemaphoreTake(test_request, 0) == pdTRUE)
		test_pushed = 1;
	else
		test_pushed = 0;
	if (xSemaphoreTake(config_request, 0) == pdTRUE)
		config_pushed = 1;
	else
		config_pushed = 0;
	if (xSemaphoreTake(cancel_request, 0) == pdTRUE)
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
	while(1) {
		xSemaphoreTake(checkTafo_semphr, portMAX_DELAY);

		if (deviceControl.test_state == MEASURE_PRIMARY)
		{
			// Hacer algo
			vTaskDelay(5000 / portTICK_PERIOD_MS);
			xSemaphoreGive(checkTafoInProgress_semphr);
		}
		else if (deviceControl.test_state == MEASURE_SECONDARY)
		{
			// Hacer algo
			vTaskDelay(5000 / portTICK_PERIOD_MS);
			xSemaphoreGive(checkTafoInProgress_semphr);
		}
	}
}

void fsm_task (void*arg)
{
	disconnect_primary_secondary();

	deviceControl.test_state = STARTUP;

	// Create semaphores
	test_request = xSemaphoreCreateBinary();
	config_request = xSemaphoreCreateBinary();
	cancel_request = xSemaphoreCreateBinary();
	checkTafo_semphr = xSemaphoreCreateBinary();
	checkTafoInProgress_semphr = xSemaphoreCreateBinary();

	test_pushed = 0;
	config_pushed = 0;
	cancel_pushed = 0;
	deviceControl.configurated = 0;
	deviceControl.lcd_msg = WELCOME;

	lcd_queue = xQueueCreate(1, sizeof(lcd_msg_t));
	checkTafo_queue = xQueueCreate(1, sizeof(lcd_msg_t));

	xTaskCreate(checkTafo_task, "checkTafo_task", 1024 * 2, NULL, 5, NULL);

	// Wait until the LCD has initiate
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	while (1) {
		check_semaphores();

		switch(deviceControl.test_state) {
		case STARTUP:
			disconnect_primary_secondary();

			deviceControl.lcd_msg = WELCOME;
			xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			deviceControl.lcd_msg = WAITING;
			deviceControl.test_state = WAIT_TEST;
			xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
			break;
		case WAIT_TEST:
			disconnect_primary_secondary();

			if (test_pushed){
				if (isConfigurated()) {
					deviceControl.test_state = POWER_UP_PRIMARY;
					deviceControl.lcd_msg = MEASURING_PRIMARY;
					xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
				}
				else {
					deviceControl.test_state = ASK_FOR_CONFIGURATION;
					deviceControl.lcd_msg = NOT_CONFIGURATED;
					xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
				}
			}
			if (config_pushed){
				if(configurate()){
					deviceControl.lcd_msg = CONFIGURATION_OK;
					xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
					vTaskDelay(3000 / portTICK_PERIOD_MS);
					deviceControl.lcd_msg = WAITING;
					xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );

				}
				else {
					deviceControl.lcd_msg = CONFIGURATION_FAIL;
					xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
					vTaskDelay(3000 / portTICK_PERIOD_MS);
					deviceControl.lcd_msg = WAITING;
					xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
				}
			}
			vTaskDelay(500 / portTICK_PERIOD_MS);
			break;
		case ASK_FOR_CONFIGURATION:
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			deviceControl.lcd_msg = WAITING;
			deviceControl.test_state = WAIT_TEST;
			xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
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
			deviceControl.lcd_msg = MEASURING_SECONDARY;
			xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
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
			deviceControl.lcd_msg = REPORT_LCD;
			xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			deviceControl.lcd_msg = WAITING;
			deviceControl.test_state = WAIT_TEST;
			xQueueSend( lcd_queue, ( void * ) &deviceControl.lcd_msg, ( TickType_t ) 0 );
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
}
