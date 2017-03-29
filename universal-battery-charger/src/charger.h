/*
* Cargador de baterías universal
* Proceso de carga
* Autor: Felixls - 2009
*/
#ifndef CHARGER_H
#define CHARGER_H

#include <pic/pic16f877a.h>

#include "delay.h"
#include "lcd.h"
#include "adc.h"
#include "serie.h"
#include "display.h"

#define BATTERYCHARGER_VERSION     "Version: 1.0"

#define LCD_BACKLIGHT              RB1

#define PIEZO                      RC5

#define OK_BUTTON                  RD7
#define DOWN_BUTTON                RD6
#define UP_BUTTON                  RD5
#define MENU_BUTTON                RD4
#define LED_CHARGEDISCHARGE        RD3
#define LED_OPCOMPLETE             RD2
#define FAN_ACTIVATOR              RD1

#define VOLTAGE                    0
#define CURRENT                    1
#define CURRENT_DISCHARGE          2
#define TEMPERATURE                3

#define NICD_BATTERY               0
#define NIMH_BATTERY               1
#define SLA_BATTERY                2
#define LIPO_BATTERY               3
#define LIIO_BATTERY               4

#define ACTION_CHARGE              0
#define ACTION_DISCHARGE           1
#define ACTION_CYCLECD             2
#define ACTION_CYCLEDC             3
#define ACTION_SETUP               4

#define MENU_BATTERYTYPE           0
#define MENU_ACTION                1
#define MENU_BATTERYCAPACITY       2
#define MENU_CHARGECURRENTSELECT   3
#define MENU_DISCHCURRENTSELECT    4
#define MENU_BATTERYCELLS          5
#define MENU_SETUPTEMPERATURE      6
#define MENU_SETUPVERSION          7

#define PHASE_ENDED               10

typedef unsigned char BYTE;
typedef unsigned int UINT;

void click(BYTE delay);
void selectbattery(BYTE );

#endif

