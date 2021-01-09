/*=============================================================================
 * Author: ctrinidad
 * Date: 2021/01/07
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_smartconfig.h"
#include "../inc/app_WiFi.h"

/*=====[Definition of private macros, constants or data types]===============*/

#define STORAGE_NAMESPACE 	"wifi"
#define NAMESPACE_SSID	 	"wifi_ssid"
#define NAMESPACE_PASS	 	"wifi_pass"
#define READ_ERROR			(-1)

#define MAXIMUM_RETRY  5

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define ESPTOUCH_DONE_BIT  BIT2

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static int s_retry_num = 0;
char ip[100];

static uint8_t ssid[32]; // 32 == length of ssid in wifi_ap_config_t
static uint8_t password[65];
static bool bssid_set;
static uint8_t bssid[6];

static uint8_t get_ssid = 0;


/*=====[Definitions of internal functions]===================================*/

static void writeSSIDData(void)
{
	nvs_handle_t my_handle;
	esp_err_t err;

	// Write
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
	if (err == ESP_OK) {
		err = nvs_set_str(my_handle, NAMESPACE_SSID, (char *) ssid);
		ESP_ERROR_CHECK(err);
		// Commit written value.
		// After setting any values, nvs_commit() must be called to ensure changes are written
		// to flash storage. Implementations may write to storage at other times,
		// but this is not guaranteed.
		err = nvs_commit(my_handle);
		ESP_ERROR_CHECK(err);
		err = nvs_set_str(my_handle, NAMESPACE_PASS, (char *) password);
		ESP_ERROR_CHECK(err);
		err = nvs_commit(my_handle);
		ESP_ERROR_CHECK(err);
	}
	// Close
	nvs_close(my_handle);
}

static void smartconfig_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
    while (1) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | ESPTOUCH_DONE_BIT | WIFI_FAIL_BIT, true, false, portMAX_DELAY);
        if(uxBits & WIFI_CONNECTED_BIT) {
            // WiFi Connected to ap
    		// Store the ssid and password
    		writeSSIDData();
        }
        if((uxBits & ESPTOUCH_DONE_BIT) || (uxBits & WIFI_FAIL_BIT))  {
            // smartconfig over
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

static void WiFiConnectToKnownAP(void){
    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config_t));
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    // Initializing ssid
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
	memcpy(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
	wifi_config.sta.bssid_set = bssid_set;
	if (wifi_config.sta.bssid_set == true) {
		memcpy(wifi_config.sta.bssid, bssid, sizeof(wifi_config.sta.bssid));
	}

	ESP_ERROR_CHECK( esp_wifi_disconnect() );
	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK( esp_wifi_connect() );
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		if (get_ssid) {
			esp_wifi_connect();
		}
		else {
			xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
		}
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		if (s_retry_num < MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
		}
		else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		strcpy(ip, ip4addr_ntoa(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
	else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
		//ESP_LOGI(TAG, "Scan done");
	}
	else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
		//ESP_LOGI(TAG, "Found channel");
	}
	else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
		//ESP_LOGI(TAG, "Got SSID and password");

		smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
		memcpy(ssid, evt->ssid, sizeof(evt->ssid));
		memcpy(password, evt->password, sizeof(evt->password));
		bssid_set = evt->bssid_set;
		if (bssid_set == true) {
			memcpy(bssid, evt->bssid, sizeof(bssid));
		}

		// try to connect
		WiFiConnectToKnownAP();
		get_ssid = 1;
	}
	else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
		xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
	}
}

static int readSSIDData(void)
{
	nvs_handle_t my_handle;
	esp_err_t err;

	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
	ESP_ERROR_CHECK(err);
	if (err == ESP_OK) {
		// Read
		size_t size = sizeof(ssid);
		for(int i=0; i<size; i++) ssid[i]=0;
		err = nvs_get_str(my_handle, NAMESPACE_SSID, (char *) ssid, &size);
		if (err == ESP_OK) {
			size = sizeof(password);
			for(int i=0; i<size; i++) password[i]=0;
			err = nvs_get_str(my_handle, NAMESPACE_PASS, (char *) password, &size);
			if (err == ESP_OK) get_ssid = 1;
		}
		// Close
		nvs_close(my_handle);
	}
	return err;
}

/*=====[Definitions of external functions]===================================*/

void app_WiFiDisconnect(void)
{
	ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
	ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
	esp_wifi_disconnect();
	esp_wifi_stop();
    esp_wifi_deinit();
    vEventGroupDelete(s_wifi_event_group);
}

wifi_state_t app_WiFiConnect(void)
{
    s_wifi_event_group = xEventGroupCreate();
	s_retry_num = 0;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    if (get_ssid) {
		wifi_config_t wifi_config;
		bzero(&wifi_config, sizeof(wifi_config_t));
		wifi_config.sta.pmf_cfg.capable = true;
		wifi_config.sta.pmf_cfg.required = false;

		// Initializing ssid
		memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
		memcpy(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
		wifi_config.sta.bssid_set = bssid_set;
		if (wifi_config.sta.bssid_set == true) {
			memcpy(wifi_config.sta.bssid, bssid, sizeof(wifi_config.sta.bssid));
		}

		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
		ESP_ERROR_CHECK(esp_wifi_start() );
    }
    else {
        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK( esp_wifi_start() );
    }

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        return WIFI_CONNECTED;
    } else if (bits & WIFI_FAIL_BIT) {
    	app_WiFiDisconnect();
    	get_ssid = 0;
    	return WIFI_FAIL;
    } else {
    	app_WiFiDisconnect();
    	get_ssid = 0;
    	return WIFI_FAIL;
    }
}

int app_WiFiInit(void){
	esp_err_t err;

	err = readSSIDData();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    return err;
}
