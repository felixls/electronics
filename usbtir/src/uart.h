#ifndef UART_H
#define UART_H

#include "global.h"

/*
 General Description:
 This module implements a function for debug logs on the serial line of the
 AVR microcontroller. Debugging can be configured with the define
 'DEBUG_LEVEL'. If this macro is not defined or defined to 0, all debugging
 calls are no-ops. If it is 1, DBG1 logs will appear, but not DBG2. If it is
 2, DBG1 and DBG2 logs will be printed.

 A debug log consists of a label ('prefix') to indicate which debug log created
 the output and a memory block to dump in hex ('data' and 'len').
 */

#ifndef F_CPU
#   define  F_CPU   12000000    /* 12 MHz */
#endif

/* make sure we have the UART defines: */
#include "usbportability.h"

#ifndef uchar
#   define  uchar   unsigned char
#endif

#if IRDEBUG_LEVEL > 0 && !(defined TXEN || defined TXEN0) /* no UART in device */
#   warning "Debugging disabled because device has no UART"
#   undef   IRDEBUG_LEVEL
#endif

#ifndef IRDEBUG_LEVEL
#   define  IRDEBUG_LEVEL 0
#endif

/* ------------------------------------------------------------------------- */

#if IRDEBUG_LEVEL > 0
#   define  DBG1(data) odDebug(data)
#else
#   define  DBG1(data)
#endif

#if IRDEBUG_LEVEL > 1
#   define  DBG2(data) odDebug(data)
#else
#   define  DBG2(data)
#endif

#if IRDEBUG_LEVEL > 0
extern void printDec(uint32_t value);
#else
#define printDec(value)
#endif

#if IRDEBUG_LEVEL > 0
extern void printHex(uint8_t value);
#else
#define printHex(value)
#endif

/* ------------------------------------------------------------------------- */

#if IRDEBUG_LEVEL > 0
extern void odDebug(const char *data);

/* Try to find our control registers; ATMEL likes to rename these */

#if defined UBRR
#   define  ODDBG_UBRR  UBRR
#elif defined UBRRL
#   define  ODDBG_UBRR  UBRRL
#elif defined UBRR0
#   define  ODDBG_UBRR  UBRR0
#elif defined UBRR0L
#   define  ODDBG_UBRR  UBRR0L
#endif

#if defined UCR
#   define  ODDBG_UCR   UCR
#elif defined UCSRB
#   define  ODDBG_UCR   UCSRB
#elif defined UCSR0B
#   define  ODDBG_UCR   UCSR0B
#endif

#if defined TXEN
#   define  ODDBG_TXEN  TXEN
#else
#   define  ODDBG_TXEN  TXEN0
#endif

#if defined USR
#   define  ODDBG_USR   USR
#elif defined UCSRA
#   define  ODDBG_USR   UCSRA
#elif defined UCSR0A
#   define  ODDBG_USR   UCSR0A
#endif

#if defined UDRE
#   define  ODDBG_UDRE  UDRE
#else
#   define  ODDBG_UDRE  UDRE0
#endif

#if defined UDR
#   define  ODDBG_UDR   UDR
#elif defined UDR0
#   define  ODDBG_UDR   UDR0
#endif

static inline void debug_init(void)
{
	ODDBG_UCR |= (1 << ODDBG_TXEN);
	ODDBG_UBRR = F_CPU / (19200 * 16L) - 1;
}
#else
#   define debug_init()
#endif

/* ------------------------------------------------------------------------- */

#endif // UART_H
