/*
 * lcdBoard.h
 *
 *  Created on: 26 sep. 2020
 *      Author: Cristian
 */

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "unistd.h"
#include "sapi_convert.h"

#ifndef INC_LCDBOARD_H_
#define INC_LCDBOARD_H_

// GPIO
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_NUM_23 | 1ULL<<GPIO_NUM_22 | 1ULL<<GPIO_NUM_21 | 1ULL<<GPIO_NUM_19 | 1ULL<<GPIO_NUM_5 | 1ULL<<GPIO_NUM_18)

inline void lcdRSClear(void){
	gpio_set_level(GPIO_NUM_5, 0);
}

inline void lcdRSSet(void){
	gpio_set_level(GPIO_NUM_5, 1);
}

inline void lcdENSet(void){
	gpio_set_level(GPIO_NUM_18, 1);
}

inline void lcdENClear(void){
	gpio_set_level(GPIO_NUM_18, 0);
}

inline void lcdRWClear( void ) {};   // RW = 0 for write

inline void lcdDelay_us( uint32_t time ){
	usleep(time);
}

inline void lcdCommandDelay(void){
	usleep(2000);
}

inline void lcdDataDelay(void){
	usleep(50);
}

inline void lcdDelay_ms( uint32_t time ){
	vTaskDelay(time / portTICK_PERIOD_MS);
}

inline void lcdBoardInit( lcd_set_t lcd_set )
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

inline void lcdSendNibble( uint8_t nibble )
{
	nibble = (nibble >> 4);
	nibble = (nibble & 0x0F);

	gpio_set_level(GPIO_NUM_19, (nibble >> 0) & 0x01);
	gpio_set_level(GPIO_NUM_21, (nibble >> 1) & 0x01);
	gpio_set_level(GPIO_NUM_22, (nibble >> 2) & 0x01);
	gpio_set_level(GPIO_NUM_23, (nibble >> 3) & 0x01);
}

inline void lcdSendPort( uint8_t data )
{

}

#endif /* INC_LCDBOARD_H_ */
