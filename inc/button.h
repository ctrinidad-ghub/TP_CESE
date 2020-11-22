/*=============================================================================
 * Copyright (c) 2020, Martin N. Menendez <menendezmartin81@gmail.com>
 * All rights reserved.
 * License: Free
 * Date: 2020/09/03
 * Version: v1.1
 *===========================================================================*/
#ifndef _BUTTON_H_
#define _BUTTON_H_

/*==================[inclusiones]============================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdlib.h"
#include "driver/gpio.h"

/*==================[definiciones y macros]==================================*/
#define BUTTON_RATE_MS 10
#define BUTTON_RATE pdMS_TO_TICKS(BUTTON_RATE_MS)

#define DEBOUNCE_TIME_MS (40/BUTTON_RATE_MS)

/*==================[definiciones de datos]=========================*/

typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} fsmButtonState_t;


typedef struct
{
	gpio_num_t button;

	fsmButtonState_t fsmButtonState;

	uint8_t contFalling;
	uint8_t contRising;

	bool pressed;
} button_t;


/*==================[prototipos de funciones]====================*/

void fsmButtonInit( button_t* config, gpio_num_t button );
void fsmButtonUpdate( button_t* config );
bool isButtonPressed( button_t* config );

#endif /* _BUTTON_H_ */
