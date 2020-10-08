/*
 * adc.h
 *
 *  Created on: 4 oct. 2020
 *      Author: Cristian
 */

#ifndef INC_APP_ADC_H_
#define INC_APP_ADC_H_

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/timer.h"

// ADC
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

#define ADC_CHANNELS 4
#define SC_CH ADC_CHANNEL_6
#define PV_CH ADC_CHANNEL_7
#define SV_CH ADC_CHANNEL_4
#define PC_CH ADC_CHANNEL_5
#define ATTEN ADC_ATTEN_DB_11

// TMR
#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_USEC  (200)    // test interval for the timer in us
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload

typedef enum {
	PV, PC, SV, SC
} adc_ch_t;

void appAdcInit(void);

extern uint32_t Vp, Vs, Ip, Is;

typedef struct {
	adc_channel_t channel;
	uint32_t sum_voltage;
	uint32_t rms;
	esp_adc_cal_characteristics_t *adc_chars;
} adc_t;

/*
 * A sample structure to pass events
 * from the timer interrupt handler to the main program.
 */
typedef struct {
    timer_idx_t timer_idx;
    TaskHandle_t Taskptr;
} timer_event_t;

#endif /* INC_APP_ADC_H_ */
