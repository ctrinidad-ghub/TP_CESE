/*=============================================================================
 * Author: ctrinidad
 * Date: 2021/04/24
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "driver/gpio.h"
#include "../inc/app_gpio.h"
#include "../inc/app_error.h"

/*=====[Definition of private macros, constants or data types]===============*/

#define BUZZER_COUNT 50000

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Definitions of internal functions]===================================*/

/*=====[Definitions of external functions]===================================*/

void appFatalError(void)
{
	// Error policy: turn on buzzer forever
	uint32_t counter = 0;

	while(1) {
		gpio_set_level(buzzer, 1);
		for (counter = 0; counter<BUZZER_COUNT; counter++);
		gpio_set_level(buzzer, 0);
		for (counter = 0; counter<BUZZER_COUNT; counter++);
	}
}
