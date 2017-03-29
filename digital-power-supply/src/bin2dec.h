#ifndef BIN2DEC_H
#define BIN2DEC_H

/*
- based on the 16 bit to 5 digit conversion written by John Payson
  & explained by Scott Dattalo

- Adaptado a SDCC por Felixls - Abril 2009

*/

unsigned char ACC0;                                // variables temporales para calculos y conversiones
unsigned char ACC1;
unsigned char  TenK, Thou, Hund, Tens, Ones;

void bin2dec(void)
{
_asm
	swapf	_ACC1,w               ;w  = A2*16+A3
	andlw	0x0F                 ;w  = A3		//*** PERSONALLY, I'D REPLACE THESE 2
	addlw	0xF0                 ;w  = A3-16	//*** LINES WITH "IORLW b'11110000B' " -AW
	movwf	_Thou                 ;B3 = A3-16
	addwf	_Thou,f               ;B3 = 2*(A3-16) = 2A3 - 32
	addlw	.226                 ;w  = A3-16 - 30 = A3-46
	movwf	_Hund                 ;B2 = A3-46
	addlw	.50                  ;w  = A3-46 + 50 = A3+4
	movwf	_Ones                 ;B0 = A3+4

	movf	_ACC1,w               ;w  = A3*16+A2
	andlw	0x0F                 ;w  = A2
	addwf	_Hund,f	             ;B2 = A3-46 + A2 = A3+A2-46
	addwf	_Hund,f	             ;B2 = A3+A2-46  + A2 = A3+2A2-46
	addwf	_Ones,f	                 ;B0 = A3+4 + A2 = A3+A2+4
	addlw	.233	               ;w  = A2 - 23
	movwf	_Tens	               ;B1 = A2-23
	addwf	_Tens,f	             ;B1 = 2*(A2-23)
	addwf	_Tens,f               ;B1 = 3*(A2-23) = 3A2-69 (Doh! thanks NG)

	swapf	_ACC0,w	             ;w  = A0*16+A1
	andlw	0x0F                 ;w  = A1
	addwf	_Tens,f              ;B1 = 3A2-69 + A1 = 3A2+A1-69 range -69...-9
	addwf	_Ones,f              ;B0 = A3+A2+4 + A1 = A3+A2+A1+4 and Carry = 0 (thanks NG)

	rlf	_Tens,f                ;B1 = 2*(3A2+A1-69) + C = 6A2+2A1-138 and Carry is now 1 as tens register had to be negitive
	rlf	_Ones,f                ;B0 = 2*(A3+A2+A1+4) + C = 2A3+2A2+2A1+9 (+9 not +8 due to the carry from prev line, Thanks NG)
 	comf	_Ones,f              ;B0 = ~(2A3+2A2+2A1+9) = -2A3-2A2-2A1-10 (ones complement plus 1 is twos complement. Thanks SD)
	rlf	_Ones,f                 ;B0 = 2*(-2A3-2A2-2A1-10) = -4A3-4A2-4A1-20
	movf	_ACC0,w               ;w  = A1*16+A0
	andlw	0x0F                 ;w  = A0
	addwf	_Ones,f              ;B0 = -4A3-4A2-4A1-20 + A0 = A0-4(A3+A2+A1)-20 range -215...-5 Carry=0
	rlf	_Thou,f                 ;B3 = 2*(2A3 - 32) = 4A3 - 64
	movlw	0x07	               ;w  = 7
	movwf	_TenK                ;B4 = 7

	movlw	.10	                 ;w  = 10
Lb1:
  addwf	_Ones,f               ; B0 += 10
	decf	_Tens,f               ; B1 -= 1
	btfss	3,0
                             ;skip no carry
	goto	Lb1	                 ; while B0 < 0
                           	 ;jmp carry
Lb2:
  addwf	_Tens,f               ; B1 += 10
	decf	_Hund,f	             ; B2 -= 1
	btfss	3,0
	goto	Lb2                  ; while B1 < 0
Lb3:
  addwf	_Hund,f	             ; B2 += 10
	decf	_Thou,f               ; B3 -= 1
	btfss	3,0
	goto	Lb3	                 ; while B2 < 0
Lb4:
  addwf	_Thou,f               ; B3 += 10
	decf	_TenK,f               ; B4 -= 1
	btfss	3,0
	goto	Lb4                  ; while B3 < 0
;  retlw	0
  movlw 0
_endasm;
}
#endif
