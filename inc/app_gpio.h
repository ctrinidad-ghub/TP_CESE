/*
 * */

#ifndef _APP_GPIO_H_
#define _APP_GPIO_H_

/*==================[inclusiones]============================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "stdlib.h"
#include "driver/gpio.h"
#include "../inc/button.h"

/*==================[definiciones y macros]==================================*/

#ifdef __cplusplus
extern "C" {
#endif

#define CPV  GPIO_NUM_26
#define CSV  GPIO_NUM_25

#define pCan 	GPIO_NUM_27
#define pConf 	GPIO_NUM_36
#define pTest 	GPIO_NUM_39

void appGpioInit( void );
void connectPrimary(void);
void connectSecondary(void);
void disconnectPrimarySecondary(void);
bool isTestPressed( void );
bool isCancelPressed( void );
bool isConfigPressed( void );

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_GPIO_H_ */
