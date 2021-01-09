/**
 * @file app_lcd.h
 * 
 * @brief 
 * 
 * @author Cristian Trinidad
 */

#ifndef _APP_LCD_H_
#define _APP_LCD_H_

#include "../inc/app_adc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LCD message type for appLcdSend function
 * 
 */
typedef enum {
    WELCOME,                /*!<Welcome LCD message                             */
	WIFI_CONNECTING,        /*!<Welcome LCD message                             */
	WIFI_NO_SSID_AND_PASS,  /*!<Welcome LCD message                             */
	WIFI_SMARTCONFIG,       /*!<Welcome LCD message                             */
	WIFI_SMARTCONFIG_FAIL,
	WIFI_SUCCESSFULLY_CONNECTED,
	WAITING,                /*!<Welcome LCD message                             */
	NOT_CONFIGURATED,       /*!<Welcome LCD message                             */
	CONFIGURATION_OK,       /*!<Welcome LCD message                             */
	CONFIGURATION_FAIL,     /*!<Welcome LCD message                             */
	MEASURING_PRIMARY,      /*!<Welcome LCD message                             */
	MEASURING_SECONDARY,    /*!<Welcome LCD message                             */
	REPORT_LCD,             /*!<Welcome LCD message                             */
	CANCEL_LCD
} lcd_msg_id_t;

/**
 * @brief Initialize LCD
 * 
 */
void appLcdInit(void);

/**
 * @brief Send a lcd_msg_id_t to the LCD
 * 
 * @param lcd_msg_id Message to send
 * @param rms Pointer to an struct that constain the RMS values to show
 * 
 * @note The rms pointer is optional and depends on the lcd_msg_id to send
 * 
 */
void appLcdSend(lcd_msg_id_t lcd_msg_id, rms_t *rms);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_LCD_H_ */
