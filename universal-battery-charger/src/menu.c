/*
* Cargador de baterías universal
* Menu de opciones
* Autor: Felixls - 2009
* Versiones
* Fecha       Versión     Descripción
* 17-09-2009  1.0         Versión inicial
*/
#include "menu.h"
#include "chargeprocess.h"
#include "dischargeprocess.h"

void displaybatterytype(BYTE m)
{
  switch(m)
  {
    case NICD_BATTERY:lcd_message("NiCd ");break;
    case NIMH_BATTERY:lcd_message("NiMH ");break;
    case SLA_BATTERY: lcd_message("SLA  ");break;
    case LIPO_BATTERY:lcd_message("LiPo ");break;
    case LIIO_BATTERY:lcd_message("LiIo ");break;
  }
}

void displayaction(BYTE a)
{
  switch(a)
  {
    case ACTION_CHARGE:lcd_message("CHARGE");break;
    case ACTION_DISCHARGE:lcd_message("DISCHARGE");break;
    case ACTION_CYCLECD:lcd_message("CYCLE C-D");break;
    case ACTION_CYCLEDC:lcd_message("CYCLE D-C");break;
    case ACTION_SETUP:lcd_message("SETUP");break;
  }
}

void showmenu()
{
  UINT valuemin;              // valor mínimo de un valor a ingresar
  UINT valuemax;              // valor máximo de un valor a ingresar
  UINT menupos;               // la posición del menu
  UINT menuposprev;           // la posición anterior del menu
  BYTE menusel;               // opción de menu elegida
  BYTE nummenu;               // item de menu actual

  BYTE lastButton;            // último botón pulsado (usado para incrementar rápidamente los valores).
  int counter;

  lastButton = 0;
  counter = 0;
  menupos = 0;
  menuposprev = 1;
  
  nummenu = MENU_BATTERYTYPE;
  action = ACTION_CHARGE;
  valuemin = 0;
  valuemax = 0;
  while(1)
  {
    menusel = 0;
    while(menusel==0)
    {
      if (menupos != menuposprev)
      {
        menuposprev = menupos;

        lcd_send(LCD_COMMANDMODE, LCD_CLEAR);
        delay_ms(50);
        lcd_send(LCD_COMMANDMODE, LCD_LINE1);
        switch(nummenu)
        {
          case MENU_BATTERYTYPE:
            lcd_message("Battery Type");
            lcd_send(LCD_COMMANDMODE, LCD_LINE2 | 10);
            valuemin = 0;
            valuemax = 5;
            displaybatterytype(menupos);
            break;
            
          case MENU_ACTION:
            displaybatterytype(mode);
            //CHARGE/DISCHARGE/CYCLE C-D/CYCLE D-C/SETUP
            lcd_send(LCD_COMMANDMODE, LCD_LINE2 | 7);
            valuemin = 0;
            valuemax = 5; 
            displayaction(menupos);
            break;
            
          case MENU_BATTERYCAPACITY:
            displaybatterytype(mode);
            lcd_message("Capacity");
            lcd_send(LCD_COMMANDMODE, LCD_LINE2 | 9);
            valuemin = 0;
            valuemax = 60000;                          // rango de 0 a 6000A

            lcd_showint(menupos, 1);
            lcd_message("Ah");
            break; 
            
          case MENU_BATTERYCELLS:
          case MENU_CHARGECURRENTSELECT:
          case MENU_DISCHCURRENTSELECT:
            displaybatterytype(mode);
            displayaction(action);
            break;
            
          case MENU_SETUPTEMPERATURE:
            lcd_message("TEMPERATURE");
            lcd_send(LCD_COMMANDMODE, LCD_LINE2 | 14);
            readbatteryinfo();
            lcd_showint(actualtemperature, 0);
            lcd_message("C");
            break;
            
          case MENU_SETUPVERSION:
            lcd_message(BATTERYCHARGER_VERSION);
            break;
            
        }

        switch(nummenu)
        {
          case MENU_CHARGECURRENTSELECT:
            lcd_send(LCD_COMMANDMODE, LCD_LINE2);
            lcd_message("CHR.CURR");
            valuemin = 0;
            valuemax = 11;
            lcd_send(LCD_COMMANDMODE, LCD_LINE2 | 12);
            if (menupos<10)
            {
              lcd_message("0.");
              lcd_showint(menupos, 0);
              lcd_message("C");
            }
            else
              lcd_message("1.0C");            
            break;
            
          case MENU_DISCHCURRENTSELECT:
            lcd_send(LCD_COMMANDMODE, LCD_LINE2);
            lcd_message("DSCH.CURR");
            valuemin = 0;
            valuemax = 1000;
            lcd_send(LCD_COMMANDMODE, LCD_LINE2 | 10);
            lcd_showint(menupos, 0);
            lcd_message("mA");
            break;
            
          case MENU_BATTERYCELLS:
            lcd_send(LCD_COMMANDMODE, LCD_LINE2);
            valuemin = 1;
            
            switch(mode)
            {
              case NICD_BATTERY:
              case NIMH_BATTERY:
                valuemax = 15;
                break;
              case SLA_BATTERY:
                valuemax = 2;
                break;
              case LIPO_BATTERY:
                valuemax = 5;
                break;
              case LIIO_BATTERY:
                valuemax = 9;
                break;                
            }            

            lcd_message("CELLS");
            lcd_send(LCD_COMMANDMODE, LCD_LINE2 | 14);
            lcd_showint(menupos, 0);
            break;
            
        }
      }

      if (UP_BUTTON==0)
      {
        delay_ms(50);
        if (UP_BUTTON==0)
        {
          counter = 0;
          if (lastButton != 1)          // si el usuario la vez anterior pulsó el botón más de 2000ms (2segundos)
          {                             // entra en modo de ingreso rápido.
            while(UP_BUTTON==0) 
            {            
              counter++;
              delay_ms(1);
              if (counter > 2000)
              {
                lastButton = 1;
                break;
              }             
            }
            if (counter < 2000)
            {
              lastButton = 0;                    
              click(30);
            }
          }

          if (menupos > valuemin)
            menupos--;           
        }
      }

      if (DOWN_BUTTON==0)
      {
        delay_ms(50);
        if (DOWN_BUTTON==0)
        {
          counter = 0;
          if (lastButton != 2)
          {
            while(DOWN_BUTTON==0) 
            {            
              counter++;
              delay_ms(1);
              if (counter > 2000)
              {
                lastButton = 2;
                break;
              }             
            }
            if (counter < 2000)
            {
              lastButton = 0;          
              click(30);
            }
          }

          if (menupos < valuemax - 1)
            menupos++;

        }
      }

      if (OK_BUTTON==0)
      {
        lastButton = 0;
        delay_ms(50);
        if (OK_BUTTON==0)
        {
          while(OK_BUTTON==0);

          click(30);

          menusel = 1;
        }
      }
      
      if (MENU_BUTTON==0)
      {
        lastButton = 0;
        delay_ms(50);
        if (MENU_BUTTON==0)
        {
          while(MENU_BUTTON==0);

          click(30);

          menusel = 2;
        }
      }
    }
    
    // si el usuario pulsó ENTER, se ejecuta la opción y pasa al menú siguiente
    if (menusel == 1)
    {
      switch(nummenu)
      {
        case MENU_BATTERYTYPE:
          selectbattery(menupos);
          nummenu++;
          menupos = action;
          break;
          
        case MENU_ACTION:
          action = menupos;
          if (action != ACTION_SETUP)
          {
            nummenu++;
            menupos = batterycapacity;
          }
          else
          {
            nummenu = MENU_SETUPTEMPERATURE;
            menupos = 0;
          }
          break;
          
        case MENU_BATTERYCAPACITY:
          batterycapacity = menupos;
          switch(action)
          {
            case ACTION_CYCLECD:
            case ACTION_CYCLEDC:
            case ACTION_CHARGE:
              menupos = chargecurrent;
              break;
            case ACTION_DISCHARGE:
              menupos = dischargecurrent;
              break;
          }          
          if (action != ACTION_DISCHARGE)
            nummenu++;
          else
            nummenu = MENU_DISCHCURRENTSELECT;
          break;
          
        case MENU_CHARGECURRENTSELECT:
          chargecurrent = menupos;
          if (action != ACTION_CHARGE)
          {
            menupos = dischargecurrent;
            nummenu++;
          }
          else
          {
            nummenu = MENU_BATTERYCELLS;
            menupos = cells;  
          }
          break;
          
        case MENU_DISCHCURRENTSELECT:
          dischargecurrent = menupos;
          nummenu++;
          menupos = cells;          
          break;
          
        case MENU_BATTERYCELLS:
          cells = menupos;                

          // comienza la operacion
          switch(action)
          {
            case ACTION_CYCLECD:
            case ACTION_CHARGE: 
              charge();
              break;
            case ACTION_CYCLEDC:
            case ACTION_DISCHARGE: 
              discharge();
              break;
          }
          if (action == ACTION_CYCLECD)
            discharge();
          else if (action == ACTION_CYCLEDC)
            charge();
          
          FAN_ACTIVATOR = 0;
          LED_OPCOMPLETE = 0;

          menupos = mode;
          nummenu = MENU_BATTERYTYPE;
          break;
          
        case MENU_SETUPTEMPERATURE:
          nummenu++;
          break;
          
        case MENU_SETUPVERSION:
          nummenu = MENU_BATTERYTYPE;
          menupos = 0;
          break;
      }
    
    }
    else   // el usuario desea volver al menu anterior...
    {
      if (nummenu > 0)
      {
        if (nummenu == MENU_SETUPTEMPERATURE)
          nummenu = 0;
        else
        {
          nummenu--;
          switch(nummenu)
          {
            case MENU_BATTERYTYPE:
              menupos = mode;
              break;
              
            case MENU_ACTION:
              menupos = action;
              break;
              
            case MENU_BATTERYCAPACITY:
              menupos = batterycapacity;
              break;
              
            case MENU_CHARGECURRENTSELECT:
              if (action == ACTION_DISCHARGE)
              {
                nummenu--;
                menupos = batterycapacity;
              }
              else
                menupos = chargecurrent;
              break;
              
            case MENU_DISCHCURRENTSELECT:
              if (action == ACTION_CHARGE)
              {
                nummenu--;
                menupos = chargecurrent;
              }
              else
                menupos = dischargecurrent;
              break;
              
            case MENU_BATTERYCELLS:
              menupos = cells;
              break;
              
            default:
              menupos = 0;
              break;
          }
        }
      }
    }

    menuposprev = 0xFFFF;
  }

}

