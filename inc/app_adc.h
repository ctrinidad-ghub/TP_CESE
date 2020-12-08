/**
 * @file app_adc.h
 * 
 * @brief 
 * 
 * @author Cristian Trinidad
 */

#ifndef INC_APP_ADC_H_
#define INC_APP_ADC_H_

#include "driver/adc.h"

/** @cond */
#define SAMPLES_IN_20MS		128 // Amount of samples in 20 ms (power line period)
#define AMOUNT_OF_CYLCES	8   // Amount of cycles to sample ()

//i2s sample rate
#define I2S_SAMPLE_RATE   	(SAMPLES_IN_20MS*50) // 50 = 1/20ms

//I2S read buffer length
#define I2S_READ_LEN      	(SAMPLES_IN_20MS*AMOUNT_OF_CYLCES)

// ADC
#define DEFAULT_VREF    1100
#define ADC_CHANNELS 4
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

// 1st order filter: y[n] = B0 * x[n] + A1 * y[n-1]
#define TAO_US 	500
#define TS_US 	20000/SAMPLES_IN_20MS
#define B0		(1000/(1+TAO_US/TS_US))
#define A1		(B0 * TAO_US/TS_US)

typedef enum {
	PV, PC, SV, SC
} adc_ch_t;

typedef struct {
	adc_channel_t channel;
	uint32_t sum_voltage;
	uint32_t *rms;
	int32_t gain; // x100
	int32_t voltageFilter;
} adc_t;
/** @endcond */

/**
 * @brief This struct type is used across the firmware to store the RMS values
 * 
 */
typedef struct {
	uint32_t Vp;
	uint32_t Vs;
	uint32_t Ip;
	uint32_t Is;
} rms_t;

/**
 * @brief Initialize ADC 
 * 
 */
void appAdcInit(void);

/**
 * @brief Start a new RMS ADC conversion
 * 
 * @param rms 
 * @return * void 
 */
void appAdcStart(rms_t *rms);

#endif /* INC_APP_ADC_H_ */
