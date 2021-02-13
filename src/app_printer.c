/*=============================================================================
 * Author: ctrinidad
 * Date: 2020/09/27
 *===========================================================================*/

#include <string.h>
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

#define BUF_SIZE (256)

// Printer commands
const char GET_STATUS[]   = {0x01, 'A', 0x0D}; 		// Status ASCII string request: <SOH>A/r
const char BEGIN_LABEL[]  = {0x02, 'L', 0x0D}; 		// Begin label: <STX>L/r
const char METRIC_MODE[]  = {'m', 0x0D}; 			// Set Metric Mode
const char HEAT_SETTING[] = {'H', '1', '5', 0x0D}; 	// Enter Heat Setting
const char DOT_SIZE[]     = {'D', '1', '1', 0x0D}; 	// Set Dot Size Width and Height
const char END_LABEL[]    = {'E', 0x0D}; 			// Terminate Label Formatting Mode and Print Label
const char ALIGN_CENTER[] = {'J', 'C', 0x0D}; 	    // Justification: Center justified
const char ALIGN_LEFT[]   = {'J', 'L', 0x0D}; 	    // Justification: Left justified

const char TEST_PASS_MSG[] = "141100001200087APROBADO\r";    // Font4, y=12mm, x=8.7mm
const char TEST_FAIL_MSG[] = "141100001200077RECHAZADO\r";   // Font4, y=12mm, x=7.7mm
const char LOTE_MSG[]      = "131100000600020LOTE: ";        // Font3, y=6mm, x=2mm
const char SPARE_MSG[]     = "131100000100020ALGO MAS?? \r"; // Font3, y=1mm, x=2mm

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static uint8_t uartBuffer[BUF_SIZE];

/*=====[Definitions of internal functions]===================================*/

// Send ASCII Status String: <SOH>A
// Printer Response: abcdefgh<CR>
static printerStatus_t getPrinterStatus(void)
{
	printerStatus_t printerStatus;
	int len;

	uart_write_bytes(UART_NUM_2, GET_STATUS, sizeof(GET_STATUS));
	len = uart_read_bytes(UART_NUM_2, uartBuffer, BUF_SIZE, 5000 / portTICK_RATE_MS);
	if (len == 0) {
		printerStatus = PRINTER_NO_COMM;
	}
	else {
		if (uartBuffer[0] == 'N' && uartBuffer[1] == 'N' && uartBuffer[2] == 'N' &&
			uartBuffer[3] == 'N' && uartBuffer[4] == 'N' && uartBuffer[5] == 'N' &&
			uartBuffer[6] == 'N' && uartBuffer[7] == 'N')
			printerStatus = PRINTER_READY;
		else
			printerStatus = PRINTER_NOT_READY;
	}

	return printerStatus;
}

static printerStatus_t printTestStatus(printer_msg_t printer_msg, const char* lote)
{
	int len;

	// Begin label: <STX>L/r
	uart_write_bytes(UART_NUM_2, BEGIN_LABEL, sizeof(BEGIN_LABEL));

	// Set Metric Mode
	uart_write_bytes(UART_NUM_2, METRIC_MODE, sizeof(METRIC_MODE));

	// Enter Heat Setting
	uart_write_bytes(UART_NUM_2, HEAT_SETTING, sizeof(HEAT_SETTING));

	// Set Dot Size Width and Height
	uart_write_bytes(UART_NUM_2, DOT_SIZE, sizeof(DOT_SIZE));

	// Test Status
	if (printer_msg == TEST_PASS) {
		uart_write_bytes(UART_NUM_2, TEST_PASS_MSG, sizeof(TEST_PASS_MSG));
	}
	else {
		uart_write_bytes(UART_NUM_2, TEST_FAIL_MSG, sizeof(TEST_FAIL_MSG));
	}

	// Lote
	for (int i=0;i<BUF_SIZE;i++) *(uartBuffer+i)=0;
	strcpy((char*) uartBuffer, (char*) LOTE_MSG);
	strcat((char*) uartBuffer, (char*) lote);
	*(uartBuffer+strlen((char*) uartBuffer))='\r';
	len = strlen((char*) uartBuffer);
	uart_write_bytes(UART_NUM_2, (const char *) uartBuffer, len);

	// Spare
	uart_write_bytes(UART_NUM_2, SPARE_MSG, sizeof(SPARE_MSG));

	// Terminate Label Formatting Mode and Print Label
	uart_write_bytes(UART_NUM_2, END_LABEL, sizeof(END_LABEL));

	return PRINTER_OK;
}

static void appUartInit(void)
{
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
}

/*=====[Definitions of external functions]===================================*/

printerStatus_t print(printer_msg_t printer_msg, const char* lote)
{
	printerStatus_t printerStatus;

	// Ask for Printer Status String
	printerStatus = getPrinterStatus();

	if (printerStatus == PRINTER_READY) {
		printTestStatus(printer_msg, lote);
		printerStatus = PRINTER_OK;
	}

	return printerStatus;
}

void appPrinter(void)
{
    appUartInit();
}
