/*=============================================================================
 * Copyright (c) 2020, Martin N. Menendez <menendezmartin81@gmail.com>
 * All rights reserved.
 * License: Free
 * Date: 2020/09/03
 * Version: v1.1
 *===========================================================================*/

/*==================[inclusiones]============================================*/

#include "../inc/button.h"

/*==================[prototipos]============================================*/

void fsmButtonError( button_t* config );
void fsmButtonInit( button_t* config, gpio_num_t button );
void fsmButtonUpdate( button_t* config );
void buttonPressed( button_t* config );
void buttonReleased( button_t* config );

/*==================[funciones]============================================*/

bool isButtonPressed( button_t* config )
{
	return config->pressed;
}

void buttonPressed( button_t* config )
{
	config->pressed = 1;
}

void buttonReleased( button_t* config )
{
	config->pressed = 0;
}

void fsmButtonError( button_t* config )
{
	config->fsmButtonState = STATE_BUTTON_UP;
}

void fsmButtonInit( button_t* config, gpio_num_t button )
{
	config->contFalling = 0;
	config->contRising = 0;
	config->fsmButtonState = STATE_BUTTON_UP;  // Set initial state
	config->button = button;
	config->pressed = 0;
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
