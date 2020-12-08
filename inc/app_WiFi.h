/*
 * */

#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the WiFi feature
 * 
 */
void app_WiFiInit(void);

/**
 * @brief Connect to WiFi
 * 
 */
void app_WiFiConnect(void);

/**
 * @brief Disconnect from WiFi
 * 
 */
void app_WiFiDisconnect(void);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_WIFI_H_ */
