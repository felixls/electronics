/*
* Cargador de baterías universal
* Proceso de descarga
* Autor: Felixls - 2009
* Versiones
* Fecha       Versión     Descripción
* 17-09-2009  1.0         Versión inicial
*/

#include "dischargeprocess.h"
#include "display.h"
#include "menu.h"


// incrementa el valor del ciclo útil del PWM
// afecta al mosfet conectado a la resistencia de 10ohm 15W
// usado para no estresar la batería y para no prender fuego la resistencia
// con baterías de 12v o más...
void incduty2(int value)
{
  pwmduty2 += value;
  if (pwmduty2 > 255)
    pwmduty2 = 255;
}

void decduty2(int value)
{
  if (pwmduty2 < value)
    pwmduty2 = 0;
  else
    pwmduty2 -= value;
}


// Importante
// Al utilizar una resistencia de 10ohm para descarga
// si se intenta descargar una batería de ácido de plomo
// causa picos de 1A no son agradables para este tipo de baterías.
//
// *** No olvidar que está desaconsejado el ciclado o la descarga profunda de baterías de ácido de plomo. ***
//
// Para los otros tipos de baterías se recomiendan las descargas a 0.2C
// que llevan a las celdas a voltaje de corte en 5 horas.
void discharge()
{
  UINT dischargecurrentcontrol;
  
  terminate = 0;
  
  pwmduty = 0;
  pwmduty2 = 0;
  pwm1_set_data();
  pwm2_set_data();
  
  readbatteryinfo();

  // una protección contra corto circuito muy mala, pero es preferible a nada...
  if (actualvoltage == 0 && actualcurrent > 0)   
  {
    click(0xFF);
    return;
  }
  else
  {   
    current_action = ACTION_DISCHARGE;

    mseconds = 0;
    seconds = 0;
    minutes = 0;

    LED_OPCOMPLETE = 0;

    voltagecutoffcontrol = voltagecutoff * cells;
    dischargecurrentcontrol = dischargecurrent * 10;
    
    FAN_ACTIVATOR = 1;        

    print("#DIS-");
    print_initaction(dischargecurrent);

    phase = 0;

    while(terminate == 0)
    {
      showstate();
      
      if (OK_BUTTON==0)
      {
        delay_ms(50);
        if (OK_BUTTON==0)
        {
          while(OK_BUTTON==0);

          click(100);

          print("#END:UC\n");
          terminate = 1;        
        }
      }

      if (terminate == 0)
      {
        readbatteryinfo();
       
        if (actualvoltage < voltagecutoffcontrol)
        {
          print("#END:DC\n");
          pwmduty2 = 0;
          terminate = 1;
        }
        else
        {
          if (actualcurrent < dischargecurrentcontrol)
            incduty2(1);
          else if (actualcurrent > dischargecurrentcontrol)
            decduty2(1);
        }

        pwm2_set_data();
          
        LED_CHARGEDISCHARGE = 1 - LED_CHARGEDISCHARGE;
      }
    }

    LED_CHARGEDISCHARGE = 0;
    LED_OPCOMPLETE = 1;

    pwmduty2 = 0;
    pwm2_set_data();
     
    print("#DIS-END\n");

    click(0xFF);
    delay_ms(50);
    click(0xFF);
    delay_ms(50);
    click(0xFF);

    terminate = 0;
    phase = PHASE_ENDED;
      
    if (action != ACTION_DISCHARGE && action != ACTION_CYCLECD)
      return;
  }

  FAN_ACTIVATOR = 0;
  while(terminate == 0)
  {
    readbatteryinfo();
    
    showstate();
    
    if (OK_BUTTON==0)
    {
      delay_ms(50);
      if (OK_BUTTON==0)
      {
        while(OK_BUTTON==0);

        click(30);

        terminate = 1;
      }
    }
  }  
}

