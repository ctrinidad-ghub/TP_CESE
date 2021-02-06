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
    WELCOME,                    /*!<Welcome LCD message                                */
	WIFI_CONNECTING,            /*!<WiFi connection in progress LCD message            */
	WIFI_NO_SSID_AND_PASS,      /*!<Failing at trying to connect into WiFi LCD message */
	USE_WIFI_CONFIG,            /*!<Ask for using WiFi configuration LCD message       */
	WIFI_SMARTCONFIG,           /*!<Ask for Smart Config WiFi app LCD message          */
	WIFI_SMARTCONFIG_FAIL,      /*!<Failing at trying to configure WiFi by Smart Config app LCD message  */
	WIFI_SUCCESSFULLY_CONNECTED,/*!<WiFi connection successfully LCD message           */
	WAITING,                    /*!<Waiting for Test or Config LCD message             */
	WAIT_CONFIG,                /*!<Waiting for Configuration LCD message              */
	CONFIGURATING_LCD,          /*!<Configuration device LCD message                   */
	NOT_CONFIGURED,             /*!<Device is not configured LCD message               */
	CONFIGURATION_OK,           /*!<Device configured successfully LCD message         */
	CONFIGURATION_FAIL,         /*!<Device configured unsuccessfully LCD message       */
	MEASURING_PRIMARY,          /*!<Measuring Primary Coil LCD message                 */
	MEASURING_SECONDARY,        /*!<Measuring Secondary Coil LCD message               */
	REPORT_LCD,                 /*!<Reporting LCD message                              */
	FAILED_REPORT_LCD,          /*!<Failed at reporting LCD message                    */
	CANCEL_LCD,                 /*!<Canceling LCD message                              */
	BLOCK_DEVICE_LCD,           /*!<No WiFi connection - Block device LCD message      */
	BLOCK_DEVICE_WEB_SERVER_LCD /*!<Web Server no available - Block device LCD message */
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
 * @param rms Pointer to an struct that contains the RMS values to show
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
