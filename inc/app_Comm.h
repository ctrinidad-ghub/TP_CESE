/**
 * @file app_Comm.h
 *
 * @brief
 *
 * @author Cristian Trinidad
 */

#ifndef _APP_COMM_H_
#define _APP_COMM_H_

#include "../inc/test_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond */
#define BATCHID_LENGTH 20
#define CODE_LENGTH    20
/** @endcond */

/**
 * @brief Parameter thresholds structure
 *
 */
typedef struct {
	int32_t max;
	int32_t min;
} parametersRange_t;

/**
 * @brief Measured Parameters structure
 *
 */
typedef struct {
	parametersRange_t Vinp;   /*!<Primary Voltage                                    */
	parametersRange_t Voutp;  /*!<Primary Voltage                                    */
	parametersRange_t Vins;   /*!<Secondary Voltage                                  */
	parametersRange_t Vouts;  /*!<Secondary Voltage                                  */
	parametersRange_t Ip;     /*!<Primary current                                    */
	parametersRange_t Is;     /*!<Secondary current                                  */
} trafoParameters_t;

/**
 * @brief Configuration data type
 *
 */
typedef struct {
	uint32_t id;					    /*!<Test Identification Number                           */
	char batchId[BATCHID_LENGTH];		/*!<Transformer BatchId                                    */
	char code[CODE_LENGTH];		        /*!<Transformer code                                     */
	trafoParameters_t trafoParameters;  /*!<Measured Parameters structure                        */
} configData_t;

/**
 * @brief Parse configuration rx data got from web server
 *
 * @param rx_buff Rx buffer got from web server
 * @param configData Pointer to a configuration struct to store the data
 *
 * @return
 *  - ESP_OK on successful
 *  - ESP_FAIL on error:
 *     - Incomplete JSON data
 *     - Invalid numbers, e.g. number contains letters
 *
 */
esp_err_t processRxData(char* rx_buff, configData_t *configData);

/**
 * @brief Perform tx package to be sent to the web server
 *
 * @param tx_buff Tx buffer to send to the web server
 * @param configData Pointer to a configuration struct to store the data
 * @param test_status test_status Pointer to the test result
 *
 */
void processTxData(char* tx_buff, configData_t *configData, test_status_t *test_status);

/**
 * @brief Check POST body from the web server
 *
 * @param rx_buff Pointer to a receive buffer
 *
 * @return
 *  - ESP_OK on successful
 *  - ESP_FAIL on error
 *
 */
esp_err_t checkPostData(char *rx_buff);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_COMM_H_ */
