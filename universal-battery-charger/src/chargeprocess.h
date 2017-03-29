/*
* Cargador de baterías universal
* Proceso de carga
* Autor: Felixls - 2009
*/
#ifndef CHARGEPROCESS_H
#define CHARGEPROCESS_H

#include "charger.h"

extern UINT adc_voltage;
extern UINT adc_current;
extern UINT adc_discharge_current;
extern UINT adc_temperature;
extern BYTE adc_channel;
extern BYTE mode;

extern BYTE mseconds, seconds;
extern UINT minutes;

extern UINT maxcurrent;                    // mA

extern BYTE cells;
extern UINT batterycapacity;
extern UINT chargecurrent;                 // 1 = 0.1 C
extern UINT dischargecurrent;              // mA

extern UINT cellmaxvoltage;                // mV
extern UINT voltagecutoff;                 // mV/celda
extern BYTE NDV;                           // mV
extern BYTE NDVwait;
extern UINT finalcurrent;                  // %
extern UINT normalchargevoltage;           // mV
extern UINT timeout;                       // minutos
extern UINT mintemphighcurrent;
extern UINT maxtemphighcurrent;

extern int abs_max_temp;
extern int abs_min_temp;

extern UINT pwmduty;                       // Valor de CCPR1L:CCP1CON<5,4> para variar el duty cycle
                                           // de acuerdo a los movimientos del encoder (modo voltaje).
extern UINT pwmduty2;

extern BYTE terminate;
extern BYTE phase;
extern BYTE current_action;

extern UINT voltagecontrol;
extern UINT normalchargevoltagecontrol;
extern UINT voltagecutoffcontrol;
extern UINT chargecurrentcontrol;
extern UINT finalcurrentcontrol;

extern UINT actualvoltage;
extern UINT actualcurrent;
extern BYTE actualtemperature;

void pwm2_set_data();
void pwm1_set_data();
void incduty(int);
void decduty(int);
void incduty2(int);
void decduty2(int);
void print_batterytype();

void charge();

extern int batteryinfo(unsigned char );
extern void readdischargeinfo();
extern void readbatteryinfo();

#endif
