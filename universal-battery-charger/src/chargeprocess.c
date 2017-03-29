/*
* Cargador de baterías universal
* Proceso de carga
* Autor: Felixls - 2009
* Versiones
* Fecha       Versión     Descripción
* 17-09-2009  1.0         Versión inicial
* 18-09-2009  1.1         Corrección en algoritmo de carga de baterías SLA/LiXX
*/

#include "chargeprocess.h"
#include "display.h"
#include "menu.h"

int last_min;
int last_min_volt;
int last_min_temp;
BYTE test_voltage;

// incrementa el valor del ciclo útil del PWM
// afecta al mosfet que funciona junto con el diodo, el inductor y capacitor
// como un Buck converter.
// De esta manera es posible regular la corriente de forma más eficiente (aprox 90%).
// Dado este diseño, la carga de las baterías es con un pulso suave.
void pwm2_set_data()
{
  unsigned char tmp=0;
  unsigned int tmp2=0;;

  tmp2 = (pwmduty2 / 4);
  CCPR2L = (unsigned char) tmp2;
  tmp2 = (pwmduty2 & 0x0003);
  tmp = (unsigned char) tmp2;
  CCP2X = (tmp >> 1) & 0x01;
  CCP2Y = tmp & 0x01;
}

void pwm1_set_data()
{
  unsigned char tmp=0;
  unsigned int tmp2=0;;

  tmp2 = (pwmduty / 4);
  CCPR1L = (unsigned char) tmp2;
  tmp2 = (pwmduty & 0x0003);
  tmp = (unsigned char) tmp2;
  CCP1X = (tmp >> 1) & 0x01;
  CCP1Y = tmp & 0x01;
}

void incduty(int value)
{
  pwmduty += value;
  if (pwmduty > 255)
    pwmduty = 255;
}

void decduty(int value)
{
  if (pwmduty < value)
    pwmduty = 0;
  else
    pwmduty -= value;
}

void stopCharge()
{
  pwmduty = 0;
  pwm1_set_data();
  terminate = 1;
}

//dTdt - La temperatura por minuto excede 1 grado por minuto.
//dVdt - Detección de pico de voltaje. Espera inicial 10 minutes
//High voltage - 1.68 volts por celda
//Timeout - 65 minutes a 1C, 130 minutes a 0.5C, etc.
void chargeNiXX()
{
  //Condiciones de fin

  //Timeout
  if (minutes > timeout)
  {
    print("#END:TO\n");
    stopCharge();
    return;
  }  

  // Voltaje máximo
  if (actualvoltage >= voltagecontrol)
  {
    test_voltage++;
    if (test_voltage>=4)
    {
      print("#END:VM\n");
      stopCharge();
      return;      
    }
  }  
  else
    test_voltage = 0;

  if (minutes != last_min)
  {
    last_min = minutes;

    // -dV/dt
    if (minutes >= NDVwait && last_min_volt>0)
    {
      if (adc_voltage <= last_min_volt - 1)
      {
        print("#END:DVDT\n");
        stopCharge();
        return;
      }
    }
 
    last_min_volt = adc_voltage;

    // dTdt
    if (last_min_temp > 0 && adc_temperature >= last_min_temp + 3)
    {
      print("#END:DTDT\n");
      stopCharge();
      return;
    }
    last_min_temp = adc_temperature;
  }

  if (actualcurrent < chargecurrentcontrol)
    incduty(1);
  else if (actualcurrent > chargecurrentcontrol)
    decduty(1);
}

//Método de carga de baterías de ácido de plomo:
//1. Verificar si la batería.
//2. Si está bien, iniciar la carga a corriente constante a capacidad/10.
//3. Cuando el voltaje alcance 2.55V/celda cambiar a carga por voltaje constante a 2.45V/celda.
//4. Si la corriente cae por debajo de capacidad/20 entonces cambiar a carga flotante.
//5. Cargar en forma flotante a 2.25V/celda por tiempo indefinido (máximo recomendable 20 horas).

//Método de carga de LiPo y LiIon:
// 1- Cargar con corriente constante (de 0 a 1C) hasta que el voltaje alcance los 4.2V por celda (en algunas celdas viejas a los 4.1)
// 2- Cargar con voltaje constante a 4.2V por celda hasta que la corriente caiga a capacidad/15.
// 3- Cargar flotante a capacidad/30 por 30 minutes.
void chargeSLAandLIXX()
{
  //Timeout
  if (minutes > timeout && mode==SLA_BATTERY)
  {
    print("#END:TO\n");
    stopCharge();
    return;
  }  

  if (actualvoltage >= voltagecontrol && phase == 0)
  {
    click(0xFF);
    
    decduty(1);
    pwm1_set_data();
    delay_ms(5000);
    decduty(1);
    pwm1_set_data();
    delay_ms(5000);
  
    phase = 1;
    return;
  }

  switch (phase)
  {
    //Iniciar la carga a corriente constante a capacidad/10.
    case 0:
      if (actualcurrent < chargecurrentcontrol)
        incduty(1);
      else if (actualcurrent > chargecurrentcontrol)
        decduty(1);
     
      break;
      
    case 1:
      if (actualvoltage > normalchargevoltagecontrol)
        decduty(1);
      else if (actualvoltage < normalchargevoltagecontrol)
        incduty(1);

      if (actualcurrent < finalcurrentcontrol)
      {
        // en lugar de terminar podría continuar con carga flotante por tiempo indefinido.
        print("#END:CC\n");
        stopCharge();
        return;
      }
     
      break;      
  }
}

void charge()
{
  terminate = 0;
  
  pwmduty = 0;
  pwmduty2 = 0;
  pwm1_set_data();
  pwm2_set_data();  

  readbatteryinfo();
  
  if (actualvoltage == 0 && actualcurrent > 0)    // SHORT CIRCUIT!!!
  {
    click(0xFF);
    return;
  }
  else
  {
    switch(mode)
    {
      case NICD_BATTERY:
      case NIMH_BATTERY:
        if (actualtemperature < mintemphighcurrent && chargecurrent >= 5)  // si la temperatura no es la adecuada para una carga rápida...
          chargecurrent = 1; // baja la carga a 0.1C
  
        timeout = (65 * (1000/(chargecurrent*10)))/10;  // 65min para 1C, 130min para 0.5C
        break;
        
      case SLA_BATTERY:
        if (actualtemperature > maxtemphighcurrent && chargecurrent >= 5)
          chargecurrent = 1; // baja la carga a 0.1C

        finalcurrent = batterycapacity * 5 * 10;    // ojo!!: (finalcurrent está multiplicado por 10)
        timeout = (batterycapacity / (batterycapacity*chargecurrent))*600;  //  timeout = (capacidad de bateria/carga)*60
        break;
      
      case LIPO_BATTERY:
      case LIIO_BATTERY:
        finalcurrent = batterycapacity * 3 * 10;     // ojo!!: (finalcurrent está multiplicado por 10)
        break;
      
    }  
    
    current_action = ACTION_CHARGE;
    
    test_voltage = 0;
    
    mseconds = 0;
    seconds = 0;
    minutes = 0;

    LED_OPCOMPLETE = 0;

    voltagecontrol = cellmaxvoltage * cells;
    normalchargevoltagecontrol = normalchargevoltage * cells;
    if (batterycapacity * chargecurrent > maxcurrent / 10)
      chargecurrentcontrol = maxcurrent * 10;                         // corriente de carga a controlar * 10
    else
      chargecurrentcontrol = batterycapacity * 100 * chargecurrent;   // corriente de carga a controlar * 10
    finalcurrentcontrol = finalcurrent;                              // corriente final a controlar * 10
    
    if (chargecurrentcontrol >= 10000)  // si la corriente de carga es mayor a 1A activa ventilación
      FAN_ACTIVATOR = 1;        

    last_min = 0;
    last_min_volt = 0;
    last_min_temp = 0;
    
    print("#CHR:");
    print_initaction(chargecurrent);
    
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
        
        if (mode == NICD_BATTERY || mode == NIMH_BATTERY)
          chargeNiXX();
        else
          chargeSLAandLIXX();
        
        //Temperatura máxima y mínima de baterias
        if (actualtemperature > abs_max_temp)
        {
          print("#END:HT\n");
          stopCharge();
        } 
        else if (actualtemperature < abs_min_temp)
        {
          print("#END:LT\n");
          stopCharge();
        }

        pwm1_set_data();        
        LED_CHARGEDISCHARGE = 1 - LED_CHARGEDISCHARGE;
      }
    }

    LED_CHARGEDISCHARGE = 0;
    LED_OPCOMPLETE = 1;

    pwmduty = 0;
    pwm1_set_data();

    print("#CHR-END\n");

    click(0xFF);
    delay_ms(50);
    click(0xFF);
    delay_ms(50);
    click(0xFF);

    terminate = 0;
    phase = PHASE_ENDED;
   
    if (action != ACTION_CHARGE && action != ACTION_CYCLEDC)
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


