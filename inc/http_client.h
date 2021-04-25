/**
 * @file http_client.h
 *
 * @brief
 *
 * @author Cristian Trinidad
 */

#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Send a GET command requesting the configuration data
 *        to the web server
 *
 * @param buffer Pointer to a receive buffer
 * @param buffSize Buffer size
 *
 * @return
 *  - ESP_OK on successful
 *  - ESP_FAIL on error
 *
 */
esp_err_t get_http_config(char *buffer, uint32_t buffSize);

/**
 * @brief Send a POST command with the test results to
 *        the web server
 *
 * @param buffer Pointer to a receive buffer
 * @param totalBuffSize Total buffer size available in bytes
 *
 * @return
 *  - ESP_OK on successful
 *  - ESP_FAIL on error
 *
 */
esp_err_t post_http_results(char *buffer, uint32_t totalBuffSize);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _HTTP_CLIENT_H_ */
