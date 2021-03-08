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

/** @cond */
#define LOTE_LENGTH 20
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
	parametersRange_t Vp;  /*!<Primary Voltage                                    */
	parametersRange_t Vs;  /*!<Secondary Voltage                                  */
	parametersRange_t Ip;  /*!<Primary current                                    */
	parametersRange_t Is;  /*!<Secondary current                                  */
} trafoParameters_t;

/**
 * @brief Configuration data type
 *
 */
typedef struct {
	uint32_t id;					    /*!<Test Identification Number                           */
	char lote[LOTE_LENGTH];		        /*!<Transformer batch                                    */
	uint32_t test_num;                  /*!<Test Number                                          */
	trafoParameters_t trafoParameters;  /*!<Measured Parameters structure                        */
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
 * @brief Perform tx package to be sent to the web server
 *
 * @param tx_buff Tx buffer to send to the web server
 * @param configData Pointer to a configuration struct to store the data
 *
 */
void processTxData(char* tx_buff, configData_t *configData);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_COMM_H_ */
