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
 *
 */

/* Date: 2017-12-05 */

/*==================[inlcusiones]============================================*/

#include "sapi_lcd.h"
#include "lcdBoard.h"

/*==================[definiciones y macros]==================================*/

/*==================[definiciones de datos internos]=========================*/

typedef struct {
   uint16_t lineWidth;
   uint16_t amountOfLines;
   uint16_t charWidth;
   uint16_t charHeight;
   lcd_config_t lcd_config;
   uint8_t x;
   uint8_t y;
} lcd_t;

static lcd_t lcd;

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

static void lcdEnablePulse( void )
{
   lcdENSet( );       // EN = 1 for H-to-L pulse
   lcdDelay_us( LCD_EN_PULSE_WAIT_US );   // Wait to make EN wider //lcdDelay_us(1);
   lcdENClear( );      // EN = 0 for H-to-L pulse
}

void lcdCommandNibble( uint8_t cmd )
{
	lcdSendNibble( cmd & 0xF0 );          // Send high nibble to D7-D4

	lcdRSClear( );   // RS = 0 for command
	lcdRWClear( );   // RW = 0 for write

	lcdEnablePulse();
}

void lcdCommand( uint8_t cmd )
{
   if (lcd.lcd_config.lcd_set & LCD_8BITMODE) {
	   lcdSendPort( cmd );

	   lcdRSClear( );   // RS = 0 for command
	   lcdRWClear( );   // RW = 0 for write

	   lcdEnablePulse();
   } else
   {
	   lcdCommandNibble( cmd & 0xF0 );
	   lcdDelay_us( LCD_LOW_WAIT_US );       // Wait
	   lcdCommandNibble( cmd << 4 );
   }
}

void lcdData( uint8_t data )
{
   if (lcd.lcd_config.lcd_set & LCD_8BITMODE) {
	   lcdSendPort( data );

	   lcdRSSet( );    // RS = 1 for data
	   lcdRWClear( );   // RW = 0 for write

	   lcdEnablePulse();
   } else
   {
	   lcdSendNibble( data & 0xF0 );         // Send high nibble to D7-D4

	   lcdRSSet( );    // RS = 1 for data
	   lcdRWClear( );   // RW = 0 for write

	   lcdEnablePulse();

	   lcdSendNibble( data << 4 );           // Send low nibble to D7-D4
	   lcdEnablePulse();
   }
}

void initByInstruction(void) {
	if (lcd.lcd_config.lcd_set & LCD_8BITMODE) {
		lcdCommand( 0x30 );
	    lcdDelay_ms(5);
	    lcdCommand( 0x30 );
	    lcdDelay_us(150);
	    lcdCommand( 0x30 );
	    lcdDelay_us(40);
	}
	else {
		lcdCommandNibble( 0x30 );
	    lcdDelay_ms(5);
	    lcdCommandNibble( 0x30 );
	    lcdDelay_us(150);
	    lcdCommandNibble( 0x30 );
	    lcdDelay_us(40);
	    lcdCommandNibble( LCD_FUNCTIONSET ); // Solo en 4BIT
	    lcdCommandDelay();
	}
}

void lcdInit( lcd_config_t *lcd_config )
{
	lcd.lineWidth = lcd_config->lineWidth; // Characters
	lcd.amountOfLines = lcd_config->amountOfLines;
	lcd.lcd_config.lcd_control = lcd_config->lcd_control;
	lcd.lcd_config.lcd_set = lcd_config->lcd_set;
	lcd.lcd_config.lcd_entry_mode = lcd_config->lcd_entry_mode;
	lcd.x = 0;
	lcd.y = 0;

	lcdBoardInit(lcd.lcd_config.lcd_set);

	if (lcd.lcd_config.lcd_set & LCD_8BITMODE) {
		lcdSendPort( 0 ); // D7-0 = 0
	}
	else {
		lcdSendNibble( 0 ); // D7-4 = 0
	}

	lcdRWClear( );     // RW = 0
	lcdRSClear( );     // RS = 0
	lcdENClear( );     // EN = 0

	// required by display controller to allow power to stabilize
	lcdDelay_ms(LCD_STARTUP_WAIT_MS);

	// Initializing by Instruction
	// If the power supply conditions for correctly operating the internal reset circuit are not met,
	// initialization by instructions becomes necessary.
	initByInstruction();

	// Function set - 4/8 bit mode
    lcdCommand( LCD_FUNCTIONSET | (lcd_config->lcd_set & LCD_8BITMODE) );
    lcdCommandDelay();

    // Function set
    lcdCommand( LCD_FUNCTIONSET | lcd_config->lcd_set );
    lcdCommandDelay();

    // Display on/off control
    lcdCommand( LCD_DISPLAYCONTROL | LCD_DISPLAYON | lcd_config->lcd_control );
    lcdCommandDelay();

    // Clear display
    lcdClear( );

    // Entry mode set
    lcdCommand( LCD_ENTRYMODESET | lcd_config->lcd_entry_mode | LCD_ENTRYNOSHIFT );
    lcdCommandDelay();
}

void lcdGoToXY( uint8_t x, uint8_t y )
{
   if( x >= lcd.lineWidth || y >= lcd.amountOfLines ) {
      return;
   }
   uint8_t firstCharAdress = 0;

   switch(y){
	   case 0:
		   firstCharAdress = 0x0;
		   break;
	   case 1:
		   firstCharAdress = 0x40;
		   break;
	   case 2:
		   firstCharAdress = lcd.lineWidth;
		   break;
	   case 3:
		   firstCharAdress = 0x40 + lcd.lineWidth;
		   break;
   }

   lcdCommand( LCD_SETDDRAMADDR | (firstCharAdress + x) );
   lcdDelay_us( LCD_HIGH_WAIT_US );      // Wait
   lcd.x = x;
   lcd.y = y;
}

void lcdClear( void )
{
   lcdCommand( LCD_CLEARDISPLAY );       // Command 0x01 for clear LCD
   lcdDelay_ms(LCD_CLR_DISP_WAIT_MS);    // Wait
}

void lcdCursorSet( lcdCursorModes_t mode )
{
   lcdCommand( 0b00001100 | mode );
   lcdDelay_ms(LCD_CLR_DISP_WAIT_MS); // Wait
}

void lcdSendStringRaw( char* str )
{
   uint8_t i = 0;
   while( str[i] != 0 ) {
      lcdData( str[i] );
      i++;
   }
}

void lcdCreateChar( uint8_t charnum, const uint8_t* chardata )
{
   uint8_t i;
   charnum &= 0x07;
   lcdCommand( LCD_SETCGRAMADDR | (charnum << 3) );
   for (i = 0; i < 8; i++) {
      lcdData( chardata[i] );
   }
   //delay(1);
   lcdGoToXY( lcd.x, lcd.y );
}

void lcdCreateCustomChar( lcdCustomChar_t* customChar )
{
   lcdCreateChar( customChar->address, customChar->bitmap );
}

void lcdSendCustomChar( lcdCustomChar_t* customChar )
{
   lcdSendCustomCharByIndex( customChar->address );
}


void lcdClearAndHome( void )
{
   lcdClear();
   lcdGoToXY( 0, 0 ); // Poner cursor en 0, 0
   //delay(100);
}

void lcdClearLine( uint8_t line )
{
   lcdClearLineFromTo( line, 0, lcd.lineWidth - 1 );
}

void lcdClearLineFrom( uint8_t line, uint8_t xFrom )
{
   lcdClearLineFromTo( line, xFrom, lcd.lineWidth - 1 );
}

void lcdClearLineFromTo( uint8_t line, uint8_t xFrom, uint8_t xTo )
{
   uint8_t i = 0;

   if( xFrom >= lcd.lineWidth || line >= lcd.amountOfLines ) {
      return;
   }
   if( xFrom > xTo ) {
      return;
   }
   if( xTo >= lcd.lineWidth ) {
      xTo = lcd.lineWidth - 1;
   }

   lcdGoToXY( xFrom, line );
   for( i=xFrom; i<=xTo; i++ ) {
      lcdSendChar( ' ' );
   }
   //lcd.x--;
   lcdGoToXY( xFrom, line );
}

void lcdSendChar( char character )
{
   if( character == '\r' ) {        // Ignore '\r'
   } else if( character == '\n' ) { // Mando enter
      lcdSendEnter();
   } else {
      // Si se extiende en ancho mando enter
      if( lcd.x >= lcd.lineWidth ) {
         lcdSendEnter();
      }
      // Mando el caracter
      lcdData( character );
      lcdDataDelay();
      lcd.x++;
   }
}

void lcdSendCustomCharByIndex( uint8_t charIndex )
{
   // Si se extiende en ancho mando enter
   if( lcd.x >= lcd.lineWidth ) {
      lcdSendEnter();
   }
   // Mando el caracter
   lcdData( charIndex );
   lcd.x++;
}

void lcdSendEnter( void )
{
   // Si llego abajo no hace nada
   if( lcd.y >= lcd.amountOfLines ) {
      return;
   } else {
      lcd.x = 0;
      lcd.y++;
      lcdGoToXY( lcd.x, lcd.y );
   }
}

void lcdSendStringClearLine( char* str )
{
   lcdSendString( str );
   lcdClearLineFrom( lcd.y, lcd.x );
}

void lcdSendString( const char* str )
{
   uint32_t i = 0;
   while( str[i] != 0 ) {
      lcdSendChar( str[i] );
      i++;
   }
}

void lcdSendStringFormXY( char* str, uint8_t x, uint8_t y )
{
   lcdGoToXY( x, y );
   lcdSendString( str );
}

void lcdSendStringFormXYClearLine( char* str, uint8_t x, uint8_t y )
{
   lcdSendStringFormXY( str, x, y );
   lcdClearLineFrom( lcd.y, lcd.x );
}

void lcdSendInt( int64_t value )
{
   lcdSendString( intToStringGlobal(value) );
}

char lcdSendIntFixedDigit( int64_t value, uint8_t dig, uint8_t dot )
{
   int8_t i,dig_r;
   char valueChar[10];
   char lcdBuff[10];

   if (dot >= dig) return -1;
   if (value < 0) return -1;

   int64ToString( value, valueChar, 10 );
   for (dig_r=0;dig_r<dig;dig_r++)
   {
	   if (*(valueChar+dig_r) == 0) break;
   }

   if ((dig_r > dig) || ((dig_r > (dig-1)) && dot > 0)) return -1;

   if (dot == 0) dot = dig;
   else dot = dig - dot - 1;

   for (i=0;i<dig;i++) lcdBuff[i] = ' ';

   for (i=0;i<dig;i++) {
	   if ((dig-i-1)<=(dig_r-1)) {
		   if (dot==i) {
            lcdBuff[i] = '.';
            lcdBuff[i-1] = valueChar[dig_r-dig+i];
         }
         else if (i<dot) {
            if (dot==dig) lcdBuff[i] = valueChar[dig_r-dig+i];
            else lcdBuff[i-1] = valueChar[dig_r-dig+i];
         }
		   else if (i>dot) lcdBuff[i] = valueChar[dig_r-dig+i];
	   }
	   if ((dig-i-1)>(dig_r-1) && i>=dot) {
		   if (dot==i){
            lcdBuff[i] = '.';
            if (i!=0) lcdBuff[i-1] = '0';
         }
		   else lcdBuff[i] = '0';
	   }
   }

   for(i=0;i<dig;i++) lcdSendChar(lcdBuff[i]);

   return 0;
}

void lcdSendIntClearLine( int64_t value )
{
   lcdSendInt( value );
   lcdClearLineFrom( lcd.y, lcd.x );
}

void lcdSendIntFormXY( int64_t value, uint8_t x, uint8_t y )
{
   lcdGoToXY( x, y );
   lcdSendInt( value );
}

void lcdSendIntFormXYClearLine( int64_t value, uint8_t x, uint8_t y )
{
   lcdSendIntFormXY( value, x, y );
   lcdClearLineFrom( lcd.y, lcd.x );
}

#ifdef SUPPORT_FLOAT
void lcdSendFloat( float value, uint32_t decDigits )
{
   lcdSendString( floatToStringGlobal(value, decDigits) );
}

void lcdSendFloatClearLine( float value, uint32_t decDigits )
{
   lcdSendString( floatToStringGlobal(value, decDigits) );
   lcdClearLineFrom( lcd.y, lcd.x );
}

void lcdSendFloatFormXY( float value, uint32_t decDigits, uint8_t x, uint8_t y )
{
   lcdGoToXY( x, y );
   lcdSendFloat( value, decDigits );
}

void lcdSendFloatFormXYClearLine( float value, uint32_t decDigits, uint8_t x, uint8_t y )
{
   lcdSendFloatFormXY( value, decDigits, x, y );
   lcdClearLineFrom( lcd.y, lcd.x );
}
#else

#endif

/*==================[fin del archivo]========================================*/
