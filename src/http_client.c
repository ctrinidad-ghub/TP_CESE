/*=============================================================================
 * Author: ctrinidad
 * Date: 2021/01/13
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_event.h"
#include "tcpip_adapter.h"
#include "esp_err.h"
#include "esp_tls.h"
#include "esp_http_client.h"

#define GET_URL   "http://Cristian-PC/get.php"
#define POST_URL  "http://Cristian-PC/index.php"

static const char *TAG = "HTTP_CLIENT";

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                // printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}

esp_err_t get_http_config(char *buffer, uint32_t buffSize)
{
    esp_http_client_config_t config = {
    	.url = GET_URL,
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err;
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        return (err);
    }
    int content_length =  esp_http_client_fetch_headers(client);
    int total_read_len = 0, read_len;
    if (total_read_len < content_length && content_length <= buffSize) {
        read_len = esp_http_client_read(client, buffer, content_length);
        if (read_len <= 0) {
        	return (ESP_FAIL);
        }
        buffer[read_len] = 0;
        err = ESP_OK;
    } else err = ESP_FAIL;

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return (err);
}

esp_err_t post_http_results(char *buffer)
{
	esp_http_client_config_t config = {
		.url = POST_URL,
		.event_handler = _http_event_handler,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);

    // POST
    //const char *post_data = "id_Dispositivo=4&lote_partida=2033657-1&test_Numero=10&tension_linea=220&corriente_vacio=50";
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, buffer, strlen(buffer));
    esp_err_t err = esp_http_client_perform(client);

    esp_http_client_cleanup(client);
    return (err);
}
