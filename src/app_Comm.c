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

	ptr = strstr(rx, data) + data_lenght-1;

	if (ptr != NULL) /* Substring found */
	{
		ptr2 = strstr(ptr, eod); // eod: end of data character
		strncpy(str, ptr, ptr2-ptr);
	}
}

int32_t parseRxInteger(char *rx, char *data, uint8_t data_lenght, char *eod)
{
	char str[20];
	int32_t idata;

	for (int i=0;i<sizeof(str);i++) str[i]=0;

	parseRxString(rx, data, data_lenght, eod, str);

	if (str != NULL) /* Substring found */
	{
		idata=atoi(str);
	}
	else /* Substring not found */
	{
		idata=0;
	}
	return idata;
}

/*=====[Definitions of external functions]===================================*/

void processRxData(char* rx_buff, configData_t *configData)
{
	configData->id=parseRxInteger(rx_buff, ID, sizeof(ID), ",");
	parseRxString(rx_buff, BATCHID, sizeof(BATCHID), "\",",configData->batchId);
	parseRxString(rx_buff, CODE, sizeof(CODE), "\",",configData->code);
	configData->trafoParameters.Vinp.max=parseRxInteger(rx_buff, VINP_MAX, sizeof(VINP_MAX), ".00,");
	configData->trafoParameters.Vinp.min=parseRxInteger(rx_buff, VINP_MIN, sizeof(VINP_MIN), ".00,");
	configData->trafoParameters.Voutp.max=parseRxInteger(rx_buff, VOUTP_MAX, sizeof(VOUTP_MAX), ".00,");
	configData->trafoParameters.Voutp.min=parseRxInteger(rx_buff, VOUTP_MIN, sizeof(VOUTP_MIN), ".00,");
	configData->trafoParameters.Ip.max=parseRxInteger(rx_buff, IP_MAX, sizeof(IP_MAX), ".00,");
	configData->trafoParameters.Ip.min=parseRxInteger(rx_buff, IP_MIN, sizeof(IP_MIN), ".00,");
	configData->trafoParameters.Vins.max=parseRxInteger(rx_buff, VINS_MAX, sizeof(VINS_MAX), ".00,")*100;
	configData->trafoParameters.Vins.min=parseRxInteger(rx_buff, VINS_MIN, sizeof(VINS_MIN), ".00,")*100;
	configData->trafoParameters.Vouts.max=parseRxInteger(rx_buff, VOUTS_MAX, sizeof(VOUTS_MAX), ".00,")*100;
	configData->trafoParameters.Vouts.min=parseRxInteger(rx_buff, VOUTS_MIN, sizeof(VOUTS_MIN), ".00,")*100;
	configData->trafoParameters.Is.min=parseRxInteger(rx_buff, IS_MIN, sizeof(IS_MIN), ".00,");
	configData->trafoParameters.Is.max=parseRxInteger(rx_buff, IS_MAX, sizeof(IS_MAX), ".00}");
}

void processTxData(char* tx_buff, configData_t *configData, test_status_t *test_status)
{
	char aux[100];

	strcpy(tx_buff, "{\"Code\":\"");
	strcat(tx_buff, configData->code);
	strcat(tx_buff, "\",\"BatchId\":\"");
	strcat(tx_buff, configData->batchId);
	strcat(tx_buff, "\",\"VinPrimary\":");
	sprintf(aux, "%d", test_status->Vinp);
	strcat(tx_buff, aux);
	strcat(tx_buff, ",\"VoutSecondary\":");
	sprintf(aux, "%d", test_status->Vouts);
	strcat(tx_buff, aux);
	strcat(tx_buff, ",\"IPrimary\":");
	sprintf(aux, "%d", test_status->Ip);
	strcat(tx_buff, aux);
	strcat(tx_buff, ",\"VinSecondary\":");
	sprintf(aux, "%d", test_status->Vins);
	strcat(tx_buff, aux);
	strcat(tx_buff, ",\"VoutPrimary\":");
	sprintf(aux, "%d", test_status->Voutp);
	strcat(tx_buff, aux);
	strcat(tx_buff, ",\"ISecondary\":");
	sprintf(aux, "%d", test_status->Is);
	strcat(tx_buff, aux);
	if (test_status->test_result == TEST_PASS) {
		strcat(tx_buff, ",\"TestResult\":\"OK\"}");
	}
	else {
		strcat(tx_buff, ",\"TestResult\":\"RECHAZADO\"}");
	}
}
