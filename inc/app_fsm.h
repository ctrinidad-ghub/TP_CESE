/**
 * @file app_fsm.h
 * 
 * @brief 
 * 
 * @author Cristian Trinidad
 */

#ifndef _APP_FSM_H_
#define _APP_FSM_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

extern QueueHandle_t printer_queue;

typedef enum {
	PRINTER_PASS,
	PRINTER_FAIL,
} printer_msg_t;

/**
 * @brief Initialize main FSM 
 * 
 */
void appFsmInit( void );

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_FSM_H_ */
