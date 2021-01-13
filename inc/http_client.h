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
 * @brief Send a lcd_msg_id_t to the LCD
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


#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _HTTP_CLIENT_H_ */
