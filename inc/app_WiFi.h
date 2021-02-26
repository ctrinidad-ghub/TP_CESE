/**
 * @file app_WiFi.h
 *
 * @brief
 *
 * @author Cristian Trinidad
 */

#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi status
 *
 */
typedef enum {
	WIFI_NO_INIT,
	WIFI_CONNECTED,
	WIFI_DISCONNECTED,
	WIFI_FAIL,
} wifi_state_t;

/**
 * @brief Initialize the WiFi feature
 * 
 */
int app_WiFiInit(wifi_state_t *wifi_state);

/**
 * @brief Connect to WiFi
 * 
 */
void app_WiFiConnect(wifi_state_t *wifi_state);

/**
 * @brief Disconnect from WiFi
 * 
 */
void app_WiFiDisconnect(wifi_state_t *wifi_state);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_WIFI_H_ */