/*=============================================================================
 * Copyright (c) 2020, Martin N. Menendez <menendezmartin81@gmail.com>
 * All rights reserved.
 * License: Free
 * Date: 2020/09/03
 * Version: v1.1
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include "../inc/teclas.h"
#include "../inc/app_fsm.h"
/*==================[prototipos]============================================*/

void fsmButtonError( tTecla* config );
void fsmButtonInit( tTecla* config, gpio_num_t tecla );
void fsmButtonUpdate( tTecla* config );
void buttonPressed( tTecla* config );
void buttonReleased( tTecla* config );

/*==================[funciones]============================================*/

/* accion de el evento de tecla pulsada */
void buttonPressed( tTecla* config )
{
	xSemaphoreGive(config->request);
}

/* accion de el evento de tecla liberada */
void buttonReleased( tTecla* config )
{
	// Nada
}

void fsmButtonError( tTecla* config )
{
	config->fsmButtonState = STATE_BUTTON_UP;
}

void fsmButtonInit( tTecla* config, gpio_num_t tecla )
{
	config->contFalling = 0;
	config->contRising = 0;
	config->fsmButtonState = STATE_BUTTON_UP;  // Set initial state
	config->tecla = tecla;
	config->request = xSemaphoreCreateBinary();
}

// FSM Update Sate Function
void fsmButtonUpdate( tTecla* config )
{

    switch( config->fsmButtonState )
    {
        case STATE_BUTTON_UP:
            /* CHECK TRANSITION CONDITIONS */
            if( !gpio_get_level( config->tecla ) )
            {
            	config->fsmButtonState = STATE_BUTTON_FALLING;
            }
            break;

        case STATE_BUTTON_FALLING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */
            if( config->contFalling >= DEBOUNCE_TIME_MS )
            {
                if( !gpio_get_level( config->tecla ) )
                {
                	config->fsmButtonState = STATE_BUTTON_DOWN;

                    /* ACCION DEL EVENTO !*/
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
			if( gpio_get_level( config->tecla ) )
			{
				config->fsmButtonState = STATE_BUTTON_RISING;
			}
			break;

        case STATE_BUTTON_RISING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */

            if( config->contRising >= DEBOUNCE_TIME_MS )
            {
                if( gpio_get_level( config->tecla ) )
                {
                	config->fsmButtonState = STATE_BUTTON_UP;

                    /* ACCION DEL EVENTO ! */
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
