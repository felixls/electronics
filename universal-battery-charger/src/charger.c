/*
* Cargador de baterías universal
* Proceso de carga
* Autor: Felixls - 2009
* Versiones
* Fecha       Versión     Descripción
* 17-09-2009  1.0         Versión inicial
* 19-09-2009  1.2         La falta de inicialización de las variables de pwm duty
*                         en las lectura de temperatura (menu SETUP), causaba el inicio
*                         de la carga y descarga.
*                         Ajuste en el cálculo de la temperatura.
*/

#include "charger.h"

typedef unsigned int word;
word at 0x2007 CONFIG = _HS_OSC & _WDT_OFF & _PWRTE_ON & _BODEN_OFF & _LVP_OFF & _CPD_OFF & _WRT_OFF & _DEBUG_OFF & _CP_OFF;

#include "menu.h"
#include "chargeprocess.h"

UINT adc_voltage;
UINT adc_current;
UINT adc_discharge_current;
UINT adc_temperature;
BYTE adc_channel;

BYTE mode;
BYTE action;
BYTE current_action;

BYTE mseconds, seconds;
UINT minutes;

UINT maxcurrent;                    // mA

BYTE cells;
UINT batterycapacity;              // mAh / 100
UINT chargecurrent;                // 1 = 0.1 C
UINT dischargecurrent;             // mA

UINT cellmaxvoltage;                // mV
UINT voltagecutoff;                 // mV/celda
BYTE NDV;                           // mV
BYTE NDVwait;                       // minutos
UINT finalcurrent;                  // %
UINT normalchargevoltage;           // mV
UINT timeout;                       // minutos
UINT mintemphighcurrent;            // temperatura mínima en cargas a altas corrientes
UINT maxtemphighcurrent;            // temperatura máxima en cargas a altas corrientes

int abs_max_temp;
int abs_min_temp;

UINT pwmduty;                             // Valor de CCPR1L:CCP1CON<5,4> para variar el duty cycle
                                          // de acuerdo a los movimientos del encoder (modo voltaje).
UINT pwmduty2;

BYTE terminate;
BYTE phase;

UINT voltagecontrol;
UINT normalchargevoltagecontrol;
UINT voltagecutoffcontrol;
UINT chargecurrentcontrol;
UINT finalcurrentcontrol;

UINT actualvoltage;
UINT actualcurrent;
BYTE actualtemperature;

/*
Datos

Corriente máxima de carga:    0 a 5A
Corriente máxima de descarga: 1A     ( I = V/R -> I = 12/10 ->  I=1.2A)
Modos:                        0:NiCd, 1:NiMh, 2:SLA, 3:LiPo, 4: LiIo
Capacidad de batería:         3000mAh
Número de celdas: (1 a 19)    6

Carga:    (1 a 10)           (10)  -> 3000*1.0=3A
Descarga:                     0 a 1A

Espera delta pico:            10 minutos

Temperatura mínima en
altas corrientes (>0.5C)      10 grados

Corte por bajo voltaje (por celda):
  NiCd (0 a 2550)             800mV
  NiMh (0 a 2550)             1000mV
  LiPo (2500 a 3500)          3000mV
  SLA  (1500 a 2500)          2000mV

Delta pico (0 a 255):
  NiCd                        40mV
  NiMh                        20mV

Voltaje máximo por celda:
  NiCd                        1680mV
  NiMh                        1680mV
  LiPo (3500 a 4500)          4200mV
  SLA (2000 a 3000)           2500mV   (2.5V x 6 celdas = 15V - voltaje máximo para SLA de 12V)

Voltaje carga normal por celda:
  LiPo (3500 a 4500)          4200mV
  SLA (2000 a 3000)           2450mV   (2.45V x 6 celdas = 14.7V)

Corriente final (% de la corriente de carga inicial):
  LiPo                        (3%)   -> 3000*3/100=90mA
  SLA                         (5%)   -> 3000*5/100=150mA

Timeout
  NiCd y NiMh                 65 min a 1C, 130 min para 0.5C
  LiPo                        30 min en flotante
  SLA                         25 horas
  
*/

// interrupción cada 5ms
// usada para contar milisengundos, segundos y minutos.
void intr() __interrupt 0
{
  if (T0IF)                                        // si se trata de una int. por desborde del timer 0.
  {
    TMR0 = 60;                                     // una interrupción cada 5ms
    T0IF = 0;                                      // Borro Bandera de Desborde
    
    if (phase != PHASE_ENDED)
    {
      mseconds++;
      if (mseconds == 200)                          // un segundos = 200 interrupciones
      {
        mseconds = 0;
        seconds++;
      }
      if (seconds == 60)
      {
        seconds = 0;
        minutes++;
      }
    }      

  }
}

// estabiliza los valores del conversor A/D
void normalizeADC()
{
  int V[4];
  char i;
  int Vmax, Vmin;
  
  for(Vmax = 10, Vmin = 0; Vmax <= (Vmin+1);)
  {
    V[3] = V[2];
    V[2] = V[1];
    V[1] = V[0];
    V[0] = adc_read(adc_channel);
    Vmin = V[0];
    Vmax = V[0];
    for (i=0; i<=3; i++)
    {
      if (V[i] > Vmax)
        Vmax = V[i];
      if (V[i] < Vmin)
        Vmin = V[i];
    }
  }
}

// lee los valores de la batería
// para leer el voltaje hay que para el PWM para que la carga
// no afecte a la medición del divisor.
int batteryinfo(unsigned char value)
{
  int adcvalue;
  char i;
  UINT pwmduty_prev = 0;
  UINT pwmduty2_prev = 0;
  
  adc_channel = value;
 
  if (value == VOLTAGE)
  {
    pwmduty_prev = pwmduty;
    pwmduty2_prev = pwmduty2;
    
    pwmduty = 0;
    pwmduty2 = 0;
    
    pwm1_set_data();
    pwm2_set_data();
    
    normalizeADC();
    
    if (current_action == ACTION_CHARGE)
      delay_ms(100);
  }
  
  for (adcvalue=0, i=8; i; --i)
    adcvalue += adc_read(adc_channel);    
  
  adcvalue = adcvalue/8;
  
  if (value == VOLTAGE)
  {
    pwmduty = pwmduty_prev;
    pwmduty2 = pwmduty2_prev;
    
    pwm1_set_data();
    pwm2_set_data();  
  }  
  
  return adcvalue;
}

// lee y calcula los valores de la batería
void readbatteryinfo()
{
  adc_current = batteryinfo(CURRENT);
  adc_discharge_current = batteryinfo(CURRENT_DISCHARGE);  
  adc_temperature = batteryinfo(TEMPERATURE);
  
  adc_voltage = batteryinfo(VOLTAGE);
  
  actualvoltage = adc_voltage * 20;
  actualtemperature = (adc_temperature * 53)/100;

  if (current_action == ACTION_CHARGE)
    actualcurrent = adc_current * 71;
  else
    actualcurrent = adc_discharge_current * 18; 
}

// BEEEEP
void click(BYTE delay)
{
  PIEZO = 1;
  delay_ms(delay);
  PIEZO = 0;
}

void selectbattery(BYTE batterytype)
{
  mode = batterytype;

  abs_max_temp = 40;
  abs_min_temp = 5;
  dischargecurrent = 100;
  
  //TODO: Estos son los valores por defecto para las diferentes baterías
  //      podría ser útil que se pudieran grabar en la eeprom interna
  //      y poder cambiarlas desde el software de la pc.
  
  switch(mode)
  {
    case NICD_BATTERY:
    case NIMH_BATTERY:
      batterycapacity = 25;                      // pilas AA de ej.: 2500mAh
      cells = 2;
      chargecurrent = 10;                        // 0.5 C para NiMh
      cellmaxvoltage = 1680;
      voltagecutoff = 1000;
      NDV = 20;
      NDVwait = 10;
      mintemphighcurrent = 10;
      break;    
      
    case SLA_BATTERY:
      batterycapacity = 12;                       // SLA de ejemplo: 1200mAh
      cells = 6;
      chargecurrent = 1;                          // 0.1 C para SLA
      cellmaxvoltage = 2500;
      normalchargevoltage = 2450;
      voltagecutoff = 1666;
      maxtemphighcurrent = 30;
      break;
      
    case LIPO_BATTERY:
      batterycapacity = 70;
      cells = 6;
      chargecurrent = 10;                         // 1 C para LiPo
      cellmaxvoltage = 4200;
      normalchargevoltage = 4200;
      voltagecutoff = 2500;
      break;
      
    case LIIO_BATTERY:
      batterycapacity = 9;                        // ejemplo una batería de celular de 3.6v de 900mAh
      cells = 1;                                  // las baterías de notebook normalmente son de 6 o 12 celdas
      chargecurrent = 3;                          // 0.3C para LiIon
      cellmaxvoltage = 4200;
      normalchargevoltage = 4200;
      voltagecutoff = 2500;
      break;      
  }
}

void main(void)
{
  GIE = 0;                               // Deshabilita interrupciones

  delay_ms(2000);

  TRISA=0xFF;                           // PORTA como entrada.
  TRISB=0b00000001;                     // PORTB como salida, RB0 como entrada
  TRISC=0b00000000;                     // PORTC como salida
  TRISD=0b11110000;                     // PORTD: RD4-7 entrada, RD0-4 salida
  TRISE=0x00;
  
  delay_ms(500);

  // Limpieza de registros
  PORTA=0x00;
  PORTB=0x00;
  PORTC=0x00;
  PORTD=0x00;
  PORTE=0x00;

  init_serie();
  print("\#Hello\n");                   // hello para la PC

  adc_current = 0;
  adc_discharge_current = 0;
  adc_init(FOSC_32, A5_R0, INT_OFF);

  // Configuración de PWM:
  PR2 = 0b01001101;                      // 16khz - 10bits - Prescaler=4
  T2CON = 0b00000101;

  CCPR1L = 0b00000000;                  // duty cycle incial en 0.
  CCP1CON = 0b00001100;

  CCPR2L = 0b00000000;                  // duty cycle incial en 0.
  CCP2CON = 0b00001100;

  delay_ms(250);

  maxcurrent = 5000;         // Este modelo fue diseñado para 5A de carga máxima.
 
  LCD_BACKLIGHT = 1;
  lcd_init();

  lcd_send(LCD_COMMANDMODE, LCD_CLEAR);
  delay_ms(1000);
  lcd_message("Battery Charger");
  delay_ms(10);
  lcd_send(LCD_COMMANDMODE, LCD_LINE2);
  delay_ms(10);
  lcd_message("Felixls 2009 :)");


  pwmduty = 0;                // Fix en 1.2 faltaba inicializar la variable duty en 0.
  pwmduty2 = 0;               // Fix en 1.2 faltaba inicializar la variable duty en 0.

  delay_ms(5000);

  // Configuración de TMR0 como temporizador
  // Temporizacion = Tcm * Prescaler * (256 - Carga TMR0)

  // Dada la frecuencia del oscilador de 20mhz:
  // 5ms = 5000us = 0.2 * 128 * (256 - Carga TMR0)  -->   Carga TMR0 = 60
  OPTION_REG = 0x06;                                     // Prescaler = 1:128
  
  T0IE = 1;                                              // activa interrupción por Tmr0
  T0IF = 0;                                              // borro bandera de desborde

  GIE=1;                                                 // Habilito Interrupciones

  // loop del menu
  showmenu();
}
