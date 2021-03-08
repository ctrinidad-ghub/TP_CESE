/**
 * @file app_printer.h
 *
 * @brief
 *
 * @author Cristian Trinidad
 */

#ifndef _APP_PRINTER_H_
#define _APP_PRINTER_H_

#include "../inc/test_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Printer status
 * 
 */
typedef enum {
	PRINTER_NO_COMM,   /*!<Printer never respond                               */
	PRINTER_NOT_READY, /*!<Printer is not ready, answer to command <SOH>A/r (Status request) */
	PRINTER_READY,     /*!<Printer ready to print                              */
	PRINTER_OK         /*!<The print command was successful                    */
} printerStatus_t;

/**
 * @brief Initialize printer 
 * 
 */
void appPrinter( void );

/**
 * @brief Send printer msg to the printer
 * 
 * @param test_status
 * @param lote
 * @return printerStatus_t 
 */
printerStatus_t print(test_status_t *test_status, const char* lote);

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _APP_PRINTER_H_ */
