/**
 * @file app_gpio.h
 * 
 * @brief 
 * 
 * @author Cristian Trinidad
 */

#ifndef _APP_GPIO_H_
#define _APP_GPIO_H_

#include "driver/gpio.h"
#include "../inc/test_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond */
#define CPV     GPIO_NUM_26
#define CSV     GPIO_NUM_25

#define pCan    GPIO_NUM_27
#define pConf   GPIO_NUM_36
#define pTest   GPIO_NUM_39

#define buzzer        GPIO_NUM_4
#define sSwitch       GPIO_NUM_2 // Safety Switch
/** @endcond */

/**
 * @brief Initialize GPIO
 * 
 * @note This function configures the GPIOs for:
 *      - Transformer's primary and secondary windings
 *      - Front panel buttons
 *      - SafetySwitch
 * 
 */
void appGpioInit( void );

/**
 * @brief Connect the transformer's primary winding to the external primary voltage input
 * 
 */
void connectPrimary(void);

/**
 * @brief Connect the transformer's secondary winding to the external secondary voltage input
 * 
 */
void connectSecondary(void);

/**
 * @brief Disconnect both transformer's winding from the external voltage inputs
 * 
 */
void disconnectPrimarySecondary(void);

/**
 * @brief Check if the test button was pressed
 * 
 * @return true 
 * @return false 
 */
bool isTestPressed( void );

/**
 * @brief Check if the cancel button was pressed
 * 
 * @return true 
 * @return false 
 */
bool isCancelPressed( void );

/**
 * @brief Check if the config button was pressed
 * 
 * @return true 
 * @return false 
 */
bool isConfigPressed( void );

/**
 * @brief Trigger buzzer sounds
 *
 * @param test_result
 */
void triggerBuzzer(test_result_t test_result);

/**
 * @brief Check if the Safety Switch is open
 *
 * @return true
 * @return false
 */
bool isSafetySwitchOpen( void );

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_GPIO_H_ */
