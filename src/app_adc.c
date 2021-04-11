/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/i2s.h"
#include "esp_err.h"
#include "../inc/app_adc.h"

/*=====[Definition of private macros, constants or data types]===============*/

typedef struct {
	adc_status_enum_t status;
	QueueHandle_t adc_queue;
	SemaphoreHandle_t startConv;
} adc_status_t;

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static adc_status_t adc_status;
static adc_t adc[ADC_CHANNELS];
static esp_adc_cal_characteristics_t *adc_chars;
static rms_t rms;
static uint16_t I2SReadBuff[I2S_READ_LEN];

/*=====[Definitions of internal functions]===================================*/

void appAdcEnable()
{
	 i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
        .sample_rate =  I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
	    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
	    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
	    .intr_alloc_flags = 0,
	    .dma_buf_count = 2, // Debe ser > 1
	    .dma_buf_len = I2S_READ_LEN,
	    .use_apll = 0,
	 };

	 //install and start i2s driver
	 i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

	 //init ADC pad
	 i2s_set_adc_mode(ADC_UNIT_1, adc[VP_INDEX].channel);

	 adc_status.status = ENABLE;
}

void appAdcDisable(void){
	adc_status.status = DISABLE;
	i2s_driver_uninstall(I2S_NUM_0);
}

adc_status_enum_t appAdcStatus(void){
	return adc_status.status;
}

void appAdcStart(rms_t *rms)
{
	rms_t rms_;
	adc_status.status = CONV;

	// Start conversion
	xSemaphoreGive(adc_status.startConv);

	xQueueReceive(adc_status.adc_queue, &rms_, portMAX_DELAY);

	adc_status.status = ENABLE;
	rms->Vp = rms_.Vp;
	rms->Ip = rms_.Ip;
	rms->Vs = rms_.Vs;
	rms->Is = rms_.Is;
}

void adc_dma_task(void*arg)
{
    size_t bytes_read;
    uint32_t i, j;
    uint32_t adcIndex = 0;
    uint8_t adc_init = 0;
    uint16_t* pI2SReadBuff= I2SReadBuff;
    int32_t s_rms; // signed rms value

    while (1) {
		// Read ADC data from I2S bus
		if (i2s_read(I2S_NUM_0, (uint16_t*) pI2SReadBuff, I2S_READ_LEN * sizeof(uint16_t), &bytes_read, 300 / portTICK_PERIOD_MS) == ESP_OK) {

			i2s_adc_disable(I2S_NUM_0);

			*adc[adcIndex].rms = 0;
			adc[adcIndex].voltageFilter = 0;

			for (j=0; j<AMOUNT_OF_CYLCES; j++) {
				for (i=0; i<SAMPLES_IN_20MS; i++) {
					I2SReadBuff[i+SAMPLES_IN_20MS*j] = esp_adc_cal_raw_to_voltage(I2SReadBuff[i+SAMPLES_IN_20MS*j] & 0x0FFF, adc_chars);
					int16_t voltage = I2SReadBuff[i+SAMPLES_IN_20MS*j];
					voltage -= ZER0;
					adc[adcIndex].voltageFilter = (B0 * voltage + A1 * adc[adcIndex].voltageFilter)/1000;
					adc[adcIndex].sum_voltage += adc[adcIndex].voltageFilter * adc[adcIndex].voltageFilter;
				}
				if (j!=0) {
					adc[adcIndex].sum_voltage /= SAMPLES_IN_20MS;
					*adc[adcIndex].rms += sqrt(adc[adcIndex].sum_voltage);
				}
			}

			*adc[adcIndex].rms /= (AMOUNT_OF_CYLCES-1);
			s_rms = (*adc[adcIndex].rms * adc[adcIndex].gain)/100 + adc[adcIndex].offset;
			if (s_rms > 0) *adc[adcIndex].rms = (uint32_t) s_rms;
			else *adc[adcIndex].rms = 0;

			adc[adcIndex].sum_voltage = 0;
			if (adcIndex == ADC_CHANNELS-1) {
				adcIndex = 0;
				if (adc_init == 1) xQueueSend( adc_status.adc_queue, ( void * ) &rms, ( TickType_t ) 0 );
				adc_init = 1;
				xSemaphoreTake(adc_status.startConv, portMAX_DELAY);
			}
			else adcIndex++;
		}
		else
		{
			i2s_adc_disable(I2S_NUM_0);
		}
		i2s_set_adc_mode(ADC_UNIT_1, adc[adcIndex].channel);
		// Dummy read to prevent from reading corrupt data at changing the channel
		i2s_read(I2S_NUM_0, (uint16_t*) pI2SReadBuff, I2S_READ_LEN * sizeof(uint16_t), &bytes_read, 300 / portTICK_PERIOD_MS);
		i2s_adc_enable(I2S_NUM_0);
    }

    vTaskDelete(NULL);
}

/*=====[Definitions of external functions]===================================*/

void appAdcInit(void)
{
	uint32_t adcIndex = 0;

	adc[VP_INDEX].channel = (adc_channel_t) PV_CH;
	adc[IP_INDEX].channel = (adc_channel_t) PC_CH;
	adc[VS_INDEX].channel = (adc_channel_t) SV_CH;
	adc[IS_INDEX].channel = (adc_channel_t) SC_CH;
	adc[VP_INDEX].rms = &rms.Vp;
	adc[IP_INDEX].rms = &rms.Ip;
	adc[VS_INDEX].rms = &rms.Vs;
	adc[IS_INDEX].rms = &rms.Is;
	adc[VP_INDEX].gain = GAIN_230V;
	adc[IP_INDEX].gain = GAIN_800mA;
	adc[VS_INDEX].gain = GAIN_30V;
	adc[IS_INDEX].gain = GAIN_1500mA;
	adc[VP_INDEX].offset = OFFSET_230V;
	adc[VS_INDEX].offset = OFFSET_30V;
	adc[IP_INDEX].offset = OFFSET_800mA;
	adc[IS_INDEX].offset = OFFSET_1500mA;

	adc1_config_width(ADC_WIDTH_BIT_12);

	for (adcIndex = 0; adcIndex < ADC_CHANNELS; adcIndex++)
	{
		adc[adcIndex].sum_voltage = 0;
		*adc[adcIndex].rms = 0;
		//Configure ADC
		adc1_config_channel_atten(adc[adcIndex].channel, (adc_atten_t) ATTEN);
	}

	adc_status.adc_queue = xQueueCreate(1, sizeof(rms_t));
	adc_status.startConv = xSemaphoreCreateBinary();
	if(adc_status.adc_queue == NULL || adc_status.startConv == NULL)
	{
		// TODO: Define error policy
		while(1);
	}

	//Characterize ADC
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	if( adc_chars == NULL)
	{
		// TODO: Define error policy
		while(1);
	}
	esp_adc_cal_characterize((adc_unit_t) ADC_UNIT_1, (adc_atten_t) ATTEN, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

	appAdcEnable();
	BaseType_t res = xTaskCreate(adc_dma_task, "adc_dma_task", 1024 * 4, NULL, 5, NULL);
	if (res != pdPASS)
	{
		// TODO: Define error policy
		while(1);
	}
}


