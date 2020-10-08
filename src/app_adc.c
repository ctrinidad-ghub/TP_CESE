/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/timer.h"
#include "../inc/adc.h"

// ADC
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

uint32_t i = 0;
uint32_t sum_voltage = 0;
uint32_t rms = 0;

static void app_timer_init(timer_idx_t timer_idx,
    bool auto_reload, double timer_interval_usec, TaskHandle_t *Taskptr);

/*
 * A sample structure to pass events
 * from the timer interrupt handler to the main program.
 */
typedef struct {
    timer_idx_t timer_idx;
    TaskHandle_t Taskptr;
} timer_event_t;

/*
 * The main task of this example program
 */
void periodic_task(void *arg)
{
	uint32_t adc_reading = 0;

    while (1) {
    	xTaskNotifyWait(0x00, 0xffffffff, NULL, portMAX_DELAY);

        // ADC processing
        adc_reading = adc1_get_raw((adc1_channel_t)channel);

        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

        /*voltage = voltage * voltage;
        sum_voltage += voltage;
        i++;
        if (i == (20000 / (TIMER_INTERVAL0_USEC))){
        	i = 0;
        	sum_voltage /= (20000 / (TIMER_INTERVAL0_USEC));
        	//printf("%d\n", sum_voltage);
        	rms = sqrt(sum_voltage);
        	sum_voltage = 0;
        	//printf("%d\n", rms);
        }*/
        sum_voltage += voltage;
        i++;
        if (i == 20) {
        	i = 0;
        	rms = sum_voltage/20;
        	sum_voltage = 0;
        }
    }
}

// TMR
#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_USEC  (200)    // test interval for the timer in us
#define TEST_WITHOUT_RELOAD   0        // testing will be done without auto reload
#define TEST_WITH_RELOAD      1        // testing will be done with auto reload

void adc_init(void)
{
	TaskHandle_t handle_periodic_task;

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

	xTaskCreate(periodic_task, "timer_periodic_task", 2048, NULL, 5, &handle_periodic_task);
	app_timer_init((timer_idx_t) TIMER_0, TEST_WITH_RELOAD, TIMER_INTERVAL0_USEC, &handle_periodic_task);
}

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
static void check_efuse()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}
*/
