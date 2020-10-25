/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <stdio.h>

#include "../inc/app_adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "../inc/app_fsm.h"

/*=====[Definition macros of private constants]==============================*/

// GPIO
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<CPV | 1ULL<<CSV)

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

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
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void fsm_task (void*arg)
{
	bool pin_status = 0;

	//gpio_set_level(CPV, 0);
	//gpio_set_level(CSV, 0);

	while (1) {
    	if (pin_status) {
			//gpio_set_level(CPV, 0);

			pin_status = 0;
		}
		else {
			//gpio_set_level(CPV, 1);

			pin_status = 1;
		}

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
/*=====[Definitions of external functions]===================================*/

void appFsmInit( void )
{
	trafoPinInit( );

	xTaskCreate(fsm_task, "fsm_task", 1024 * 2, NULL, 5, NULL);
}
