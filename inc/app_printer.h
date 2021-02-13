/**
 * @file app_printer.h
 *
 * @brief
 *
 * @author Cristian Trinidad
 */

#ifndef _APP_PRINTER_H_
#define _APP_PRINTER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Message to print, used in print function
 * 
 */
typedef enum {
	TEST_PASS,
	TEST_FAIL,
} printer_msg_t;

/**
 * @brief Printer status
 * 
 */
typedef enum {
	PRINTER_NO_COMM, // Printer never respond
	PRINTER_NOT_READY,
	PRINTER_READY,
	PRINTER_OK
} printerStatus_t;

/**
 * @brief Initialize printer 
 * 
 */
void appPrinter( void );

/**
 * @brief Send printer_msg to the printer
 * 
 * @param printer_msg 
 * @return printerStatus_t 
 */
printerStatus_t print(printer_msg_t printer_msg, const char* lote);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_PRINTER_H_ */
