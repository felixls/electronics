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
#include <util/delay.h>
#include "global.h"
#include "uart.h"
#include "usb.h"

static int8_t ir_proto;						// protocolo del control remoto
static int8_t ir_proto_control;				// protocolo del dispositivo a controlar (target)
static uint8_t ir_state; 		            // ḿaquina de estado
static uint8_t ir_bits = 0;					// bits leidos
static uint8_t ir_defbits;					// bits definidos para el protocolo
static uint32_t ir_value = 0; 	            // Decoded value
static uint16_t bit_mark;					// tiempo de pulso
static uint16_t one_space;					// tiempo para un 1
static uint16_t zero_space;					// tiempo para un 0
static volatile uint16_t irbuf[RAWBUF]; 	// buffer circular, almacena tiempos de pulsos y pausas
static volatile uint8_t irbuf_head = 0;		// puntero de inicio de datos dentro del buffer circular
static uint8_t irbuf_tail = 0;				// puntero de fin de datos
static uint8_t rc6tooglebit = 0;			// flag para alternar el bit de botón liberado (protocolo RC6)
static uint8_t playpause_state = 0;			// flag de play/pause (control PCREMOTE no tiene botón Pause)

#if IRDEBUG_LEVEL > 0
static uint16_t header_mark;				// tiempo de pulso del encabezado
static uint16_t header_space;				// tiempo de pausa del encabezado
static uint8_t period_state;				// máquina de estado para RC5
#endif

static struct code_entry					// mapeo de códigos PCREMOTE <-> HID keys
{
	uint16_t remote_code;
	uint8_t hid_code;
	uint8_t hid_code_mod;
} code_map[] = {
		{ BTNREMOTE_POWER, HIDKEY_P, HIDMOD_CONTROL_RIGHT | HIDMOD_ALT_RIGHT
				| HIDMOD_GUI_RIGHT },
		{ BTNREMOTE_EMAIL, HIDKEY_M, HIDMOD_ALT_RIGHT | HIDMOD_GUI_RIGHT }, {
				BTNREMOTE_WWW, HIDKEY_F7, HIDMOD_ALT_RIGHT }, {
				BTNREMOTE_CLOSE, HIDKEY_F4, HIDMOD_ALT_LEFT }, {
				BTNREMOTE_PREVTRACK, HIDKEY_ARROWLEFT, HIDMOD_GUI_LEFT
						| HIDMOD_CONTROL_LEFT }, {
				BTNREMOTE_PLAY, HIDKEY_P, HIDMOD_GUI_LEFT | HIDMOD_ALT_RIGHT },
		{ BTNREMOTE_NEXTTRACK, HIDKEY_ARROWLEFT, HIDMOD_GUI_RIGHT }, {
				BTNREMOTE_REWIND, HIDKEY_ARROWLEFT, HIDMOD_GUI_LEFT
						| HIDMOD_CONTROL_LEFT }, {
				BTNREMOTE_STOP, HIDKEY_SPACEBAR, HIDMOD_GUI_LEFT
						| HIDMOD_CONTROL_LEFT }, {
				BTNREMOTE_FORWARD, HIDKEY_ARROWRIGHT, HIDMOD_GUI_LEFT
						| HIDMOD_CONTROL_LEFT }, {
				BTNREMOTE_FULLSCR, HIDKEY_F11, 0 }, {
				BTNREMOTE_PAGEPLUS, HIDKEY_PAGEUP, 0 }, {
				BTNREMOTE_PAGEMINUS, HIDKEY_PAGEMINUS, 0 }, {
				BTNREMOTE_MUTE, HIDKEY_SPACEBAR, HIDMOD_GUI_LEFT
						| HIDMOD_ALT_LEFT }, {
				BTNREMOTE_MYPC, HIDKEY_E, HIDMOD_GUI_LEFT }, {
				BTNREMOTE_BACKSPACE, HIDKEY_BACKSPACE, 0 }, {
				BTNREMOTE_TAB, HIDKEY_TAB, 0 }, {
				BTNREMOTE_ARROWUP, HIDKEY_ARROWUP, 0 }, {
				BTNREMOTE_WINKEY, 0, HIDMOD_GUI_LEFT }, {
				BTNREMOTE_ARROWLEFT, HIDKEY_ARROWLEFT, 0 }, {
				BTNREMOTE_ENTER, HIDKEY_ENTER, 0 }, {
				BTNREMOTE_ARROWRIGHT, HIDKEY_ARROWRIGHT, 0 }, {
				BTNREMOTE_FOLDERS, HIDKEY_S, HIDMOD_GUI_LEFT }, {
				BTNREMOTE_ARROWDOWN, HIDKEY_ARROWDOWN, 0 }, {
				BTNREMOTE_ESC, HIDKEY_ESC, 0 },
		{ BTNREMOTE_NUMLOCK, HIDKEY_N, HIDMOD_ALT_RIGHT | HIDMOD_GUI_RIGHT }, {
				BTNREMOTE_ALTTAB, HIDKEY_TAB, HIDMOD_ALT_LEFT }, {
				BTNREMOTE_DESKTOP, HIDKEY_D, HIDMOD_GUI_LEFT }, };

#define NUM_CODES	(sizeof(code_map) / sizeof(struct code_entry))

/*
 * Establece el protocolo del control remoto
 * El código funciona con un control PCREMOTE, para usar otros
 * es necesario compilar el proyecto en IRDEBUG_LEVEL > 0 y
 * probar con los diferentes protocolos codificados hasta determinar de que tipo es.
 * En caso de tratarse de un protocolo no codificado aquí
 * usar el modo RECORD, tomar nota de los tiempos de pulsos,
 * determinar la trama típica y agregar el código necesario.
 */
static void set_protocol(uint8_t proto)
{
	ir_proto = proto;
	switch (ir_proto)
	{
		case IR_PROTO_PCREMOTE:
			bit_mark = PCREMOTE_BIT_MARK;
			one_space = PCREMOTE_ONE_SPACE;
			zero_space = PCREMOTE_ZERO_SPACE;
			ir_defbits = PCREMOTE_BITS;
			break;
#if IRDEBUG_LEVEL > 0
			case IR_PROTO_PHILIPS:
			bit_mark = PHILIPS_BIT_MARK;
			one_space = PHILIPS_ONE_SPACE;
			zero_space = PHILIPS_ZERO_SPACE;
			ir_defbits = PHILIPS_BITS;
			break;
			case IR_PROTO_ONKYO:
			bit_mark = ONKYO_BIT_MARK;
			one_space = ONKYO_ONE_SPACE;
			zero_space = ONKYO_ZERO_SPACE;
			ir_defbits = ONKYO_BITS;
			header_mark = ONKYO_HEADER_MARK;
			header_space = ONKYO_HEADER_SPACE;
			break;
			case IR_PROTO_CABLE:
			bit_mark = CABLE_BIT_MARK;
			one_space = CABLE_ONE_SPACE;
			zero_space = CABLE_ZERO_SPACE;
			ir_defbits = CABLE_BITS;
			header_mark = CABLE_HEADER_MARK;
			break;
			case IR_PROTO_LG:
			bit_mark = LG_BIT_MARK;
			one_space = LG_ONE_SPACE;
			zero_space = LG_ZERO_SPACE;
			ir_defbits = LG_BITS;
			header_mark = LG_HEADER_MARK;
			header_space = LG_HEADER_SPACE;
			break;
			case IR_PROTO_XBOX:
			bit_mark = XBOX_BIT_MARK;
			ir_defbits = XBOX_BITS;
			header_mark = XBOX_HEADER_MARK;
			header_space = XBOX_HEADER_SPACE;
			break;
#endif
	}
}

/*
 * Establece el protocolo del target
 */
static void set_proto_control(uint8_t proto)
{
	ir_proto_control = proto;
	cbi(PORTC, LED_CABLE);
	cbi(PORTC, LED_PCREMOTE);
	cbi(PORTC, LED_GAME);
	cbi(PORTC, LED_ONKYO);

	if (proto == IR_PROTO_CABLE)
		sbi(PORTC, LED_CABLE);
	else if (proto == IR_PROTO_PCREMOTE)
		sbi(PORTC, LED_PCREMOTE);
	else if (proto == IR_PROTO_XBOX)
		sbi(PORTC, LED_GAME);
	else if (proto == IR_PROTO_ONKYO)
		sbi(PORTC, LED_ONKYO);
}

/*
 * Configura la portadora de IR en la frecuencia de parámetro
 */
static inline void configIR(uint8_t khz)
{
	OCR2 = F_CPU / 2 / khz / 1000;
}

/*
 * Envía un pulso por IR
 */
static void ir_mark(int times)
{
	sbi(DDRB, IROUT);
	for (int i = 0; i < times; i++)
		_delay_us(4.5);
	cbi(DDRB, IROUT);
}

/*
 * "Envía" un espacio por IR
 */
static void ir_space(int times)
{
	cbi(DDRB, IROUT);
	for (int i = 0; i < times; i++)
		_delay_us(5.3);
}

/*
 * Verifica si el pulso o espacio obtenido está dentro de un rango aceptable
 * de acuerdo a lo definido.
 */
static uint8_t ir_check(const uint16_t ref, uint16_t value)
{
	short delta = ABS((short) value - (short) ref);

	if (delta > 100) /* aeps = 100us */
		return 0;
	if (delta > ref / 3) /* eps = 30%*/
		return 0;
	return 1;
}

/*
 * Iniciliza la máquina de estado
 */
static uint8_t ir_reset(void)
{
	ir_state = STATE_IDLE;
	ir_bits = 0;
	ir_value = 0;
	return ERR;
}

#if IRDEBUG_LEVEL > 0
/*
 * Envía un código usando RC5
 */
static void sendRC5(uint16_t code)
{
	configIR(36);

	code = code << 5;

	ir_space(PHILIPS_BIT_MARK);
	ir_mark(PHILIPS_BIT_MARK);
	ir_space(PHILIPS_BIT_MARK);
	ir_mark(PHILIPS_BIT_MARK);
	ir_space(PHILIPS_BIT_MARK);
	ir_mark(PHILIPS_BIT_MARK);

	for (uint8_t i = 0; i < 11; i++)
	{
		if (code & TOP_UINT16)
		{
			ir_space(PHILIPS_BIT_MARK);
			ir_mark(PHILIPS_BIT_MARK);
		}
		else
		{
			ir_mark(PHILIPS_BIT_MARK);
			ir_space(PHILIPS_BIT_MARK);
		}
		code <<= 1;
	}
}
#endif

/*
 * Envía un código IR usando protocolo NEC
 */
static void sendNEC(uint32_t code)
{
	configIR(38);

	ir_mark(ONKYO_HEADER_MARK);
	ir_space(ONKYO_HEADER_SPACE);

	for (uint8_t i = 0; i < 32; i++)
	{
		if (code & TOP_UINT32)
		{
			ir_mark(ONKYO_BIT_MARK);
			ir_space(ONKYO_ONE_SPACE);
		}
		else
		{
			ir_mark(ONKYO_BIT_MARK);
			ir_space(ONKYO_ZERO_SPACE);
		}
		code <<= 1;
	}
	ir_mark(ONKYO_BIT_MARK);
}

/*
 * Envía un código usando protocolo RC6
 */
static void sendRC6(uint32_t code)
{
	configIR(36);

	if (rc6tooglebit)
		code = code ^ 0x8000;
	rc6tooglebit = 1 - rc6tooglebit;

	// header
	ir_mark(XBOX_HEADER_MARK);
	ir_space(XBOX_HEADER_SPACE);

	// start bit (1)
	ir_mark(XBOX_BIT_MARK);
	ir_space(XBOX_BIT_MARK);

	// mode bits (110)
	ir_mark(XBOX_BIT_MARK);
	ir_space(XBOX_BIT_MARK);
	ir_mark(XBOX_BIT_MARK);
	ir_space(XBOX_BIT_MARK);
	ir_space(XBOX_BIT_MARK);
	ir_mark(XBOX_BIT_MARK);

	// trailer bit (0)
	ir_space(XBOX_TBIT_MARK);
	ir_mark(XBOX_TBIT_MARK);

	for (uint8_t i = 0; i < 32; i++)
	{
		if (code & TOP_UINT32)
		{
			ir_mark(XBOX_BIT_MARK);
			ir_space(XBOX_BIT_MARK);
		}
		else
		{
			ir_space(XBOX_BIT_MARK);
			ir_mark(XBOX_BIT_MARK);
		}
		code <<= 1;
	}
	ir_space(XBOX_SIGNAL_FREE);
}

/*
 * Envía un código IR usando el protocolo propietario del DVR cablevisión.
 */
static void sendCable(uint32_t code)
{
	GICR &= ~_BV(INT1); /* disable INT1 */

	configIR(38);

	int8_t i=5;
	while (--i)
	{
		_delay_ms(20);
	}

	code = code << 15;

	ir_mark(CABLE_HEADER_MARK);

	for (uint8_t i = 0; i < 17; i++)
	{
		if (code & TOP_UINT32)
		{
			ir_space(CABLE_ONE_SPACE);
			ir_mark(CABLE_BIT_MARK);
		}
		else
		{
			ir_space(CABLE_ZERO_SPACE);
			ir_mark(CABLE_BIT_MARK);
		}
		code <<= 1;
	}
	GICR |= _BV(INT1); /* enable INT1 */
}

/*
 * Mapea los códigos IR de PCREMOTE con codigos de otros controles o teclado/mouse
 */
static inline void ircodemap()
{
	if (ir_proto == IR_PROTO_PCREMOTE)
	{
		if (ir_value == BTNREMOTE_VOLPLUS) 		// VOL+
		{
#if IRDEBUG_LEVEL > 0
			if (ir_proto_control == IR_PROTO_PHILIPS)
				sendRC5(PHILIPS_VOLPLUS);
			else
#endif
				sendNEC(ONKYO_VOLPLUS);
		}
		else if (ir_value == BTNREMOTE_VOLMINUS) // VOL-
		{
#if IRDEBUG_LEVEL > 0
			if (ir_proto_control == IR_PROTO_PHILIPS)
				sendRC5(PHILIPS_VOLMINUS);
			else
#endif
				sendNEC(ONKYO_VOLMINUS);
		}
		else if (ir_value == BTNREMOTE_MUTE) // MUTE
		{
#if IRDEBUG_LEVEL > 0
			if (ir_proto_control == IR_PROTO_PHILIPS)
			sendRC5(PHILIPS_MUTE);
			else
#endif
			sendNEC(ONKYO_MUTE);
		}
		else if (ir_value == BTNREMOTE_PAGEPLUS) // CH+
		{
#if IRDEBUG_LEVEL > 0
			if (ir_proto_control == IR_PROTO_PHILIPS)
			sendRC5(PHILIPS_CHANNELPLUS);
			else
#endif
			if (ir_proto_control == IR_PROTO_ONKYO
					|| ir_proto_control == IR_PROTO_CABLE)
				sendCable(CABLE_CHANNELPLUS);
		}
		else if (ir_value == BTNREMOTE_PAGEMINUS) // CH-
		{
#if IRDEBUG_LEVEL > 0
			if (ir_proto_control == IR_PROTO_PHILIPS)
			sendRC5(PHILIPS_CHANNELMINUS);
			else
#endif
			if (ir_proto_control == IR_PROTO_ONKYO
					|| ir_proto_control == IR_PROTO_CABLE)
				sendCable(CABLE_CHANNELMINUS);
		}
		else if (ir_value == BTNREMOTE_TV)
		{
			sendNEC(ONKYO_CBLSAT);				//pulsa Cbl/sat del onkyo
			set_proto_control(IR_PROTO_CABLE);
			playpause_state = 0;
		}
		else if (ir_value == BTNREMOTE_MUSIC)
		{
			set_proto_control(IR_PROTO_ONKYO);	// modo para modificar configuración del onkyo
			playpause_state = 0;
		}
		else if (ir_value == BTNREMOTE_MOVIES)
		{
			sendNEC(ONKYO_BDDVD);				//pulsa bd/dvd del onkyo (PC)
			set_proto_control(IR_PROTO_PCREMOTE);
			playpause_state = 0;
		}
		else if (ir_value == BTNREMOTE_PHOTOS)
		{
			sendNEC(ONKYO_GAME);				// pulsa GAME del onkyo
			set_proto_control(IR_PROTO_XBOX);
			playpause_state = 0;
		}
		else if (ir_value == BTNREMOTE_POWER
				&& (ir_proto_control == IR_PROTO_CABLE
						|| ir_proto_control == IR_PROTO_PCREMOTE
						|| ir_proto_control == IR_PROTO_ONKYO)) // POWER
		{
			sendNEC(LG_POWER);  				//apaga el LG
			_delay_ms(5);
			sendCable(CABLE_POWER);				//apaga el cable
			_delay_ms(5);
			sendNEC(ONKYO_POWER);  				//apaga el Onkyo
		}
		else if (ir_proto_control == IR_PROTO_XBOX)
		{
			if (ir_value == BTNREMOTE_POWER)
				sendRC6(XBOX_POWER);
			else if (ir_value == BTNREMOTE_MOUSEDRAG)
				sendRC6(XBOX_FANCYBUTTON);
			else if (ir_value == BTNREMOTE_ARROWLEFT)
				sendRC6(XBOX_X);
			else if (ir_value == BTNREMOTE_ARROWUP)
				sendRC6(XBOX_Y);
			else if (ir_value == BTNREMOTE_ARROWDOWN)
				sendRC6(XBOX_A);
			else if (ir_value == BTNREMOTE_ARROWRIGHT)
				sendRC6(XBOX_B);
			else if (ir_value == BTNREMOTE_MOUSEUP)
				sendRC6(XBOX_UPARROW);
			else if (ir_value == BTNREMOTE_MOUSEDOWN)
				sendRC6(XBOX_DOWNARROW);
			else if (ir_value == BTNREMOTE_MOUSELEFT)
				sendRC6(XBOX_LEFTARROW);
			else if (ir_value == BTNREMOTE_MOUSERIGHT)
				sendRC6(XBOX_RIGHTARROW);
			else if (ir_value == BTNREMOTE_MOUSECLICK)
				sendRC6(XBOX_OK);
			else if (ir_value == BTNREMOTE_MOUSERIGHTCLK)
				sendRC6(XBOX_BACK);
			else if (ir_value == BTNREMOTE_ENTER)
				sendRC6(XBOX_ENTER);
			else if (ir_value == BTNREMOTE_CLOSE)
				sendRC6(XBOX_OPENCLOSE);
			else if (ir_value == BTNREMOTE_REWIND)
				sendRC6(XBOX_REWIND);
			else if (ir_value == BTNREMOTE_FORWARD)
				sendRC6(XBOX_FASTFORWARD);
			else if (ir_value == BTNREMOTE_PLAY)
			{
				if (playpause_state)
					sendRC6(XBOX_PAUSE);
				else
					sendRC6(XBOX_PLAY);
				playpause_state = 1 - playpause_state;
			}
			else if (ir_value == BTNREMOTE_STOP)
			{
				sendRC6(XBOX_STOP);
				playpause_state = 0;
			}
			else if (ir_value == BTNREMOTE_NEXTTRACK)
				sendRC6(XBOX_NEXT);
			else if (ir_value == BTNREMOTE_PREVTRACK)
				sendRC6(XBOX_PREV);
		}
#if IRDEBUG_LEVEL > 0
		else if (ir_proto_control == IR_PROTO_PHILIPS)
		{
			if (ir_value == BTNREMOTE_BACKSPACE)
				sendRC5(PHILIPS_PREVCHANNEL);
			else if (ir_value == BTNREMOTE_TAB)
				sendRC5(PHILIPS_ONE);
			else if (ir_value == BTNREMOTE_ARROWUP)
				sendRC5(PHILIPS_TWO);
			else if (ir_value == BTNREMOTE_WINKEY)
				sendRC5(PHILIPS_THREE);
			else if (ir_value == BTNREMOTE_ARROWLEFT)
				sendRC5(PHILIPS_FOUR);
			else if (ir_value == BTNREMOTE_ENTER)
				sendRC5(PHILIPS_FIVE);
			else if (ir_value == BTNREMOTE_ARROWRIGHT)
				sendRC5(PHILIPS_SIX);
			else if (ir_value == BTNREMOTE_FOLDERS)
				sendRC5(PHILIPS_SEVEN);
			else if (ir_value == BTNREMOTE_ARROWDOWN)
				sendRC5(PHILIPS_EIGHT);
			else if (ir_value == BTNREMOTE_ESC)
				sendRC5(PHILIPS_NINE);
			else if (ir_value == BTNREMOTE_ALTTAB)
				sendRC5(PHILIPS_ZERO);
		}
#endif
		else if (ir_proto_control == IR_PROTO_CABLE)
		{
			if (ir_value == BTNREMOTE_MOUSEUP)
				sendCable(CABLE_ARROWUP);
			else if (ir_value == BTNREMOTE_MOUSEDOWN)
				sendCable(CABLE_ARROWDOWN);
			else if (ir_value == BTNREMOTE_MOUSERIGHT)
				sendCable(CABLE_ARROWRIGHT);
			else if (ir_value == BTNREMOTE_MOUSELEFT)
				sendCable(CABLE_ARROWLEFT);
			else if (ir_value == BTNREMOTE_MOUSEDRAG)
				sendCable(CABLE_GUIDE);
			else if (ir_value == BTNREMOTE_MOUSECLICK)
				sendCable(CABLE_SELECT);
			else if (ir_value == BTNREMOTE_PLAY)
			{
				if (playpause_state)
					sendCable(CABLE_PAUSE);
				else
					sendCable(CABLE_PLAY);
				playpause_state = 1 - playpause_state;
			}
			else if (ir_value == BTNREMOTE_REWIND)
				sendCable(CABLE_REWIND);
			else if (ir_value == BTNREMOTE_FORWARD)
				sendCable(CABLE_FORWARD);
			else if (ir_value == BTNREMOTE_STOP)
			{
				sendCable(CABLE_STOP);
				playpause_state = 0;
			}
			else if (ir_value == BTNREMOTE_TAB)
				sendCable(CABLE_ONE);
			else if (ir_value == BTNREMOTE_ARROWUP)
				sendCable(CABLE_TWO);
			else if (ir_value == BTNREMOTE_WINKEY)
				sendCable(CABLE_THREE);
			else if (ir_value == BTNREMOTE_ARROWLEFT)
				sendCable(CABLE_FOUR);
			else if (ir_value == BTNREMOTE_ENTER)
				sendCable(CABLE_FIVE);
			else if (ir_value == BTNREMOTE_ARROWRIGHT)
				sendCable(CABLE_SIX);
			else if (ir_value == BTNREMOTE_FOLDERS)
				sendCable(CABLE_SEVEN);
			else if (ir_value == BTNREMOTE_ARROWDOWN)
				sendCable(CABLE_EIGHT);
			else if (ir_value == BTNREMOTE_ESC)
				sendCable(CABLE_NINE);
			else if (ir_value == BTNREMOTE_ALTTAB)
				sendCable(CABLE_ZERO);
			else if (ir_value == BTNREMOTE_MOUSERIGHTCLK)
				sendCable(CABLE_EXIT);
			else if (ir_value == BTNREMOTE_NEXTTRACK)
				sendCable(CABLE_LIVE);
			else if (ir_value == BTNREMOTE_PREVTRACK)
				sendCable(CABLE_LIST);
			else if (ir_value == BTNREMOTE_NUMLOCK)
				sendCable(CABLE_RECORD);
			else if (ir_value == BTNREMOTE_MYPC)
				sendCable(CABLE_SETTINGS);
			else if (ir_value == BTNREMOTE_DESKTOP)
				sendCable(CABLE_INFO);
		}
		else if (ir_proto_control == IR_PROTO_ONKYO)
		{
			if (ir_value == BTNREMOTE_MOUSEUP)
				sendNEC(ONKYO_ARROWUP);
			else if (ir_value == BTNREMOTE_MOUSEDOWN)
				sendNEC(ONKYO_ARROWDOWN);
			else if (ir_value == BTNREMOTE_MOUSERIGHT)
				sendNEC(ONKYO_ARROWRIGHT);
			else if (ir_value == BTNREMOTE_MOUSELEFT)
				sendNEC(ONKYO_ARROWLEFT);
			else if (ir_value == BTNREMOTE_MOUSECLICK)
				sendNEC(ONKYO_ENTER);
			else if (ir_value == BTNREMOTE_MOUSERIGHTCLK)
				sendNEC(ONKYO_RETURN);
			else if (ir_value == BTNREMOTE_MYPC)
				sendNEC(ONKYO_SETUP);
			else if (ir_value == BTNREMOTE_TAB)
				sendNEC(ONKYO_AUDIO);
			else if (ir_value == BTNREMOTE_WINKEY)
				sendNEC(ONKYO_AUDIOMOVIETV);
			else if (ir_value == BTNREMOTE_ARROWRIGHT)
				sendNEC(ONKYO_AUIDOMUSIC);
			else if (ir_value == BTNREMOTE_ESC)
				sendNEC(ONKYO_AUDIOGAME);
			else if (ir_value == BTNREMOTE_DESKTOP)
				sendNEC(ONKYO_AUDIOSTEREO);
		}

		if (ir_proto_control == IR_PROTO_PCREMOTE)
		{
			if (usb_interruptisready())
			{
				if (ir_value == BTNREMOTE_MOUSEUP)
					mouse_move(0, -MOUSE_MOVE_INC);
				else if (ir_value == BTNREMOTE_MOUSEDOWN)
					mouse_move(0, MOUSE_MOVE_INC);
				else if (ir_value == BTNREMOTE_MOUSERIGHT)
					mouse_move(MOUSE_MOVE_INC, 0);
				else if (ir_value == BTNREMOTE_MOUSELEFT)
					mouse_move(-MOUSE_MOVE_INC, 0);
				else if (ir_value == BTNREMOTE_MOUSEUPRIGHT)
					mouse_move(MOUSE_MOVE_INC, -MOUSE_MOVE_INC);
				else if (ir_value == BTNREMOTE_MOUSEDNRIGHT)
					mouse_move(MOUSE_MOVE_INC, MOUSE_MOVE_INC);
				else if (ir_value == BTNREMOTE_MOUSEDNLEFT)
					mouse_move(-MOUSE_MOVE_INC, MOUSE_MOVE_INC);
				else if (ir_value == BTNREMOTE_MOUSEUPLEFT)
					mouse_move(-MOUSE_MOVE_INC, -MOUSE_MOVE_INC);
				else if (ir_value == BTNREMOTE_MOUSECLICK)
					mouse_click();
				else if (ir_value == BTNREMOTE_MOUSERIGHTCLK)
					mouse_rclick();
				else
				{
					for (uint8_t k = 0; k < NUM_CODES; k++)
					{
						if (ir_value != code_map[k].remote_code)
							continue;

						kbd_send(code_map[k].hid_code_mod,
								code_map[k].hid_code);
						break;
					}
				}
			}
		}
	}
	reset_timeout();
}

/*
 * Agrega un 0 un 1 al valor recibido por IR
 * Si se llega al máximo de bits definido se detiene y mapea a una acción.
 */
static uint8_t addbit(uint8_t nbit)
{
	ir_bits++;
	ir_value <<= 1;

	if (nbit == 1)
	{
		ir_value |= 1;
#if IRDEBUG_LEVEL > 1
		if (global.mode == MODE_NORMAL)
		DBG2("1");
#endif
	}
#if IRDEBUG_LEVEL > 1
	else if (global.mode == MODE_NORMAL)
	DBG2("0");
#endif

	if (ir_bits == ir_defbits)
	{
		ircodemap();

		output_toggle(PORTB, LED_IR_RECEIVED);			// alterna estado del led de actividad

		DBG1("CODE:");printDec(ir_value);DBG1("\r\n");

		return 1;
	}
	else if (ir_bits > MAX_BITS)
	{
		DBG1("\r\n");
		return 1;
	}
	return 0;
}

/*
 * Máquina de estado para la decodificación manchester de PCREMOTE u otros protocolos.
 */
static inline uint8_t decode(uint8_t level, uint16_t time)
{
	if (ir_state == STATE_IDLE)
	{
		if (!level)
			return ir_reset();

		if (ir_proto == IR_PROTO_PCREMOTE && ir_check(bit_mark, time))
			ir_state = STATE_MARK;
#if IRDEBUG_LEVEL > 0
		else if (ir_proto == IR_PROTO_PHILIPS && ir_check(bit_mark, time))
		{
			period_state = 0;
			addbit(1);
			ir_state = STATE_SPACE;
		}
		else if ((ir_proto == IR_PROTO_ONKYO || ir_proto == IR_PROTO_LG)
				&& ir_check(header_mark, time))
		ir_state = STATE_HEADER;
		else if (ir_proto == IR_PROTO_CABLE && ir_check(header_mark, time))
		ir_state = STATE_MARK;
#endif
		else
			return ir_reset();
	}
#if IRDEBUG_LEVEL > 0
	else if (ir_state == STATE_HEADER)
	{
		if (!level && ir_check(header_space, time))
		{
			ir_state = STATE_SPACE;
			ir_value = 0;
		}
		else
		return ir_reset();
	}
#endif
	else if (ir_state == STATE_SPACE)
	{
#if IRDEBUG_LEVEL > 0
		if (ir_proto == IR_PROTO_PHILIPS)
		{
			if (level)
			return ir_reset();

			if (period_state == 0 && ir_check(bit_mark, time))
			{
				period_state = 1;
				if (addbit(1))
				return ir_reset();
			}
			else if (period_state == 1 && ir_check(one_space, time))
			{
				if (addbit(1))
				return ir_reset();
			}
			else
			period_state = 1 - period_state;
		}
#endif
		ir_state = STATE_MARK;
	}
	else if (ir_state == STATE_MARK)
	{
#if IRDEBUG_LEVEL > 0
		if (ir_proto == IR_PROTO_PHILIPS)
		{
			if (!level)
			return ir_reset();

			if (period_state == 0 && ir_check(bit_mark, time))
			{
				period_state = 1;
				if (addbit(0))
				return ir_reset();
			}
			else if (period_state == 1 && ir_check(one_space, time))
			{
				if (addbit(0))
				return ir_reset();
			}
			else
			period_state = 1 - period_state;
		}
		else
		{
#endif
		if (level)
			return ir_reset();

		if (ir_check(one_space, time))
		{
			if (addbit(1))
				return ir_reset();
		}
		else if (ir_check(zero_space, time))
		{
			if (addbit(0))
				return ir_reset();
		}
		else
			return ir_reset();
#if IRDEBUG_LEVEL > 0
	}
#endif
		ir_state = STATE_SPACE;
	}
	return DECODED;
}

/*
 * Si hay pulsos en el buffer circular, invoca a la decodificación
 */
void ir_decode()
{
	while (irbuf_head != irbuf_tail)
	{
		uint16_t timing = irbuf[irbuf_tail];

		decode(irbuf_tail & 1, timing);
		irbuf_tail = (irbuf_tail + 1) % RAWBUF;
	}
}

#if IRDEBUG_LEVEL > 0
/*
 * Si hay pulsos en el buffer circular, los envía por puerto serial
 */
void ir_record()
{
	/* if any pulses were received, decode them */
	while (irbuf_head != irbuf_tail)
	{
		uint16_t timing = irbuf[irbuf_tail];

		printDec(timing); DBG1(",");

		irbuf_tail = (irbuf_tail + 1) % RAWBUF;
	}
}

/*
 * Alterna protocolo de control remoto
 */
void inc_protocol()
{
	ir_proto++;
	if (ir_proto == MAXPROTO)
	ir_proto = 0;
	set_protocol(ir_proto);
	ir_reset();

	DBG1("INPUT PROTOCOL:");printDec(ir_proto);DBG1("\r\n");
}
#endif

/*
 * Inicializa la portadora de la señal infrarroja.
 */
static inline void ctc_init()
{
// Inicia timer2 en modo CTC
// Focn = F_CPU/(2*N*(1+OCR2)) = HZ => F_CPU/(2*HZ)-1 = OCR2 => F_CPU/(2*38000) = OCR2
// ;donde N es el prescaler = 1
// COM20 = Toggle OC2 on Compare Match
// CS20  = N=1 => Sin prescaler
	TCCR2 = (1 << WGM21) | (1 << CS20) | (1 << COM20);  // CTC
}

/*
 * Inicializa la INT1 para calcular el tiempo de cada pulso, medido cada 50us.
 */
static inline void int1_init()
{
	GICR |= _BV(INT1); /* enable INT1 */

	// inicializa el timer1 para contar el tiempo de un pulso en ticks de 4us
	TCCR1A = 0; /* Modo Normal */
	MCUCR = (MCUCR & ~(_BV(ISC11) | _BV(ISC10))) | _BV(ISC11); /* INT1 para flanco descendente */
	TCCR1B = 3; // prescaler = 1/64 -> 64/F_CPU = 5.3us
	TCNT1 = 0; /* limpia contador */
}

/*
 * Interrupción INT1, se detectó flanco de subida o bajada
 * que proviene del sensor IR.
 */
ISR (INT1_vect)
{
	irbuf[irbuf_head] = TCNT1; /* almacena el ancho del pulso */

	irbuf_head = (irbuf_head + 1) % RAWBUF;

	MCUCR ^= _BV(ISC10); /* alterna entre flanco ascendente/descendente */
	TCNT1 = 0; /* limpia contador */
}

/*
 * Inicializa lo necesario para reconocer protocolos IR
 */
void ir_init(uint8_t proto)
{
	set_protocol(proto);

	set_proto_control(IR_PROTO_CABLE);
	//set_proto_control(IR_PROTO_PHILIPS);   // solo usado para algunas pruebas

	ir_reset();

	cbi(PORTB, LED_IR_RECEIVED);			// apaga el led de actividad

	int1_init();
	ctc_init();
}

