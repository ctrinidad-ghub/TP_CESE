/*=============================================================================
 * Copyright (c) 2020, Martin N. Menendez <menendezmartin81@gmail.com>
 * All rights reserved.
 * License: Free
 * Date: 2020/09/03
 * Version: v1.1
 *===========================================================================*/
#ifndef _TECLAS_H_
#define _TECLAS_H_

/*==================[inclusiones]============================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "stdlib.h"
#include "driver/gpio.h"

/*==================[definiciones y macros]==================================*/
#define BUTTON_RATE_MS 10
#define BUTTON_RATE pdMS_TO_TICKS(BUTTON_RATE_MS)

#define DEBOUNCE_TIME_MS (40/BUTTON_RATE_MS)

/*==================[definiciones de datos]=========================*/
// Tipo de dato FSM
typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} fsmButtonState_t;

// Estructura principal
typedef struct
{
	gpio_num_t tecla;

	fsmButtonState_t fsmButtonState;

	uint8_t contFalling;
	uint8_t contRising;

	SemaphoreHandle_t request;
} tTecla;


/*==================[prototipos de funciones]====================*/
TickType_t get_diff();
void clear_diff();

void fsmButtonError( tTecla* config );
void fsmButtonInit( tTecla* config, gpio_num_t tecla );
void fsmButtonUpdate( tTecla* config );
void buttonPressed( tTecla* config );
void buttonReleased( tTecla* config );

#endif /* _TECLAS_H_ */
