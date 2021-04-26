/*=============================================================================
 * Author: ctrinidad
 * Date: 2021/01/07
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "../inc/app_Comm.h"

/*=====[Definition of private macros, constants or data types]===============*/

#define ID			"id\":"
#define BATCHID 	"batchId\":\""
#define CODE 		"code\":\""
#define VINP_MAX	"vinPrimaryMax\":"
#define VINP_MIN	"vinPrimaryMin\":"
#define IP_MAX		"iPrimaryMax\":"
#define IP_MIN		"iPrimaryMin\":"
#define VOUTS_MAX	"voutSecondaryMax\":"
#define VOUTS_MIN	"voutSecondaryMin\":"
#define VINS_MAX	"vinSecondaryMax\":"
#define VINS_MIN	"vinSecondaryMin\":"
#define VOUTP_MAX	"voutPrimaryMax\":"
#define VOUTP_MIN	"voutPrimaryMin\":"
#define IS_MAX		"iSecondaryMax\":"
#define IS_MIN		"iSecondaryMin\":"

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Definitions of internal functions]===================================*/

void parseRxString(char *rx, char *data, uint8_t data_lenght, char *eod, char *str)
{
	char *ptr, *ptr2;

	ptr = strstr(rx, data);

	if (ptr != NULL) {
		// Substring found
		ptr += data_lenght-1;
		ptr2 = strstr(ptr, eod); // eod: end of data character
		if (ptr2 != NULL) {
			// Substring 2 found
			strncpy(str, ptr, ptr2-ptr);
		}
	}
}

int32_t parseRxInteger(char *rx, char *data, uint8_t data_lenght, char *eod)
{
	char str[20];
	int32_t idata;

	for (int i=0;i<sizeof(str);i++) str[i]=0;

	parseRxString(rx, data, data_lenght, eod, str);

	// Validating received data from webserver
	for (int i=0;i<strlen(str);i++) {
		if (str[i]<'0' || str[i]>'9') str[0]=0;
	}

	if (str[0] != 0) /* Substring found */
	{
		idata=atoi(str);
	}
	else /* Substring not found */
	{
		idata=-1;
	}
	return idata;
}

/*=====[Definitions of external functions]===================================*/

esp_err_t processRxData(char* rx_buff, configData_t *configData)
{
	if ((configData->id=parseRxInteger(rx_buff, ID, sizeof(ID), ",")) == -1) return ESP_FAIL;
	parseRxString(rx_buff, BATCHID, sizeof(BATCHID), "\",",configData->batchId);
	parseRxString(rx_buff, CODE, sizeof(CODE), "\",",configData->code);
	if ((configData->trafoParameters.Vinp.max=parseRxInteger(rx_buff, VINP_MAX, sizeof(VINP_MAX), ".00,")) == -1) return ESP_FAIL;
	if ((configData->trafoParameters.Vinp.min=parseRxInteger(rx_buff, VINP_MIN, sizeof(VINP_MIN), ".00,")) == -1) return ESP_FAIL;
	if ((configData->trafoParameters.Voutp.max=parseRxInteger(rx_buff, VOUTP_MAX, sizeof(VOUTP_MAX), ".00,")) == -1) return ESP_FAIL;
	if ((configData->trafoParameters.Voutp.min=parseRxInteger(rx_buff, VOUTP_MIN, sizeof(VOUTP_MIN), ".00,")) == -1) return ESP_FAIL;
	if ((configData->trafoParameters.Ip.max=parseRxInteger(rx_buff, IP_MAX, sizeof(IP_MAX), ".00,")) == -1) return ESP_FAIL;
	if ((configData->trafoParameters.Ip.min=parseRxInteger(rx_buff, IP_MIN, sizeof(IP_MIN), ".00,")) == -1) return ESP_FAIL;
	if ((configData->trafoParameters.Vins.max=parseRxInteger(rx_buff, VINS_MAX, sizeof(VINS_MAX), ".00,")*100) == -100) return ESP_FAIL;
	if ((configData->trafoParameters.Vins.min=parseRxInteger(rx_buff, VINS_MIN, sizeof(VINS_MIN), ".00,")*100) == -100) return ESP_FAIL;
	if ((configData->trafoParameters.Vouts.max=parseRxInteger(rx_buff, VOUTS_MAX, sizeof(VOUTS_MAX), ".00,")*100) == -100) return ESP_FAIL;
	if ((configData->trafoParameters.Vouts.min=parseRxInteger(rx_buff, VOUTS_MIN, sizeof(VOUTS_MIN), ".00,")*100) == -100) return ESP_FAIL;
	if ((configData->trafoParameters.Is.min=parseRxInteger(rx_buff, IS_MIN, sizeof(IS_MIN), ".00,")) == -1) return ESP_FAIL;
	if ((configData->trafoParameters.Is.max=parseRxInteger(rx_buff, IS_MAX, sizeof(IS_MAX), ".00}")) == -1) return ESP_FAIL;

	return ESP_OK;
}

void processTxData(char* tx_buff, configData_t *configData, test_status_t *test_status)
{
	char aux[100];

	strcpy(tx_buff, "{\"Code\":\"");
	strcat(tx_buff, configData->code);
	strcat(tx_buff, "\",\"BatchId\":\"");
	strcat(tx_buff, configData->batchId);
	strcat(tx_buff, "\",\"VinPrimary\":\"");
	sprintf(aux, "%d", test_status->Vinp);
	strcat(tx_buff, aux);
	strcat(tx_buff, ".0\",\"VoutSecondary\":\"");
	// Send one decimal digit in Vs
	sprintf(aux, "%d.%d", test_status->Vouts/100, test_status->Vouts/10 - (test_status->Vouts/100)*10);
	strcat(tx_buff, aux);
	strcat(tx_buff, "\",\"IPrimary\":\"");
	sprintf(aux, "%d", test_status->Ip);
	strcat(tx_buff, aux);
	strcat(tx_buff, ".0\",\"VinSecondary\":\"");
	// Send one decimal digit in Vs
	sprintf(aux, "%d.%d", test_status->Vins/100, test_status->Vins/10 - (test_status->Vins/100)*10);
	strcat(tx_buff, aux);
	strcat(tx_buff, "\",\"VoutPrimary\":\"");
	sprintf(aux, "%d", test_status->Voutp);
	strcat(tx_buff, aux);
	strcat(tx_buff, ".0\",\"ISecondary\":\"");
	sprintf(aux, "%d", test_status->Is);
	strcat(tx_buff, aux);
	if (test_status->test_result == TEST_PASS) {
		strcat(tx_buff, ".0\",\"TestResult\":\"OK\",}");
	}
	else {
		strcat(tx_buff, ".0\",\"TestResult\":\"RECHAZADO\",}");
	}
}

esp_err_t checkPostData(char *rx_buff)
{
	// Dummy POST body data check
	// Future improvement: verify that the whole parameters were
	// properly written based on the sent data
	char *ptr;

	ptr = strstr(rx_buff, ID);
	if (ptr != NULL) return ESP_OK;
	else return ESP_FAIL;
}
