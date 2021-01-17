/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "stdlib.h"
#include "driver/gpio.h"
#include "../inc/button.h"
#include "../inc/app_gpio.h"

/*=====[Definition of private macros, constants or data types]===============*/

// GPIO
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<CPV | 1ULL<<CSV)
#define GPIO_INPUT_PIN_SEL   (1ULL<<pCan | 1ULL<<pConf | 1ULL<<pTest)

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static button_t testButton, cancelButton, configButton;

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

void button_task( void* taskParmPtr )
{

	fsmButtonInit( &cancelButton, pCan );
	fsmButtonInit( &configButton, pConf );
	fsmButtonInit( &testButton, pTest );

	while( 1 )
	{
		fsmButtonUpdate( &cancelButton );
		fsmButtonUpdate( &configButton );
		fsmButtonUpdate( &testButton );

		vTaskDelay( BUTTON_RATE );
	}
}

/*=====[Definitions of external functions]===================================*/

void connectPrimary(void)
{
	gpio_set_level(CPV, 0);
	gpio_set_level(CSV, 1);
}

void connectSecondary(void) // connect secondary
{
	gpio_set_level(CPV, 1);
	gpio_set_level(CSV, 0);
}

void disconnectPrimarySecondary(void) // disconnect primary secondary
{
	gpio_set_level(CPV, 1);
	gpio_set_level(CSV, 1);
}

bool isTestPressed( void ){
	return isButtonPressed( &testButton );
}

bool isCancelPressed( void ){
	return isButtonPressed( &cancelButton );
}

bool isConfigPressed( void ){
	return isButtonPressed( &configButton );
}

void appGpioInit( void )
{
	trafoPinInit( );

	BaseType_t res = xTaskCreate(button_task, "button_task", 1024 * 2, NULL, 5, NULL);
	if (res != pdPASS)
	{
		// TODO: Define error policy
		while(1);
	}
}
