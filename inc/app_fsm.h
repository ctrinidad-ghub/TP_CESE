/*
 * */

#ifndef _APP_FSM_H_
#define _APP_FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CPV  GPIO_NUM_26
#define CSV  GPIO_NUM_25

#define pCan GPIO_NUM_27

#include "freertos/semphr.h"
#include "freertos/queue.h"

extern SemaphoreHandle_t config_request;
extern SemaphoreHandle_t cancel_request;
extern QueueHandle_t lcd_queue;

typedef enum {
    WELCOME,
	WAITING,
	NOT_CONFIGURATED,
	CONFIGURATION_OK,
	CONFIGURATION_FAIL,
	MEASURING_PRIMARY,
	MEASURING_SECONDARY,
	REPORT_LCD,
} lcd_msg_t;

void appFsmInit( void );

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_FSM_H_ */
