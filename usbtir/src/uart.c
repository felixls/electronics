/*
 * -----------------------------------------------------------------------------------------------
 * Title: USBTIR
 *
 * Author: Felixls - 2012
 *
 * Web: http://sergiols.blogspot.com
 *
 * Compiler: AVR-GCC 4.5.1
 *
 * Licence: Todos los contenidos por Felixls son licenciados por
 *          Creative Commons Reconocimiento-Compartir bajo la misma licencia 3.0 Unported License.
 *          http://creativecommons.org/licenses/by-sa/3.0/
 *
 * Description:
 * Transceptor de Infrarrojo por USB
 *
 * Este proyecto usa VID/PID de USB licenciado para uso EXCLUSIVO de laboratorio o uso personal!.
 *
 * Sources:
 * 			http://www.rentron.com/Infrared_Communication.htm
 * 			http://www.remotecentral.com/cgi-bin/mboard/rc-discrete/thread.cgi?5489
 * 			http://www.obdev.at/products/vusb/hidkeys.html
 *			https://sites.google.com/site/vamposdecampos2/infrahid
 *
 * Date: April 2012
 *
 * Interfaz de Debug por puerto serial, modificado del proyecto AVR library de Christian Starkjohann
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 *
 * -----------------------------------------------------------------------------------------------
 */
#include <stdlib.h>
#include "uart.h"

#if IRDEBUG_LEVEL > 0

#warning "Never compile production devices with debugging enabled"

static void uartPutc(char c)
{
	while (!(ODDBG_USR & (1 << ODDBG_UDRE)))
		; /* wait for data register empty */
	ODDBG_UDR = c;
}

static uchar hexAscii(uchar h)
{
	h &= 0xf;
	if (h >= 10)
		h += 'a' - (uchar) 10 - '0';
	h += '0';
	return h;
}

void printHex(uint8_t c)
{
	uartPutc(hexAscii(c >> 4));
	uartPutc(hexAscii(c));
}

void printDec(uint32_t value)
{
	char buffer[10];
	char *data = buffer;

	utoa(value, buffer, 10);
	while (*data)
		uartPutc(*data++);
}

void odDebug(const char *data)
{
	while (*data)
		uartPutc(*data++);
}

#endif
