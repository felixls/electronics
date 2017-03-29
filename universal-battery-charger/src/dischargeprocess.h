/*
* Cargador de baterías universal
* Proceso de descarga
* Autor: Felixls - 2009
*/

#ifndef DISCHARGEPROCESS_H
#define DISCHARGEPROCESS_H

#include "charger.h"

extern UINT maxcurrent;                    // mA

extern BYTE cells;
extern UINT batterycapacity;
extern UINT chargecurrent;                 // 1 = 0.1 C
extern UINT dischargecurrent;              // mA
extern BYTE mode;

extern BYTE mseconds, seconds;
extern UINT minutes;

extern UINT cellmaxvoltage;                // mV
extern UINT voltagecutoff;                 // mV/celda
extern BYTE NDV;                           // mV
extern UINT finalcurrent;                  // %
extern UINT normalchargevoltage;           // mV
extern UINT timeout;                       // minutos

extern UINT pwmduty;                             // Valor de CCPR1L:CCP1CON<5,4> para variar el duty cycle
                                          // de acuerdo a los movimientos del encoder (modo voltaje).
extern UINT pwmduty2;

extern BYTE terminate;
extern BYTE phase;
extern BYTE current_action;

extern UINT voltagecutoffcontrol;

extern UINT actualvoltage;
extern UINT actualcurrent;
extern BYTE actualtemperature;

void discharge();

extern void pwm2_set_data();
extern void pwm1_set_data();

extern int batteryinfo(unsigned char);
extern void displaybatterytype(BYTE);
extern void print_batterytype();
extern void readbatteryinfo();

#endif

