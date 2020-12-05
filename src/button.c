/*=============================================================================
 * Copyright (c) 2020, Martin N. Menendez <menendezmartin81@gmail.com>
 * All rights reserved.
 * License: Free
 * Date: 2020/09/03
 * Version: v1.1
 *===========================================================================*/

/*=====[Definition macros of private constants]==============================*/

#include "../inc/button.h"

/*=====[Definition of private macros, constants or data types]===============*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

// ESP-IDF FreeRTOS implements critical sections using special mutexes, referred by
// portMUX_Type objects on top of specific spinlock component
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/freertos-smp.html?highlight=portenter_critical#critical-sections
portMUX_TYPE buttonMux = portMUX_INITIALIZER_UNLOCKED;

/*=====[Definitions of internal functions]===================================*/

void buttonPressed( button_t* config )
{
	portENTER_CRITICAL(&buttonMux);
	config->pressed = 1;
	portEXIT_CRITICAL(&buttonMux);
}

void buttonReleased( button_t* config )
{
	portENTER_CRITICAL(&buttonMux);
	config->pressed = 0;
	portEXIT_CRITICAL(&buttonMux);
}

void fsmButtonError( button_t* config )
{
	config->fsmButtonState = STATE_BUTTON_UP;
}

/*=====[Definitions of external functions]===================================*/

void fsmButtonInit( button_t* config, gpio_num_t button )
{
	config->contFalling = 0;
	config->contRising = 0;
	config->fsmButtonState = STATE_BUTTON_UP;  // Set initial state
	config->button = button;
	config->pressed = 0;
}

bool isButtonPressed( button_t* config )
{
	bool pressed;
	portENTER_CRITICAL(&buttonMux);
	pressed = config->pressed;
	portEXIT_CRITICAL(&buttonMux);
	return pressed;
}

// FSM Update Sate Function
void fsmButtonUpdate( button_t* config )
{

    switch( config->fsmButtonState )
    {
        case STATE_BUTTON_UP:
            /* CHECK TRANSITION CONDITIONS */
            if( !gpio_get_level( config->button ) )
            {
            	config->fsmButtonState = STATE_BUTTON_FALLING;
            }
            break;

        case STATE_BUTTON_FALLING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */
            if( config->contFalling >= DEBOUNCE_TIME_MS )
            {
                if( !gpio_get_level( config->button ) )
                {
                	config->fsmButtonState = STATE_BUTTON_DOWN;

                    buttonPressed(config);
                }
                else
                {
                	config->fsmButtonState = STATE_BUTTON_UP;
                }

                config->contFalling = 0;
            }

            config->contFalling++;

            /* LEAVE */
            break;

        case STATE_BUTTON_DOWN:
			/* CHECK TRANSITION CONDITIONS */
			if( gpio_get_level( config->button ) )
			{
				config->fsmButtonState = STATE_BUTTON_RISING;
			}
			break;

        case STATE_BUTTON_RISING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */

            if( config->contRising >= DEBOUNCE_TIME_MS )
            {
                if( gpio_get_level( config->button ) )
                {
                	config->fsmButtonState = STATE_BUTTON_UP;

                    buttonReleased(config);
                }
                else
                {
                	config->fsmButtonState = STATE_BUTTON_DOWN;
                }
                config->contRising = 0;
            }
            config->contRising++;

            /* LEAVE */
            break;

        default:
            fsmButtonError(config);
            break;
    }
}
