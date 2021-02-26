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

#define VS_FAILED 1
#define IP_FAILED 2
#define IS_FAILED 4
#define TEST_PASS 0

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * 
 */
typedef uint8_t test_result_t;

/**
 * @brief
 *
 */
typedef struct {
	test_result_t test_result;
	rms_t rms;
} test_status_t;


#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _TEST_STATUS_H_ */
