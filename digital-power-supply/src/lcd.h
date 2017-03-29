#ifndef LCD_H
#define LCD_H
/*-------------------------------------------------------------------------------------------------
  LCD Library (44780 compatible).

  No usa el pin R/W, ya que normalmente se lo conecta a la masa del circuito.

  Fecha de creación: 8/04/2009
  Autor: Felixls
  Web: http://sergiols.blogspot.com
  Changelog:
  Fecha   Versión     Observaciones
  8/4/09  1.00        Versión inicial

  Frecuencia de reloj: 4MHZ
  Compilador: SDCC 2.9.1

  -------------------------------------------------------------------------------------------------*/

/*  PORTB:4  ----->  LCD bit 4           */
/*  PORTB:5  ----->  LCD bit 5           */
/*  PORTB:6  ----->  LCD bit 6           */
/*  PORTB:7  ----->  LCD bit 7           */
/*  PORTB:1  ----->  LCD RS              */
/*  PORTB:3  ----->  LCD E               */

#define LCD_DATA                PORTB // Puerto de datos
#define LCD_DATA_TRIS           TRISB // Control del puerto de datos
#define LCD_RS                  RB2   // Modo
#define LCD_ENABLE              RB3   // Habilitación/deshab. en envíos de datos al lcd.

#define LCD_CLEAR                0x01 // Clear Display
#define LCD_HOME                 0x02 // Cursor a Home
#define LCD_NORMAL               0x06 // Cursor en modo incrementar.
#define LCD_REV                  0x04 // Normal-reverse
#define LCD_SCROLL               0x07 // Usa scroll
#define LCD_SCROLL_REV           0x05 // Reverse
#define LCD_D8_BIT               0x38 // 8 bit 2 lineas ( 5x7 font )
#define LCD_D4_BIT_CONF          0x20 // 4 bit
#define LCD_D4_BIT               0x28 // 4 bit 2 lineas ( 5x7 font )
#define LCD_RESET                0x30 // Reset
#define LCD_DIS_ON               0x0C // Display on modo 2 lineas
#define LCD_DIS_OFF              0x08 // Display off
#define LCD_LINEA1               0x80 // Linea 1 posicion 1
#define LCD_LINEA2               0xC0 // Linea 2 posicion 1
#define LCD_CURSOR_ON            0x0E // Cursor on
#define LCD_CURSOR_OFF           0x0C // Cursor off
#define LCD_BLINK_ON             0x0F // Cursor blink
#define LCD_CURSOR_DER           0x14 // Mover cursor derecha
#define LCD_CURSOR_IZQ           0x10 // Mover cursor izquierda
#define LCD_DISPLAY__DER         0x1C // Scroll display derecha
#define LCD_DISPLAY__IZQ         0x18 // Scroll display izquierda
#define LCD_CHARMODE             0x01
#define LCD_COMMANDMODE          0x00

#include "delay.h"

void lcd_init (void);
void lcd_send( char mode, char dato );
void lcd_message ( char * mess );

void lcd_send_quartet( char mode, char dato );
void lcd_send( char mode, char dato );
void lcd_init (void);
void lcd_message ( char * mess );
char lcd_hexa(char a);

#endif
