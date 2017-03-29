//-------------------------------------------------------------------------------------------------
// DELAY.C
//
// Autor: Felixls
// Web: http://sergiols.blogspot.com
//
// Esperas de tiempo en microsegundos/milisegundos basado en un micro a 20mhz
//-------------------------------------------------------------------------------------------------

#include "delay.h"

// tiempo = (5a+1)µs a>=2
void delay_us ( char a )
{ a;
 _asm                 // a ya está cargado en el registro w
    sublw 1           //(1-a)
bucle:
    ADDLW 1
	  btfss 0x03,0        //test si w=0 carry=1
    goto bucle
 _endasm;
}

void delay_ms(int ms)
{
    unsigned int j;
    unsigned int k = ms * 10;
    for (j=0; j<k; j++)
      delay_us (33);
}
