/*
 * */

#ifndef _APP_LCD_H_
#define _APP_LCD_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include "../inc/app_adc.h"
#include "../inc/sapi_lcd.h"

#ifdef __cplusplus
extern "C" {
#endif

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

void appLcdInit(void);

void appLcdSend(lcd_msg_t lcd_msg);
void appLcdSendRms(rms_t *rms);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_LCD_H_ */
