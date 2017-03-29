/*
* Cargador de baterías universal
* Menu de opciones
* Autor: Felixls - 2009
*/
#ifndef MENU_H
#define MENU_H

#include "charger.h"

extern BYTE action;
extern BYTE actualtemperature;

extern void selectbattery(BYTE );
extern void readbatteryinfo();
extern void writetoserie();

void displaybatterytype(BYTE m);

void showmenu();


#endif

