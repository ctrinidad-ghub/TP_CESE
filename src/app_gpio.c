/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "stdlib.h"
#include "driver/gpio.h"
#include "../inc/button.h"
#include "../inc/app_gpio.h"
#include "driver/timer.h"

/*=====[Definition of private macros, constants or data types]===============*/

// GPIO
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<CPV | 1ULL<<CSV | 1ULL<<buzzer)
#define GPIO_INPUT_PIN_SEL   (1ULL<<pCan | 1ULL<<pConf | 1ULL<<pTest)

// TMR
#define TIMER_DIVIDER         16     //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL0_USEC  200    // timer interval in us --> 5khz tone
#define WITHOUT_RELOAD        0
#define WITH_RELOAD           1

#define BUZZER_ACTIVE_US	  500000 // 500 ms
#define BUZZER_ACTIVE_COUNT   (BUZZER_ACTIVE_US/TIMER_INTERVAL0_USEC)

#define BUZZER_PERIODS_PASS   1 // 1 cycle if test passed
#define BUZZER_PERIODS_FAIL   5 // 5 cycles if test failed

// structure from timer interrupt handler to the main program
typedef struct {
    timer_idx_t timer_idx;
    TaskHandle_t Taskptr;
} timer_event_t;

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static button_t testButton, cancelButton, configButton;
static uint32_t buzzerPeriods = 0;
static bool activePeriod = 1;

/*=====[Definitions of internal functions]===================================*/

static void trafoPinInit( void )
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

    gpio_set_level(CPV, 1);
	gpio_set_level(CSV, 1);
	gpio_set_level(buzzer, 1);

    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;

    gpio_config(&io_conf);
}

void button_task( void* taskParmPtr )
{

	fsmButtonInit( &cancelButton, pCan );
	fsmButtonInit( &configButton, pConf );
	fsmButtonInit( &testButton, pTest );

	while( 1 )
	{
		fsmButtonUpdate( &cancelButton );
		fsmButtonUpdate( &configButton );
		fsmButtonUpdate( &testButton );

		vTaskDelay( BUTTON_RATE );
	}
}

// Timer group0 ISR handler
void IRAM_ATTR timer_group0_isr(void *para)
{
	timer_event_t *evt = (timer_event_t *) para;

    uint32_t intr_status = TIMERG0.int_st_timers.val;
    TIMERG0.hw_timer[evt->timer_idx].update = 1;

    // Clear the interrupt
    if ((intr_status & BIT(evt->timer_idx)) && evt->timer_idx == TIMER_0) {
        TIMERG0.int_clr_timers.t0 = 1;
    }
    // Renable alarm
    TIMERG0.hw_timer[evt->timer_idx].config.alarm_en = TIMER_ALARM_EN;

    // TaskNotify
    xTaskNotifyFromISR(evt->Taskptr,0x00,eNoAction,NULL);
    portYIELD_FROM_ISR();
}

// Initialize selected timer of the timer group 0

// timer_idx - the timer number to initialize
// auto_reload - should the timer auto reload on alarm?
// timer_interval_usec - the interval of alarm to set
// Taskptr - pointer to the handler timer task

static void app_timer_init(timer_idx_t timer_idx,
    bool auto_reload, double timer_interval_usec, TaskHandle_t *Taskptr)
{
	// define struct for ISR Pointer
	timer_event_t *Timer_Specs_p = calloc(sizeof(timer_event_t), 1);
	Timer_Specs_p->Taskptr = *Taskptr;
	Timer_Specs_p->timer_idx = timer_idx;
    // Select and initialize basic parameters of the timer
    timer_config_t config;
    config.divider = TIMER_DIVIDER;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = auto_reload;
    timer_init(TIMER_GROUP_0, timer_idx, &config);
    // Timer's counter will initially start from value below.
    // Also, if auto_reload is set, this value will be automatically reload on alarm
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);
    // Configure the alarm value and the interrupt on alarm
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, (TIMER_SCALE * timer_interval_usec) / 1000000);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
    		 (void *) Timer_Specs_p, ESP_INTR_FLAG_IRAM, NULL);
}

void periodic_task(void *arg)
{
	bool level = 0;
	uint32_t buzzerCounter = BUZZER_ACTIVE_COUNT;

    while (1) {
    	// Wait until alarm timer (200 us)
    	xTaskNotifyWait(0x00, 0xffffffff, NULL, portMAX_DELAY);

    	if (activePeriod == 1) {
			if (level) {
				gpio_set_level(buzzer, 0);
				level = 0;
			}
			else {
				gpio_set_level(buzzer, 1);
				level = 1;
			}
    	}

    	if (buzzerCounter == 0) {
    		if (buzzerPeriods == 0) timer_pause(TIMER_GROUP_0, TIMER_0);
    		else if (activePeriod == 1) {
    			buzzerPeriods--;
    			activePeriod = 0;
    		}
    		else activePeriod = 1;

    		buzzerCounter = BUZZER_ACTIVE_COUNT;
    	}
    	else buzzerCounter--;
    }
}

/*=====[Definitions of external functions]===================================*/

void triggerBuzzer(test_result_t test_result)
{
	if (test_result == TEST_PASS)
		buzzerPeriods = BUZZER_PERIODS_PASS;
	else
		buzzerPeriods = BUZZER_PERIODS_FAIL;

	activePeriod = 1;

    timer_start(TIMER_GROUP_0, TIMER_0);
}

void connectPrimary(void)
{
	gpio_set_level(CPV, 0);
	gpio_set_level(CSV, 1);
}

void connectSecondary(void) // connect secondary
{
	gpio_set_level(CPV, 1);
	gpio_set_level(CSV, 0);
}

void disconnectPrimarySecondary(void) // disconnect primary secondary
{
	gpio_set_level(CPV, 1);
	gpio_set_level(CSV, 1);
}

bool isTestPressed( void ){
	return isButtonPressed( &testButton );
}

bool isCancelPressed( void ){
	return isButtonPressed( &cancelButton );
}

bool isConfigPressed( void ){
	return isButtonPressed( &configButton );
}

void appGpioInit( void )
{
	TaskHandle_t handle_periodic_task;

	trafoPinInit( );

	BaseType_t res = xTaskCreate(button_task, "button_task", 1024 * 2, NULL, 5, NULL);
	if (res != pdPASS)
	{
		// TODO: Define error policy
		while(1);
	}
	res = xTaskCreate(periodic_task, "timer_periodic_task", 1024, NULL, 5, &handle_periodic_task);
	if (res != pdPASS)
	{
		// TODO: Define error policy
		while(1);
	}
	app_timer_init((timer_idx_t) TIMER_0, WITH_RELOAD, TIMER_INTERVAL0_USEC, &handle_periodic_task);
}
