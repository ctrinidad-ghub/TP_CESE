/*
 * */

#ifndef _APP_FSM_H_
#define _APP_FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CPV  GPIO_NUM_26
#define CSV  GPIO_NUM_25

#define pCan 	GPIO_NUM_27
#define pConf 	GPIO_NUM_36
#define pTest 	GPIO_NUM_39

#define ADC_ITERATION 5

#include "freertos/semphr.h"
#include "freertos/queue.h"

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
