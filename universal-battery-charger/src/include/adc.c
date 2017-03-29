
/*****************************************************************************
******************************************************************************
****                                                                      ****
****                    --- DRIVER ADC para PIC16F ---                  ****
**** ****
******************************************************************************
******************************************************************************

                            FUNCIONES DISPONIBLES:
------------------------------------------------------------------------------

adc_init( FOSC_32, A5_R0, INT_OFF); iniciar módulo ADC, donde:

FOSC_32:
Frecuencia de adc, en este caso 1/32 de la del oscilador del pic

A5_R0:
Definir entradas analógicas y Vref, en este caso todas las 5 del pic16f876a como analógicas
y ninguna como Vref, Vref serán Vdd y GND (ver las posibles combinaciones en el driver y datasheet del pic a usar.

INT_OFF:
Interrupciones ADC deshabilitadas, la otra posiblidad es INT_ON.


adc_read(1); Leer canal ADC.

adc_close(); cerrar módulo ADC.

Agregado de adc_startread por Felixls - Abril 2009

******************************************************************************
*****************************************************************************/

#include <pic/pic16f877a.h>
#include "adc.h"
#include "delay.h"

void adc_init(unsigned char fosc, unsigned char pcfg, unsigned char config)
{
   ADCON0 = 0;
   ADCON0 = (fosc & 0x03) << 6;         // establecer frecuencia
   ADCON1 = (pcfg & 0x0F) | 0x080;      // establecer entradas
   if (fosc & 0x04)
      ADCS2 = 1;
   if (config & INT_ON)                 // establecer interrupciones
   {
      ADIF = 0;
      ADIE = 1;
      PEIE = 1;
   }
   ADON = 1;                            // habilitar módulo ADC
}

void adc_close(void)                    // deshabilitar módulo ADC
{
   ADON = 0;
   ADIE = 0;
}

unsigned int adc_read(unsigned char canal)
{
   unsigned int valor;

   ADCON0 &= 0xC7;                     // borrar anteriores selecciones
   ADCON0 |= (canal & 0x07) << 3;      // establecer canal seleccionado

   delay_ms(30);
   GO = 1;                             // iniciar conversión
   while (GO);                         // esperar que termine
  
   valor = ADRESH << 8 | ADRESL;       // leer valor
   return valor;
}

void adc_startread(unsigned char canal)
{
   ADCON0 &= 0xC7;                     // borrar anteriores selecciones
   ADCON0 |= (canal & 0x07) << 3;      // establecer canal seleccionado

   delay_us(30);
   GO = 1;               // iniciar conversión
}
