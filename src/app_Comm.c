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

#define ID			"id_Dispositivo\":"
#define LOTE 		"lote_partida\":\""
#define TEST_NUM 	"test_Numero\":"
#define VP_MAX		"tension_primario_max\":"
#define VP_MIN		"tension_primario_min\":"
#define IP_MAX		"corriente_primario_max\":"
#define IP_MIN		"corriente_primario_min\":"
#define VS_MAX		"tension_secundario_max\":"
#define VS_MIN		"tension_secundario_min\":"
#define IS_MAX		"corriente_secundario_max\":"
#define IS_MIN		"corriente_secundario_min\":"

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
	parseRxString(rx_buff, LOTE, sizeof(LOTE), "\",",configData->lote);
	configData->test_num=parseRxInteger(rx_buff, TEST_NUM, sizeof(TEST_NUM), ",");
	configData->trafoParameters.Vp.max=parseRxInteger(rx_buff, VP_MAX, sizeof(VP_MAX), ",");
	configData->trafoParameters.Vp.min=parseRxInteger(rx_buff, VP_MIN, sizeof(VP_MIN), ",");
	configData->trafoParameters.Ip.max=parseRxInteger(rx_buff, IP_MAX, sizeof(IP_MAX), ",");
	configData->trafoParameters.Ip.min=parseRxInteger(rx_buff, IP_MIN, sizeof(IP_MIN), ",");
	configData->trafoParameters.Vs.max=parseRxInteger(rx_buff, VS_MAX, sizeof(VS_MAX), ",")*10;
	configData->trafoParameters.Vs.min=parseRxInteger(rx_buff, VS_MIN, sizeof(VS_MIN), ",")*10;
	configData->trafoParameters.Is.max=parseRxInteger(rx_buff, IS_MAX, sizeof(IS_MAX), ",");
	configData->trafoParameters.Is.min=parseRxInteger(rx_buff, IS_MIN, sizeof(IS_MIN), "}\r\n");
}

void processTxData(char* tx_buff, configData_t *configData)
{
	char aux[100];

	// "id_Dispositivo=4&lote_partida=2033657-1&test_Numero=10&tension_linea=220&corriente_vacio=50"
	strcpy(tx_buff, "id_Dispositivo=");
	sprintf(aux, "%d", (configData->id+1));
	strcat(tx_buff, aux);
	strcat(tx_buff, "&lote_partida=");
	strcat(tx_buff, configData->lote);
	strcat(tx_buff, "&test_Numero=");
	sprintf(aux, "%d", configData->test_num);
	strcat(tx_buff, aux);
	strcat(tx_buff, "&tension_primario_max=");
	sprintf(aux, "%d", (configData->trafoParameters.Vp.max+1));
	strcat(tx_buff, aux);
	strcat(tx_buff, "&tension_primario_min=");
	sprintf(aux, "%d", configData->trafoParameters.Vp.min);
	strcat(tx_buff, aux);
	strcat(tx_buff, "&corriente_primario_max=800&corriente_primario_min=400&tension_secundario_max=25&tension_secundario_min=20&corriente_secundario_max=200&corriente_secundario_min=150");
}
