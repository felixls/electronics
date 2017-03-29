/*
* Cargador de baterías universal
* Despligue datos en LCD
* Autor: Felixls - 2009
*/
#ifndef DISPLAY_H
#define DISPLAY_H

#include "charger.h"

extern unsigned int adc_voltage;
extern unsigned int adc_current;
extern unsigned int adc_discharge_current;
extern unsigned int adc_temperature;

extern unsigned int actualvoltage;
extern unsigned int actualcurrent;
extern unsigned char actualtemperature;

extern void readdischargeinfo();

void showstate();
void lcd_showint(unsigned int, char);
void print_uint(unsigned int value);
void print_initaction(unsigned int current);

#endif
