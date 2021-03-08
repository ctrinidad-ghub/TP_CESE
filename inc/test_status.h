/**
 * @file test_status.h
 * 
 * @brief 
 * 
 * @author Cristian Trinidad
 */

#ifndef _TEST_STATUS_H_
#define _TEST_STATUS_H_

#include "../inc/app_adc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VS_FAILED 1 /*!<Secondary Voltage test has failed                           */
#define IP_FAILED 2 /*!<Primary current test has failed                             */
#define IS_FAILED 4 /*!<Secondary current test has failed                           */
#define TEST_PASS 0 /*!<The test has passed without failures                        */

/**
 * @brief Test result data type
 * 
 */
typedef uint8_t test_result_t;

/**
 * @brief Test status information
 *
 */
typedef struct {
	test_result_t test_result;
	rms_t rms;
	uint32_t Vl; // Line Voltage - V
} test_status_t;


#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _TEST_STATUS_H_ */
