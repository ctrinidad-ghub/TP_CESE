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

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

uint32_t Vp = 0, Vs = 0, Ip = 0, Is = 0;

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static adc_t adc[ADC_CHANNELS];

/*=====[Definitions of internal functions]===================================*/

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
        voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc[adcIndex].adc_chars);

        /*voltage = voltage * voltage;
        adc[adcIndex].sum_voltage += voltage;
        i++;
        if (i == (20000 / (TIMER_INTERVAL0_USEC))){
        	i = 0;
        	adc[adcIndex].sum_voltage /= (20000 / (TIMER_INTERVAL0_USEC));
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

/*=====[Definitions of external functions]===================================*/

void appAdcInit(void)
{
	TaskHandle_t handle_periodic_task;
	uint32_t adcIndex = 0;

	adc[0].channel = (adc_channel_t) PV_CH;
	adc[1].channel = (adc_channel_t) PC_CH;
	adc[2].channel = (adc_channel_t) SV_CH;
	adc[3].channel = (adc_channel_t) SC_CH;

	for (adcIndex = 0; adcIndex < ADC_CHANNELS; adcIndex++)
	{
		//Configure ADC
		adc1_config_width(ADC_WIDTH_BIT_12);
		adc1_config_channel_atten(adc[adcIndex].channel, (adc_atten_t) ATTEN);

		//Characterize ADC
		adc[adcIndex].adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
		esp_adc_cal_characterize((adc_unit_t) ADC_UNIT_1, (adc_atten_t) ATTEN, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc[adcIndex].adc_chars);
	}

	xTaskCreate(periodic_task, "timer_periodic_task", 2048, NULL, 5, &handle_periodic_task);
	app_timer_init((timer_idx_t) TIMER_0, TEST_WITH_RELOAD, TIMER_INTERVAL0_USEC, &handle_periodic_task);
}


