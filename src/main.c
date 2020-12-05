/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <app_printer.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "../inc/app_adc.h"
#include "../inc/app_lcd.h"
#include "../inc/app_fsm.h"
#include "../inc/app_gpio.h"
#include "../inc/main.h"

/*=====[Definition of private macros, constants or data types]===============*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

void app_main()
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	// LCD Initialization
	appLcdInit( );

	// ADC Initialization
	appAdcInit( );

	// Printer Initialization
	appPrinter();

	// GPIO Initialization
	appGpioInit( );

	// Main FSM Initialization
	appFsmInit( );
}
