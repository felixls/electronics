//-------------------------------------------------------------------------------------------------
// LCD.C
//
// Autor: Felixls
// Web: http://sergiols.blogspot.com
//
// Rutinas para el manejo de un display Hitachi 44780 compatible.
//-------------------------------------------------------------------------------------------------

#include <pic/pic16f877a.h>
#include "lcd.h"

void lcd_init (void);
void lcd_send( char mode, char dato );
void lcd_message ( char * mess );

void lcd_send_quartet( char mode, char dato )
{
    LCD_RS = mode;

    PORTB = dato  | (PORTB & 0x0f) ;

    LCD_ENABLE = 1;
    _asm                              // delay de 4탎
    nop
    nop
    nop
    nop
     _endasm;
    LCD_ENABLE = 0;

}
void lcd_send( char mode, char dato )
{
    unsigned int j;
    char dat = dato;

    LCD_RS = mode;

    dat = dato & 0xf0;
    PORTB = dat | (PORTB & 0x0f) ;

    LCD_ENABLE = 1;
    _asm                              // delay de 4탎
    nop
    nop
    nop
    nop
     _endasm;
    LCD_ENABLE = 0;

    dat = ((dato<<4)& 0xf0);
    PORTB = dat | (PORTB & 0x0f) ;

    LCD_ENABLE = 1;
    _asm                              // delay de 4탎
    nop
    nop
    nop
    nop
     _endasm;
    LCD_ENABLE = 0;

    for (j=0; j<30; j++)             // delay de 41탎
    {
       _asm
       nop
       _endasm;
    }
}


void lcd_init (void)
{
  LCD_DATA_TRIS &= 0x03;
	//LCD_DATA = 0x00;

  delay_ms(15);

  lcd_send_quartet(LCD_COMMANDMODE, LCD_RESET);
  delay_ms(5);
  lcd_send_quartet(LCD_COMMANDMODE, LCD_RESET);
  delay_us(25);
  lcd_send_quartet(LCD_COMMANDMODE, LCD_RESET);
  delay_us(25);
  lcd_send_quartet(LCD_COMMANDMODE, LCD_D4_BIT_CONF);
  delay_us(25);

  lcd_send(LCD_COMMANDMODE, LCD_D4_BIT);
  delay_us(25);

  lcd_send(LCD_COMMANDMODE, LCD_CLEAR);
  delay_ms(2);
  lcd_send(LCD_COMMANDMODE, LCD_CURSOR_OFF);
  delay_ms(2);
  lcd_send(LCD_COMMANDMODE, LCD_NORMAL);
  delay_us(10);

  lcd_send(LCD_COMMANDMODE, LCD_DIS_ON);
  delay_us(10);
}

void lcd_message ( char * mess )
{
  while ( *mess )
  {
    lcd_send(LCD_CHARMODE, *mess ) ;
    mess++ ;
  }
}

char lcd_hexa(char a)
{
    if (a >9)
         a+=55;
    else
        a+=48;

    return a;
}
