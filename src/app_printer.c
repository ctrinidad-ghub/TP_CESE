/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "../inc/app_printer.h"

/*=====[Definition of private macros, constants or data types]===============*/

#define UART_TX  (GPIO_NUM_17)
#define UART_RX  (GPIO_NUM_16)
#define UART_RTS  (UART_PIN_NO_CHANGE)
#define UART_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (1024)

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

uint8_t uartBuffer[BUF_SIZE];

int len;

QueueHandle_t printer_queue;

/*=====[Definitions of internal functions]===================================*/

static void appUart_task()
{
	printer_msg_t printer_msg;

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, UART_TX, UART_RX, UART_RTS, UART_CTS);
    uart_driver_install(UART_NUM_2, BUF_SIZE, 0, 0, NULL, 0);

    vTaskDelay(200 / portTICK_PERIOD_MS);

    while (1) {
        /*// Read data from the UART
        len = uart_read_bytes(UART_NUM_2, data, BUF_SIZE, 20 / portTICK_RATE_MS);

		if (len != 0) {

		}*/
    	xQueueReceive(printer_queue, &printer_msg, portMAX_DELAY);
    	// Send ASCII Status String: <SOH>A
    	// Printer Response: abcdefgh<CR>
    	uartBuffer[0]= 0x01;
    	uartBuffer[1]= 0x41;
    	uartBuffer[2]= 0x0D;
    	len = 3;
    	uart_write_bytes(UART_NUM_2, (const char *) uartBuffer, len);
    	len = uart_read_bytes(UART_NUM_2, uartBuffer, BUF_SIZE, 5000 / portTICK_RATE_MS);

    }
}

/*=====[Definitions of external functions]===================================*/

printerStatus_t print(printer_msg_t printer_msg)
{
	xQueueSend( printer_queue, ( void * ) &printer_msg, ( TickType_t ) 0 );
	return PRINT_OK;
}

void appPrinter(void)
{
	printer_queue = xQueueCreate(1, sizeof(printer_msg_t));

    xTaskCreate(appUart_task, "appUart_task", 1024, NULL, 10, NULL);
}
