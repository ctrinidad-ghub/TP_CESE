/* Copyright 2017, Eric Pernia.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Date: 2017-12-05 */

#ifndef _SAPI_LCD_H_
#define _SAPI_LCD_H_

/*==================[inclusions]=============================================*/

#include "stdint.h"

/*==================[c++]====================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

// commands
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFT          0x01
#define LCD_ENTRYNOSHIFT        0x00

// flags for display on/off control
#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

// flags for function set
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

// LCD delay Times
#define LCD_EN_PULSE_WAIT_US   25    // 25 us
#define LCD_LOW_WAIT_US        25    // 25 us
#define LCD_HIGH_WAIT_US       100   // 100 us
#define LCD_CMD_WAIT_US        110   // Wait time for every command 45 us, except:
#define LCD_CLR_DISP_WAIT_MS   3     // - Clear Display 1.52 ms
#define LCD_RET_HOME_WAIT_MS   3     // - Return Home  1.52 ms
#define LCD_RBUSY_ADDR_WAIT_US 0     // - Read Busy flag and address 0 us
#define LCD_STARTUP_WAIT_MS    1000   // Wait for more than 40 ms after VCC rises to 2.7 V

/*==================[typedef]================================================*/

typedef enum {
    LCD_SET_4BITMODE_2LINE = LCD_4BITMODE | LCD_2LINE,
	LCD_SET_8BITMODE_2LINE = LCD_8BITMODE | LCD_2LINE,
	LCD_SET_4BITMODE_1LINE_5x10DOTS = LCD_4BITMODE | LCD_1LINE | LCD_5x10DOTS,
	LCD_SET_8BITMODE_1LINE_5x10DOTS = LCD_8BITMODE | LCD_1LINE | LCD_5x10DOTS,
	LCD_SET_4BITMODE_1LINE_5x8DOTS = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS,
	LCD_SET_8BITMODE_1LINE_5x8DOTS = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS,
} lcd_set_t;

typedef enum {
    LCD_CONTROL_CURSORON_BLINKON = LCD_CURSORON | LCD_BLINKON,
	LCD_CONTROL_CURSOROFF_BLINKON = LCD_CURSOROFF | LCD_BLINKON,
	LCD_CONTROL_CURSORON_BLINKOFF = LCD_CURSORON | LCD_BLINKOFF,
	LCD_CONTROL_CURSOROFF_BLINKOFF = LCD_CURSOROFF | LCD_BLINKOFF,
} lcd_control_t;

typedef enum {
    LCD_ENTRY_MODE_RIGHT = LCD_ENTRYRIGHT,
	LCD_ENTRY_MODE_LEFT = LCD_ENTRYLEFT,
} lcd_entry_mode_t;

typedef struct {
	uint16_t lineWidth;
	uint16_t amountOfLines;
	lcd_set_t lcd_set;
	lcd_control_t lcd_control;
	lcd_entry_mode_t lcd_entry_mode;
} lcd_config_t;

// This enumeration defines the available cursor modes
typedef enum{
   LCD_CURSOR_OFF      = 0x00,
   LCD_CURSOR_ON       = 0x02,
   LCD_CURSOR_ON_BLINK = 0x03
} lcdCursorModes_t;

typedef struct{
   const uint8_t address;   // Custom character address
   const uint8_t bitmap[8]; // Custom character bitmap
} lcdCustomChar_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

// BIBLIOTECA NO REENTRANTE, CUIDADO CON EL RTOS!!!

void lcdInit( lcd_config_t *lcd_config );
void lcdCommand( uint8_t cmd );
void lcdData( uint8_t data );

void lcdGoToXY( uint8_t x, uint8_t y );
void lcdClear( void );
void lcdCursorSet( lcdCursorModes_t mode );
void lcdCreateChar( uint8_t charnum, const uint8_t* chardata );

void lcdCreateCustomChar( lcdCustomChar_t* customChar );
void lcdSendCustomChar( lcdCustomChar_t* customChar );

void lcdClearAndHome( void );
void lcdClearLine( uint8_t line );
void lcdClearLineFrom( uint8_t line, uint8_t xFrom );
void lcdClearLineFromTo( uint8_t line, uint8_t xFrom, uint8_t xTo );
void lcdSendStringRaw( char* str );

void lcdSendEnter( void );
void lcdSendChar( char character );
void lcdSendCustomCharByIndex( uint8_t charIndex );

void lcdSendString( const char* str );
void lcdSendStringClearLine( char* str );
void lcdSendStringFormXY( char* str, uint8_t x, uint8_t y );
void lcdSendStringFormXYClearLine( char* str, uint8_t x, uint8_t y );

void lcdSendInt( int64_t value );
void lcdSendIntClearLine( int64_t value );
void lcdSendIntFormXY( int64_t value, uint8_t x, uint8_t y );
void lcdSendIntFormXYClearLine( int64_t value, uint8_t x, uint8_t y );
void lcdSendIntFixedDigit( int64_t value, uint8_t dig, int8_t dot );

#ifdef SUPPORT_FLOAT
void lcdSendFloat( float value, uint32_t decDigits );
void lcdSendFloatClearLine( float value, uint32_t decDigits );
void lcdSendFloatFormXY( float value, uint32_t decDigits, uint8_t x, uint8_t y );
void lcdSendFloatFormXYClearLine( float value, uint32_t decDigits, uint8_t x, uint8_t y );
#endif

#define lcdSendStringLn(str)   lcdSendString(str); \
                               lcdSendEnter()

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _SAPI_LCD_H_ */
