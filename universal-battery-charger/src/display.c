/*
* Cargador de baterías universal
* Despligue datos en LCD
* Autor: Felixls - 2009
* Versiones
* Fecha       Versión     Descripción
* 17-09-2009  1.0         Versión inicial
*/
#include "display.h"
#include "menu.h"
#include "chargeprocess.h"
#include "utilities.h"

char numtemp[10];
char numformattemp[7];

// envía por RS232 el inicio de la operación en curso.
void print_initaction(unsigned int current)
{
    switch(mode)
    {
      case NICD_BATTERY: print("NICD-"); break;
      case NIMH_BATTERY: print("NIMH-"); break;
      case SLA_BATTERY:  print("SLA-"); break;
      case LIPO_BATTERY: print("LIPO-"); break;
      case LIIO_BATTERY: print("LIIO-"); break;
    }
    print_uint(batterycapacity);
    print("-");
    print_uint(current);
    print("-");
    print_uint(cells);  
    print("\n");
}

// muestra en el LCD un número entero
// al especificar el parámetro decimal agrega una coma
// en la ubicación necesaria.
void lcd_showint(unsigned int value, char decimal)
{
  uitoa(value, numtemp, 10, decimal);
  lcd_message(numtemp);
}

// muestra en el LCD un número ajustado a la derecha (queda mejor ;)
void lcd_showintright(unsigned int value, int index, char fill)
{
  char *s = numformattemp;
  uitoa(value, numtemp, 10, 0);
  s = padr(numformattemp, numtemp, index, fill);
  lcd_message(s);
}

// envía por RS232 un número en formato ASCII
void print_uint(unsigned int value)
{
  uitoa(value, numtemp, 10, 0);
  print(numtemp);
}

// muestra en el LCD los valores actuales de la batería y la operación en curso.
void showstate()
{
  UINT actualdisplaycurrent;

  actualdisplaycurrent = actualcurrent / 10;
 
  // envía por RS232 los valores de la operación que se está realizando
  // el caracter '>' es una señal para el GUI del administrador.
  if (phase != PHASE_ENDED)
  {
    print(">");
    print_uint(minutes);
    print(":");

    print_uint(seconds);
    print(" ");
    
    print_uint(actualvoltage);
    print("mV ");

    print_uint(actualdisplaycurrent);
    print("mA ");
    
    print_uint(actualtemperature);
    print("C \n");
  }
  
  
/*

**** Menu cargar

|1234567890123456|
 NiMh CHG 0000mAh
  3210mV 13C 0000

**** Menu descargar

 NiMh DIS 0000mAh
  3210mV 13C 0000

**** Menu ciclar

 NiMh C-D 0000mAh      C-D : Carga y luego descarga
  3210mV 13C 0000      D-C : Descarga y luego carga

*/  

  lcd_send(LCD_COMMANDMODE, LCD_LINE1);
  displaybatterytype(mode);
  switch(action)
  {
    case ACTION_CHARGE:
      lcd_message("CHG");
      break;
    case ACTION_DISCHARGE:
      lcd_message("DIS");
      break;
    case ACTION_CYCLECD:
      if (seconds % 2)                        // crea un efecto de parpadeo en el caso de un ciclado
      {
        if (current_action == ACTION_CHARGE)
          lcd_message(" -D");
        else
          lcd_message("C- ");
      }
      else
        lcd_message("C-D");
      break;
    case ACTION_CYCLEDC:
      if (seconds % 2)
      {
        if (current_action == ACTION_CHARGE)
          lcd_message("D- ");
        else
          lcd_message(" -C");
      }
      else
          lcd_message("D-C");
      break;
  }
  lcd_message("  ");
  lcd_showintright(actualdisplaycurrent, 4, 32);
  
  lcd_message("mA");        
  
  lcd_send(LCD_COMMANDMODE, LCD_LINE2);

  lcd_showintright(actualvoltage, 5, 32);
  lcd_message("mV ");

  if (phase == PHASE_ENDED)
  {
    lcd_message("END");
  }
  else
  {
    lcd_showintright(actualtemperature, 2, 32);
    lcd_message("C");
  }
  lcd_showintright(minutes / 60, 2, 32);
  lcd_message(":");
  lcd_showintright(minutes % 60, 2, 48);
}



