#ifndef CALCULO_H
#define CALCULO_H
//--------------------------------------------------------------------------------------------------
// código para multiplicar por 2.5
// generado por
// http://www.piclist.com/cgi-bin/constdivmul.exe?Acc=ACC&Bits=10&endian=little&Const=2.5&ConstErr=0&Temp=TEMP0&cpu=pic16
// Parámetros de entrada
// ACC0, ACC1 (LO/HI)
// Devuelve el resultado en ACC0 y ACC1.
//
// Adaptado para SDCC por Felixls
//--------------------------------------------------------------------------------------------------

unsigned char TEMP0;
unsigned char TEMP1;

void multi2_5()
{
_asm
; Approximated constant: 2.5, Error: 0 %
;     Input: ACC0 .. ACC1, 10 bits
;    Output: ACC0 .. ACC1, 12 bits
; Code size: 16 instructions
; MULTI_2.5:		;MULTIPLICA POR 2,5

;copy accumulator to temporary
	movf	_ACC1, w
	movwf	_TEMP1
	movf	_ACC0, w
	movwf	_TEMP0
;shift accumulator right 1 times
	clrc
	rrf	_ACC1, f
	rrf	_ACC0, f
;shift temporary left 1 times
	clrc
	rlf	_TEMP0, f
	rlf	_TEMP1, f
;add temporary to accumulator
	movf	_TEMP0, w
	addwf	_ACC0, f
	movf	_TEMP1, w
	skpnc
	incfsz	_TEMP1, w
	addwf	_ACC1, f
_endasm;
}

#endif
