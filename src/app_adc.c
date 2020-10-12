/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "../inc/app_adc.h"
#include "esp_adc_cal.h"
#include "driver/timer.h"
#include "driver/i2s.h"

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

uint32_t Vp = 0, Vs = 0, Ip = 0, Is = 0;

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static adc_t adc[ADC_CHANNELS];
static esp_adc_cal_characteristics_t *adc_chars;

/*=====[Definitions of internal functions]===================================*/

#ifdef USE_DMA
uint16_t i2s_read_buff_[I2S_READ_LEN];

void appAdcDmaInit()
{
	 i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN,
        .sample_rate =  I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
	    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
	    .channel_format = I2S_CHANNEL_FMT_ALL_LEFT,
	    .intr_alloc_flags = 0,
	    .dma_buf_count = 2,
	    .dma_buf_len = 1024,
	    .use_apll = 0,
	 };

	 //install and start i2s driver
	 i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

	 //init ADC pad
	 i2s_set_adc_mode(ADC_UNIT_1, adc[0].channel);
}

void adc_dma(void*arg)
{

    size_t bytes_read;
    uint32_t i;
    uint32_t adcIndex = 0;

    // Read from ADC and save
    uint16_t* i2s_read_buff = i2s_read_buff_;

    while (1) {
		i2s_adc_enable(I2S_NUM_0);

		//read data from I2S bus, in this case, from ADC.
		i2s_read(I2S_NUM_0, (uint16_t*) i2s_read_buff, I2S_READ_LEN * sizeof(uint16_t), &bytes_read, portMAX_DELAY);

		i2s_adc_disable(I2S_NUM_0);

		for (i=0;i<I2S_READ_LEN;i++){
			uint16_t voltage = esp_adc_cal_raw_to_voltage(i2s_read_buff_[i] & 0x0FFF, adc_chars);
			adc[adcIndex].sum_voltage += voltage;
		}

		adc[adcIndex].rms = adc[adcIndex].sum_voltage/I2S_READ_LEN;
		switch(adcIndex){
			case 0:
				Vp = adc[adcIndex].rms;
				break;
			case 1:
				Vs = adc[adcIndex].rms;
				break;
			case 2:
				Ip = adc[adcIndex].rms;
				break;
			case 3:
				Is = adc[adcIndex].rms;
				break;
		}
		adc[adcIndex].sum_voltage = 0;
		if (adcIndex == ADC_CHANNELS-1) adcIndex = 0;
		else adcIndex++;

		i2s_set_adc_mode(ADC_UNIT_1, adc[adcIndex].channel);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

#else
/*
 * Timer group0 ISR handler
 *
 * Note:
 * We don't call the timer API here because they are not declared with IRAM_ATTR.
 * If we're okay with the timer irq not being serviced while SPI flash cache is disabled,
 * we can allocate this interrupt without the ESP_INTR_FLAG_IRAM flag and use the normal API.
 */
void IRAM_ATTR timer_group0_isr(void *para)
{
	timer_event_t *evt = (timer_event_t *) para;

    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
    uint32_t intr_status = TIMERG0.int_st_timers.val;
    TIMERG0.hw_timer[evt->timer_idx].update = 1;


    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
    if ((intr_status & BIT(evt->timer_idx)) && evt->timer_idx == TIMER_0) {
        TIMERG0.int_clr_timers.t0 = 1;
    }

    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    TIMERG0.hw_timer[evt->timer_idx].config.alarm_en = TIMER_ALARM_EN;

    //TaskNotify
    xTaskNotifyFromISR(evt->Taskptr,0x00,eNoAction,NULL);
    portYIELD_FROM_ISR();
}

/*
 * Initialize selected timer of the timer group 0
 *
 * timer_idx - the timer number to initialize
 * auto_reload - should the timer auto reload on alarm?
 * timer_interval_usec - the interval of alarm to set
 * Taskptr - pointer to the handler timer task
 */
static void app_timer_init(timer_idx_t timer_idx,
    bool auto_reload, double timer_interval_usec, TaskHandle_t *Taskptr)
{
	//define struct for ISR Pointer
	timer_event_t *Timer_Specs_p = calloc(sizeof(timer_event_t), 1);
	Timer_Specs_p->Taskptr = *Taskptr;
	Timer_Specs_p->timer_idx = timer_idx;

    /* Select and initialize basic parameters of the timer */
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, (TIMER_SCALE * timer_interval_usec) / 1000000);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
    		 (void *) Timer_Specs_p, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}

/*
 * The main task of this example program
 */
void periodic_task(void *arg)
{
	uint32_t adc_reading = 0;
	uint32_t voltage;
	uint32_t i = 0;
	uint32_t adcIndex = 0;

    while (1) {
    	xTaskNotifyWait(0x00, 0xffffffff, NULL, portMAX_DELAY);

        // ADC processing
        adc_reading = adc1_get_raw((adc1_channel_t) adc[adcIndex].channel);

        //Convert adc_reading to voltage in mV
        voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

        /*voltage = voltage * voltage;
        adc[adcIndex].sum_voltage += voltage;
        i++;
        if (i == NO_OF_SAMPLES){
        	i = 0;
        	adc[adcIndex].sum_voltage /= NO_OF_SAMPLES_PRE_CH;
        	//printf("%d\n", sum_voltage);
        	rms = sqrt(sum_voltage);
        	adc[adcIndex].sum_voltage = 0;
        	//printf("%d\n", rms);
        }*/
        adc[adcIndex].sum_voltage += voltage;
        i++;
        if (i == 20) {
        	i = 0;
        	adc[adcIndex].rms = adc[adcIndex].sum_voltage/20;
        	switch(adcIndex){
				case 0:
					Vp = adc[adcIndex].rms;
					break;
				case 1:
					Vs = adc[adcIndex].rms;
					break;
				case 2:
					Ip = adc[adcIndex].rms;
					break;
				case 3:
					Is = adc[adcIndex].rms;
					break;
        	}
        	adc[adcIndex].sum_voltage = 0;
        	if (adcIndex == ADC_CHANNELS-1) adcIndex = 0;
        	else adcIndex++;
        }
    }
}

#endif

/*=====[Definitions of external functions]===================================*/

void appAdcInit(void)
{
#ifndef USE_DMA
	TaskHandle_t handle_periodic_task;
#endif
	uint32_t adcIndex = 0;

	adc[0].channel = (adc_channel_t) PV_CH;
	adc[1].channel = (adc_channel_t) PC_CH;
	adc[2].channel = (adc_channel_t) SV_CH;
	adc[3].channel = (adc_channel_t) SC_CH;

	adc1_config_width(ADC_WIDTH_BIT_12);

	for (adcIndex = 0; adcIndex < ADC_CHANNELS; adcIndex++)
	{
		adc[adcIndex].sum_voltage = 0;
		adc[adcIndex].rms = 0;
		//Configure ADC
		adc1_config_channel_atten(adc[adcIndex].channel, (adc_atten_t) ATTEN);
	}

	//Characterize ADC
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_characterize((adc_unit_t) ADC_UNIT_1, (adc_atten_t) ATTEN, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

#ifdef USE_DMA
	appAdcDmaInit();
	xTaskCreate(adc_dma, "adc_dma", 1024 * 2, NULL, 5, NULL);
#else
	xTaskCreate(periodic_task, "timer_periodic_task", 2048, NULL, 5, &handle_periodic_task);
	app_timer_init((timer_idx_t) TIMER_0, TEST_WITH_RELOAD, TIMER_INTERVAL0_USEC, &handle_periodic_task);
#endif
}


