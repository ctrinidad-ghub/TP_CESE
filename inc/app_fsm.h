/*
 * */

#ifndef _APP_FSM_H_
#define _APP_FSM_H_

#include <stdio.h>

#include "../inc/app_adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "../inc/app_lcd.h"
#include "../inc/app_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_ITERATION 5

extern QueueHandle_t printer_queue;

typedef enum {
	PRINTER_PASS,
	PRINTER_FAIL,
} printer_msg_t;

void appFsmInit( void );

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_FSM_H_ */
