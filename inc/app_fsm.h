/**
 * @file app_fsm.h
 * 
 * @brief 
 * 
 * @author Cristian Trinidad
 */

#ifndef _APP_FSM_H_
#define _APP_FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_MSG_WAIT            (3000 / portTICK_PERIOD_MS)  /*!<All the temporary messages in the LCD will be shown for 3 seg      */

/**
 * @brief Initialize main FSM 
 * 
 */
void appFsmInit( void );

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_FSM_H_ */
