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
 * -----------------------------------------------------------------------------------------------
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "global.h"
#include "uart.h"
#include "usb.h"
#include "ir.h"

global_t global;

/*
 * Inicializa todos los pines
 */
static inline void hardware_init(void)
{
	PORTB = 0xFF; /* activa todos los pull-ups menos para los leds*/
	cbi(PORTB, IROUT);
	cbi(PORTB, LED_IR_RECEIVED);
	DDRB = 0; /* todos los pines como entrada */

	PORTC = 0xFF; /* activa todos los pull-ups menos para los leds */
	cbi(PORTC, LED_PCREMOTE);
	cbi(PORTC, LED_CABLE);
	cbi(PORTC, LED_GAME);
	cbi(PORTC, LED_ONKYO);
	DDRC = 0; /* todos los pines como entrada */

	PORTD = 0xFF; /* activa todos los pull-ups menos para D+ y D- */
	cbi(PORTD, PIND2);
	cbi(PORTD, PIND4);
	DDRD = 0; /* todos los pines como entrada */

	/* IOs de leds como salida */
	sbi(DDRB, LED_IR_RECEIVED);
	sbi(DDRC, LED_PCREMOTE);
	sbi(DDRC, LED_CABLE);
	sbi(DDRC, LED_GAME);
	sbi(DDRC, LED_ONKYO);
}

int main(void)
{
	uint16_t i;

	wdt_enable(WDTO_1S);							// Iniciliza el perro guardián a 1 segundo.

	global.mode = MODE_NORMAL;

	hardware_init();
	ir_init(IR_PROTO_PCREMOTE);
	idle_timer_init();

	debug_init(); DBG1("\r\nUSBTIR - Felilxs\r\n");

	usb_init();
	usb_disconnect();								// fuerza la re-enumeración
	for (i = 0; i < 255; i++) 						// simula una desconección USB
	{
		wdt_reset();
		_delay_ms(1);
	}
	usb_connect();
	sei();											// habilita interrupciones

	DBG1("Ready.\r\n");

	while (1)
	{
		wdt_reset();								// resetea el perro guardián
		usb_poll();

#if IRDEBUG_LEVEL > 0
		if (bit_is_clear(PINC, PC0))				// pulsador en PC0 sirve para cambiar de modo
		{
			_delay_ms(20);
			if (bit_is_clear(PINC, PC0))
			{
				global.mode = 1 - global.mode;
				DBG1("MODE:"); printDec(global.mode); DBG1("\r\n");
				loop_until_bit_is_set(PINC, PC0);
			}
		}
		if (bit_is_clear(PINC, PC1))				// pulsador en PC1 sirve para cambiar de protocolo de control (target)
		{
			_delay_ms(20);
			if (bit_is_clear(PINC, PC1))
			{
				inc_protocol();
				loop_until_bit_is_set(PINC, PC1);
			}
		}
		if (global.mode == MODE_RECORD)				// en modo record se envía por RS232 la "trama" puera (tiempos de pulsos)
			ir_record();
#endif

		if (global.mode == MODE_NORMAL)
			ir_decode();

		if (TIFR & _BV(TOV0))
		{
			/* 22 ms timer */
			TIFR = 1 << TOV0;
			idle_timer();
		}
	}

	return 0;
}
