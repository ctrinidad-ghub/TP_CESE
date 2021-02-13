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
#define VP 			"tension_linea\":"
#define IV 			"corriente_vacio\":"

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
	configData->vp=parseRxInteger(rx_buff, VP, sizeof(VP), ",");
	configData->iv=parseRxInteger(rx_buff, IV, sizeof(IV), "}\r\n");
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
	strcat(tx_buff, "&tension_linea=");
	sprintf(aux, "%d", (configData->vp+1));
	strcat(tx_buff, aux);
	strcat(tx_buff, "&corriente_vacio=");
	sprintf(aux, "%d", configData->iv);
	strcat(tx_buff, aux);
}
