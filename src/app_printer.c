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
#include "sapi_convert.h"

/*=====[Definition of private macros, constants or data types]===============*/

#define UART_TX  (GPIO_NUM_17)
#define UART_RX  (GPIO_NUM_16)
#define UART_RTS  (UART_PIN_NO_CHANGE)
#define UART_CTS  (UART_PIN_NO_CHANGE)

#define BUF_SIZE (256)

// Printer commands
const char GET_STATUS[]   = {0x01, 'A', 0x0D}; 		// Status ASCII string request: <SOH>A/r
const char RESET_PRINTER[]= {0x01, '*', 0x0D}; 		// The printer will reset: <SOH>#/r
const char BEGIN_LABEL[]  = {0x02, 'L', 0x0D}; 		// Begin label: <STX>L/r
const char METRIC_MODE[]  = {'m', 0x0D}; 			// Set Metric Mode
const char HEAT_SETTING[] = {'H', '1', '5', 0x0D}; 	// Enter Heat Setting
const char DOT_SIZE[]     = {'D', '1', '1', 0x0D}; 	// Set Dot Size Width and Height
const char END_LABEL[]    = {'E', 0x0D}; 			// Terminate Label Formatting Mode and Print Label
const char ALIGN_CENTER[] = {'J', 'C', 0x0D}; 	    // Justification: Center justified
const char ALIGN_LEFT[]   = {'J', 'L', 0x0D}; 	    // Justification: Left justified

const char TEST_FAIL_MSG[] = "131100001400077RECHAZADO\r";   // Font3, y=14mm, x=7.7mm
const char LOTE_MSG_FAIL[] = "121100001100040LOTE: ";        // Font2, y=11mm, x=4mm
const char IP_MSG_FAIL[]   = "121100000810040IP = 120mA     OK \r"; 	// Font2, y=8.1mm, x=4mm
const char VS_MSG_FAIL[]   = "121100000510040VS = 16.6V  **FALLO** \r"; // Font2, y=5.1mm, x=4mm
const char IS_MSG_FAIL[]   = "121100000220040IS = 200mA     OK \r"; 	// Font2, y=2.2mm, x=4mm
const char VP_MSG_FAIL[]   = "111100000000040VP = 230V \r"; 			// Font1, y=0mm, x=4mm

const char TEST_PASS_MSG[] = "161100000250030OK\r";    // Font6, y=2.5mm, x=3mm
const char LOTE_MSG_PASS[] = "131100001300040LOTE: ";  // Font3, y=13mm, x=4mm
const char VS_MSG_PASS[]   = "171100000800140VS ";     // Font7, y=8mm, x=14mm
const char IP_MSG_PASS[]   = "171100000400140IP "; 	   // Font7, y=4mm, x=14mm
const char IS_MSG_PASS[]   = "171100000000140IS "; 	   // Font7, y=0mm, x=14mm

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static uint8_t uartBuffer[BUF_SIZE];
static QueueHandle_t uart0_queue;

/*=====[Definitions of internal functions]===================================*/

// Send ASCII Status String: <SOH>A
// Printer Response: abcdefgh<CR>
static printerStatus_t getPrinterStatus(void)
{
	uart_event_t event;

	printerStatus_t printerStatus;
	int len;

	uart_flush(UART_NUM_2);
	xQueueReset( uart0_queue );
	// Get the printer status
	uart_write_bytes(UART_NUM_2, GET_STATUS, sizeof(GET_STATUS));
	xQueueReceive(uart0_queue, (void * )&event, (portTickType) 60000 / portTICK_RATE_MS); // Wait until 1 minute
	if(event.type == UART_DATA) {
		len = uart_read_bytes(UART_NUM_2, uartBuffer, BUF_SIZE, 0);
	}
	else {
		len = 0;
	}
	if (len == 0) {
		printerStatus = PRINTER_NO_COMM;
	}
	else {
		if (uartBuffer[0] == 'N' && uartBuffer[1] == 'N' && uartBuffer[2] == 'N' &&
			uartBuffer[3] == 'N' && uartBuffer[4] == 'N' && uartBuffer[5] == 'N' &&
			uartBuffer[6] == 'N' && uartBuffer[7] == 'N')
			printerStatus = PRINTER_READY;
		else {
			printerStatus = PRINTER_NOT_READY;
		}
	}

	return printerStatus;
}

static printerStatus_t printTestStatus(test_status_t *test_status, const char* lote)
{
	int len;
	char aux[10];

	// Begin label: <STX>L/r
	uart_write_bytes(UART_NUM_2, BEGIN_LABEL, sizeof(BEGIN_LABEL));

	// Set Metric Mode
	uart_write_bytes(UART_NUM_2, METRIC_MODE, sizeof(METRIC_MODE));

	// Enter Heat Setting
	uart_write_bytes(UART_NUM_2, HEAT_SETTING, sizeof(HEAT_SETTING));

	// Set Dot Size Width and Height
	uart_write_bytes(UART_NUM_2, DOT_SIZE, sizeof(DOT_SIZE));

	// Test Status
	if (test_status->test_result == TEST_PASS) {
		uart_write_bytes(UART_NUM_2, TEST_PASS_MSG, sizeof(TEST_PASS_MSG));

		// Lote
		for (int i=0;i<BUF_SIZE;i++) *(uartBuffer+i)=0;
		strcpy((char*) uartBuffer, (char*) LOTE_MSG_PASS);
		strcat((char*) uartBuffer, (char*) lote);
		*(uartBuffer+strlen((char*) uartBuffer))='\r';
		len = strlen((char*) uartBuffer);
		uart_write_bytes(UART_NUM_2, (const char *) uartBuffer, len);

		// RMS values
		// VS
		for (int i=0;i<BUF_SIZE;i++) *(uartBuffer+i)=0;
		strcpy((char*) uartBuffer, (char*) VS_MSG_PASS);
		sprintf(aux, "%d", test_status->rms.Vs/100);
		strcat((char*) uartBuffer, (char*) aux);
		strcat((char*) uartBuffer, (char*) "V\r");
		len = strlen((char*) uartBuffer);
		uart_write_bytes(UART_NUM_2, (const char *) uartBuffer, len);
		// IP
		for (int i=0;i<BUF_SIZE;i++) *(uartBuffer+i)=0;
		strcpy((char*) uartBuffer, (char*) IP_MSG_PASS);
		sprintf(aux, "%d", test_status->rms.Ip);
		strcat((char*) uartBuffer, (char*) aux);
		strcat((char*) uartBuffer, (char*) "mA\r");
		len = strlen((char*) uartBuffer);
		uart_write_bytes(UART_NUM_2, (const char *) uartBuffer, len);
		// IS
		for (int i=0;i<BUF_SIZE;i++) *(uartBuffer+i)=0;
		strcpy((char*) uartBuffer, (char*) IS_MSG_PASS);
		sprintf(aux, "%d", test_status->rms.Is);
		strcat((char*) uartBuffer, (char*) aux);
		strcat((char*) uartBuffer, (char*) "mA\r");
		len = strlen((char*) uartBuffer);
		uart_write_bytes(UART_NUM_2, (const char *) uartBuffer, len);
	}
	else {
		uart_write_bytes(UART_NUM_2, TEST_FAIL_MSG, sizeof(TEST_FAIL_MSG));
		// Lote
		for (int i=0;i<BUF_SIZE;i++) *(uartBuffer+i)=0;
		strcpy((char*) uartBuffer, (char*) LOTE_MSG_FAIL);
		strcat((char*) uartBuffer, (char*) lote);
		*(uartBuffer+strlen((char*) uartBuffer))='\r';
		len = strlen((char*) uartBuffer);
		uart_write_bytes(UART_NUM_2, (const char *) uartBuffer, len);
		// RMS values
		uart_write_bytes(UART_NUM_2, VP_MSG_FAIL, sizeof(VP_MSG_FAIL));
		uart_write_bytes(UART_NUM_2, IP_MSG_FAIL, sizeof(IP_MSG_FAIL));
		uart_write_bytes(UART_NUM_2, VS_MSG_FAIL, sizeof(VS_MSG_FAIL));
		uart_write_bytes(UART_NUM_2, IS_MSG_FAIL, sizeof(IS_MSG_FAIL));
	}

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
    uart_driver_install(UART_NUM_2, BUF_SIZE, 0, 20, &uart0_queue, 0);
}

/*=====[Definitions of external functions]===================================*/

printerStatus_t print(test_status_t *test_status, const char* lote)
{
	printerStatus_t printerStatus;

	// Ask for Printer Status String
	printerStatus = getPrinterStatus();

	if (printerStatus == PRINTER_READY) {
		printTestStatus(test_status, lote);
		printerStatus = PRINTER_OK;
	}

	return printerStatus;
}

void appPrinter(void)
{
    appUartInit();
}
