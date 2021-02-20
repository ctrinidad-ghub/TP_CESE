/**
 * @file app_Comm.h
 *
 * @brief
 *
 * @author Cristian Trinidad
 */

#ifndef _APP_COMM_H_
#define _APP_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LOTE_LENGTH 20

typedef struct {
	int32_t max;
	int32_t min;
} parametersRange_t;

typedef struct {
	parametersRange_t Vp;
	parametersRange_t Vs;
	parametersRange_t Ip;
	parametersRange_t Is;
} trafoParameters_t;

/**
 * @brief Configuration data type
 *
 */
typedef struct {
	uint32_t id;
	char lote[LOTE_LENGTH];
	uint32_t test_num;
	trafoParameters_t trafoParameters;
} configData_t;

/**
 * @brief Parse configuration rx data got from web server
 *
 * @param rx_buff Rx buffer got from web server
 * @param configData Pointer to a configuration struct to store the data
 *
 */
void processRxData(char* rx_buff, configData_t *configData);

/**
 * @brief Perform tx package to be send to the web server
 *
 * @param rx_buff Tx buffer to send to the web server
 * @param configData Pointer to a configuration struct to store the data
 *
 */
void processTxData(char* tx_buff, configData_t *configData);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_COMM_H_ */
