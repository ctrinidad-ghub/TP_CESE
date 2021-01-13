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

#define ID			"id_Dispositivo\": "
#define LOTE 		"lote_partida\": "
#define TEST_NUM 	"test_Numero\": "
#define VP 			"tension_linea\": "
#define IV 			"corriente_vacio\": "

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Definitions of internal functions]===================================*/

int32_t parseRx(char *rx, char *data, uint8_t data_lenght, char *eod)
{
	char *ptr, *ptr2;
	char str[20];
	int32_t idata;

	for (int i=0;i<sizeof(str);i++) str[i]=0;

	ptr = strstr(rx, data) + data_lenght-1;

	if (ptr != NULL) /* Substring found */
	{
		ptr2 = strstr(ptr, eod); // eod: end of data character
		strncpy(str, ptr, ptr2-ptr);
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
	configData->id=parseRx(rx_buff, ID, sizeof(ID), ",");
	configData->test_num=parseRx(rx_buff, TEST_NUM, sizeof(TEST_NUM), ",");
	configData->vp=parseRx(rx_buff, VP, sizeof(VP), ",");
	configData->iv=parseRx(rx_buff, IV, sizeof(IV), "\r\n}");
}


