/*
 * */

#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	WIFI_CONNECTED,
	WIFI_FAIL,
} wifi_state_t;

/**
 * @brief Initialize the WiFi feature
 * 
 */
int app_WiFiInit(void);

/**
 * @brief Connect to WiFi
 * 
 */
wifi_state_t app_WiFiConnect(void);

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
