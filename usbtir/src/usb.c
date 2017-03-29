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
#include "global.h"
#include "usbdrv.h"

static uint8_t inputBuffer1[3] = { 1, 0, 0 };		// teclado
static uint8_t inputBuffer2[4] = { 2, 0, 0, 0 };	// ratón
static uint8_t tx_kbd = 0;							// flag envio tecla
static uint8_t tx_mouse = 0;						// flag envio mov. ratón

static uint8_t idleRate;							// tiempo en idle, en unidades de 4 ms
static uint8_t ir_timeout = 0;						// contador de tiempo límite antes de liberar un botón con un
#define IR_TIMEOUT	8								//  intervalo de 21.8ms

PROGMEM char usbHidReportDescriptor[91] = {
        /* keyboard */
		0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
		0x09, 0x06,                    // USAGE (Keyboard)
		0xa1, 0x01,                    // COLLECTION (Application)
		0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
		0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
		0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
		0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
		0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
		0x85, 0x01, 				   //   Report Id (1)
		0x75, 0x01,                    //   REPORT_SIZE (1)
		0x95, 0x08,                    //   REPORT_COUNT (8)
		0x81, 0x02,                    //   INPUT (Data,Var,Abs)
		0x95, 0x01,                    //   REPORT_COUNT (1)
		0x75, 0x08,                    //   REPORT_SIZE (8)
		0x15, 0x00, 				   //   Logical Minimum (0)
		0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
		0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
		0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
		0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
		0xc0,                          // END_COLLECTION

		/* mouse */
		0x05, 0x01, /* Usage Page (Generic Desktop), */
		0x09, 0x02, /* Usage (Mouse), */
		0xA1, 0x01, /* Collection (Application), */
		0x09, 0x01, /*   Usage (Pointer), */
		0xA1, 0x00, /*   Collection (Physical), */
		0x05, 0x09, /* Usage Page (Buttons), */
		0x19, 0x01, /* Usage Minimum (01), */
		0x29, 0x03, /* Usage Maximun (03), */
		0x15, 0x00, /* Logical Minimum (0), */
		0x25, 0x01, /* Logical Maximum (1), */
		0x85, 0x02, /* Report Id (2) */
		0x95, 0x03, /* Report Count (3), */
		0x75, 0x01, /* Report Size (1), */
		0x81, 0x02, /* Input (Data, Variable, Absolute), ;3 button bits */
		0x95, 0x01, /* Report Count (1), */
		0x75, 0x05, /* Report Size (5), */
		0x81, 0x01, /* Input (Constant),                 ;5 bit padding */
		0x05, 0x01, /* Usage Page (Generic Desktop), */
		0x09, 0x30, /* Usage (X), */
		0x09, 0x31, /* Usage (Y), */
		0x15, 0x81, /* Logical Minimum (-127), */
		0x25, 0x7F, /* Logical Maximum (127), */
		0x75, 0x08, /* Report Size (8), */
		0x95, 0x02, /* Report Count (2), */
		0x81, 0x06, /* Input (Data, Variable, Relative), ;2 position bytes (X & Y) */
		0xC0, /*   End Collection, */
		0xC0, /* End Collection */
};

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	usbRequest_t *rq = (void *) data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)
	{
		/* class request type */
		if (rq->bRequest == USBRQ_HID_GET_REPORT)
		{
			/* wValue: ReportType (highbyte), ReportID (lowbyte) */

			if (rq->wValue.bytes[0] == 1)		// ReportID = 1 ? (teclado)
			{
				usbMsgPtr = inputBuffer1;
				return sizeof(inputBuffer1);
			}
			else if (rq->wValue.bytes[0] == 2)  // ReportID = 2 ? (mouse)
			{
				usbMsgPtr = inputBuffer2;
				return sizeof(inputBuffer2);
			}
			return 0;
		}
		else if (rq->bRequest == USBRQ_HID_SET_REPORT)
		{
			/* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we have no output/feature reports */
			return 0;
		}
		else if (rq->bRequest == USBRQ_HID_GET_IDLE)
		{
			usbMsgPtr = &idleRate;
			return 1;
		}
		else if (rq->bRequest == USBRQ_HID_SET_IDLE)
		{
			idleRate = rq->wValue.bytes[1];
		}
	}
	else
	{
		/* ignore vendor type requests, we don't use any */
	}
	return 0;
}

/*
 * Borra los buffer de transferencia
 */
static void hid_clear(void)
{
	inputBuffer2[1] = 0; /* mouse buttons */
	inputBuffer2[2] = 0; /* X axis delta */
	inputBuffer2[3] = 0; /* Y axis delta */

	inputBuffer1[1] = 0; /* Modificador (shift/control/alt/super) */
	inputBuffer1[2] = 0; /* Tecla */
}

/*
 * Envía paquetes de datos pendientes
 */
static inline void send_packets(void)
{
	if (usbInterruptIsReady())
	{
		if (tx_kbd)
		{
			usbSetInterrupt(inputBuffer1, sizeof(inputBuffer1)); /* envía un evento de teclado */
			tx_kbd = 0;
		}
		else if (tx_mouse)
		{
			usbSetInterrupt(inputBuffer2, sizeof(inputBuffer2)); /* envía un evento de ratón */
			tx_mouse = 0;
		}
	}
}

/*
 * Inicializa el timer0 usado como temporizador de tiempo en idle.
 */
void idle_timer_init()
{
	/* configure timer 0 for a rate of 12M/(1024 * 256) = 45.78 Hz (~22ms) */
	TCCR0 = 5;
	/* timer 0 prescaler: 1024 */
}

/*
 * Opera para determinar si envía reportes de idle o liberación de pulsación.
 */
void idle_timer(void)
{
	static uint8_t idleCounter = 0;

	/* reportes de idle */
	if (idleRate)
	{
		if (idleCounter > 4)
		{
			idleCounter -= 5;
		}
		else
		{
			idleCounter = idleRate;
			tx_kbd = 1;
			tx_mouse = 1;
		}
	}

	/* liberación de pulsación */
	if (ir_timeout)
	{
		if (!--ir_timeout)
		{
			hid_clear();
			tx_kbd = 1;
			tx_mouse = 1;
			//led_on();
		}
	}

	/* apaga el led de actividad */
	cbi(PORTB, LED_IR_RECEIVED);
}

/*
 * Procesamiento de envíos USB
 */
void usb_poll()
{
	usbPoll();
	send_packets();
}

/*
 * Inicializa la emulación USB
 */
void usb_init(void)
{
	usbInit();
}

/*
 * Desconecta el dispositivo del host.
 */
void usb_disconnect(void)
{
	usbDeviceDisconnect();
}

/*
 * Conecta el dispositivo al host.
 */
void usb_connect(void)
{
	usbDeviceConnect();
}

/*
 * Determina si es posible insertar un evento en el bus.
 */
uint8_t usb_interruptisready()
{
	return usbInterruptIsReady();
}

/*
 * Reiniciliza el tiempo de liberación de pulsaciones.
 */
void reset_timeout(void)
{
	ir_timeout = IR_TIMEOUT;
}

/*
 * Envía un evento de ratón
 */
static void mouse_send()
{
	tx_mouse = 1;
	usbSetInterrupt(inputBuffer2, sizeof(inputBuffer2)); /* send mouse event */
}

/*
 * Prepara el buffer con el movimiento de mouse requerido
 */
void mouse_move(int8_t dx, int8_t dy)
{
	hid_clear();
	if (dx != 0)
		inputBuffer2[2] = dx;
	if (dy != 0)
		inputBuffer2[3] = dy;
	mouse_send();
}

/*
 * Prepara el buffer con el click del mouse
 */
void mouse_click(void)
{
	hid_clear();
	inputBuffer2[1] = 1;
	mouse_send();
}

/*
 * Prepara el buffer con el click derecho del mouse
 */
void mouse_rclick(void)
{
	hid_clear();
	inputBuffer2[1] = 2;
	mouse_send();
}

/*
 * Prepara el buffer con la tecla o combinación de teclas.
 */
void kbd_send(uint8_t mod, uint8_t keycode)
{
	hid_clear();
	inputBuffer1[1] = mod;
	inputBuffer1[2] = keycode;
	tx_kbd = 1;
	usbSetInterrupt(inputBuffer1, sizeof(inputBuffer1)); /* send kbd event */
}
