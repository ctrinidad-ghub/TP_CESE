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

/**
 * @brief Configuration data type
 *
 */
typedef struct {
	uint32_t id;
	char lote[LOTE_LENGTH];
	uint32_t test_num;
	int32_t vp;
	int32_t iv;
} configData_t;

/**
 * @brief Parse configuration rx data got from web server
 *
 * @param rx_buff Rx buffer got from web server
 * @param configData Pointer to a configuration struct to store the data
 *
 */
void processRxData(char* rx_buff, configData_t *configData);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_COMM_H_ */
