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
#include "freertos/queue.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/timer.h"

#define USE_DMA

#define SAMPLES_IN_20MS		128 // Amount of samples in 20 ms (power line period)
#define AMOUNT_OF_CYLCES	8   // Amount of cycles to sample ()

//i2s sample rate
#define I2S_SAMPLE_RATE   	(SAMPLES_IN_20MS*50) // 50 = 1/20ms

//I2S read buffer length
#define I2S_READ_LEN      	(SAMPLES_IN_20MS*AMOUNT_OF_CYLCES)


// TMR
#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_USEC  (50)     // test interval for the timer in us
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload

// ADC
#define DEFAULT_VREF    1100
#define NO_OF_SAMPLES   20000 / (TIMER_INTERVAL0_USEC)

#define ADC_CHANNELS 4
#define NO_OF_SAMPLES_PRE_CH   NO_OF_SAMPLES / ADC_CHANNELS
#define SC_CH ADC1_CHANNEL_6
#define PV_CH ADC1_CHANNEL_7
#define SV_CH ADC1_CHANNEL_4
#define PC_CH ADC1_CHANNEL_5
#define ATTEN ADC_ATTEN_DB_11

#define ZER0        1300 // mV
#define GAIN_230V    300
#define GAIN_30V     100
#define GAIN_800mA   100 // Primary current
#define GAIN_1500mA  100 // Secondary current

typedef enum {
	PV, PC, SV, SC
} adc_ch_t;

typedef struct {
	adc_channel_t channel;
	uint32_t sum_voltage;
	uint32_t *rms;
	int32_t gain; // x100
} adc_t;

typedef struct {
	uint32_t Vp;
	uint32_t Vs;
	uint32_t Ip;
	uint32_t Is;
} rms_t;

void appAdcInit(void);
void appAdcStart(rms_t *rms);

/*
 * A sample structure to pass events
 * from the timer interrupt handler to the main program.
 */
typedef struct {
    timer_idx_t timer_idx;
    TaskHandle_t Taskptr;
} timer_event_t;

#endif /* INC_APP_ADC_H_ */
