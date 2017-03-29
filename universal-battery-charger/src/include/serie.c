/* Test to get Serial working.
   This version is to use hardware UART.  */
// This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include <pic/pic16f877a.h>
#include "delay.h"
#include "serie.h"

// Configuré pour:  9600 Bauds 8 bits sans parité 1bit de stop
void init_serie(void)
{
  // Configuration UARTtransmission serie
	// SPBRG - Baud Rate Generator Register

//	SPBRG = 25; // 4MHz => 9600 baud (BRGH = 1)
	SPBRG = 129; // 20MHz => 9600 baud (BRGH = 1)

	// BRGH - High Baud Rate Select Bit
	BRGH = 1; // (1 = high speed)
	// SYNC - USART Mode select Bit
	SYNC = 0; // (0 = asynchrone)
	// TRISC - Tri-state Data Direction Register for port C
	// RC6 - 6eme broche du port C - utilise pour Transmission serie
	// RC7 - 7eme broche du port C - utilise pour Reception serie
	TRISC6 = 0; // 0 = broche en sortie)
	TRISC7 = 1; // 1 = broche en entree)
		// SPEN - Serial Port Enable Bit 
  SPEN = 1; // 1 = l port serie valide
	// TXIE - USART Transmit Interupt Enable Bit
	TXIE = 0; // 1 = interruption transmission valide
	// RCIE - USART Receive Interupt Enable Bit
	RCIE = 0; // 1 = interruption reception valide
		// TX9 - 9-bit Transmit Enable Bit
	TX9 = 0; // 0 = 8-bit transmis)
		// RX9 - 9-bit Receive Enable Bit
	RX9 = 0; // 0 = 8-bit reception)
		// CREN - Continuous Receive Enable Bit
	CREN = 1; // 1 = reception valide
		// TXEN - Transmit Enable Bit
	TXEN = 1; // (1 = transmission valide
}

// *******************EMISSION
void putchar(char c)
{
  // envía una serie de caracteres en el enlace
	while(!TXIF); //TXIF=1=> TXREG vide
	TXREG = c;// Char TXREG en el lugar - se inicia la transmisión
}

//mostrar una cadena
void print(char *t)
{
  // texto completo de la atención / 0
  while (*t)
  {
    putchar(*t); // Data to transmit
    t++;
  }
}

void puthex(char nb)
{
  // un byte se convierte en 2 caracteres ASCII
  char n;
  n=(nb>>4)&0x0f;
  puth(n);
  n=nb&0x0f;
  puth(n);
}

void puth(char a)
{
  // convierte un medio de bytes en caracteres ASCII 1
  if (a >9)
    a+=55;
  else
    a+=48;
  putchar(a);
}

void putint(int num)
{
  // convierte un entero (16 bits) en 4 caracteres ASCII
  puthex(num>>8);
  puthex(num & 0xff);
}

// ************************RECEPTION 
char getchar(void)
{
  char a=0;
  while (!a)
    a=leccar();//expectativa de que el pleno RCREG
  return a;
}

char lirecar(void)
{
  // variante de la no definición de tiempo de bloqueo getchar
  int compt=0;
  char a=0;
  while ((!a) && (compt!=TEMPS))
  {
    //Espera RCREG carácter completo o no después de la ruptura
    compt++;
    a=leccar();
  }
  return a;
}

char leccar(void)
 {
   //lee un carácter si se dispone otra cosa devuelve 0
  if (OERR)
  {
    //error de recepción
		CREN=0; //se pone a 0 de CREN
		CREN =1; //reactivation de la réception CREN=1
	}
  if (RCIF)
    return RCREG;// carácter recibido
  else
    return 0;
}

char geth(void)
{
  char hex;
	hex=getchar();

	if ((hex>0x29) && (hex<0x3a))
    hex =hex & 0x0f;
	else
  {
		if (((hex>0x40) && (hex<0x47))||((hex>0x60) && (hex<0x67)))
      hex=(hex+0x09) &0x0f;
		else
      hex =0xff;
	}
	return hex;
}

char gethex(void)
{
  char hex;
  hex=geth()<<4;
  hex|=geth();
  return hex;
}

char getd(void)
{
  char dec;
	dec=getchar();
	if ((dec>0x29) && (dec<0x3a))
    dec&= 0x0f;
	else
    dec =0xff;
	return dec;
}


