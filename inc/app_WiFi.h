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
	WIFI_NO_INIT,      /*!<WiFi interface was not initialized                            */
	WIFI_CONNECTED,    /*!<WiFi interface connected                                      */
	WIFI_DISCONNECTED, /*!<WiFi interface initialized but not connected                  */
	WIFI_FAIL,         /*!<WiFi interface failed at initializing or connecting           */
} wifi_state_t;

/**
 * @brief Initialize the WiFi feature
 * 
 *  @param wifi_state
 *
 *  @return
 *   - ESP_OK on successful
 *   - ESP_FAIL on error
 */
esp_err_t app_WiFiInit(wifi_state_t *wifi_state);

/**
 * @brief Connect to WiFi
 * 
 *  @param wifi_state
 */
void app_WiFiConnect(wifi_state_t *wifi_state);

/**
 * @brief Disconnect from WiFi
 * 
 *  @param wifi_state
 */
void app_WiFiDisconnect(wifi_state_t *wifi_state);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_WIFI_H_ */
