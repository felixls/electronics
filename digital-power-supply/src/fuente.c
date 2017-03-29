//-------------------------------------------------------------------------------------------------
// FUENTE.C
// Fuente de alimentación digital
//
// Fecha de inicio: 02/04/2009
// Autor: Felixls
// Web: http://sergiols.blogspot.com
// Basado en: ASM de Osvaldo valdorre (http://www.todopic.com.ar/foros/index.php?topic=18804.0)
//
// Changelog:
// Fecha   Versión     Observaciones
// 3/4/09  0.01        Versión inicial
//
// Frecuencia de reloj: 4MHZ
// Compilador: SDCC 2.9.1
//-------------------------------------------------------------------------------------------------

#include <pic/pic16f877a.h>

/* ----------------------------------------------------------------------- */
/* Bits de configuración */

typedef unsigned int word;
word at 0x2007 CONFIG = _XT_OSC & _WDT_OFF & _PWRTE_ON & _BODEN_OFF & _LVP_OFF & _CPD_OFF & _WRT_OFF & _DEBUG_OFF & _CP_OFF;

//#define USAR_PULSADORES                           // indica que se va a usar pulsadores o un encoder mecánico,
                                                    // en lugar del encoder optico (sirve además para simulación)
//#define MOSTRARPWM                                // en lugar de la sección de memorias "M:x", muestra el duty (0-1024)

#include "delay.h"
#include "lcd.h"
#include "adc.h"
#include "serie.h"
#include "bin2dec.h"
#include "calculo.h"

// ** SALIDAS **
#define LEDCORTE         RD0                      // led indicador de corte por sobre corriente
#define CONTROL_FAN      RD1                      // Turbina de refrigeración
#define LCD_BACKLIGHT    RD2                      // Backlight del LCD
#define CORTE_CORRIENTE  RD3                      // Control del corte por sobre corriente

#define ENC_CANAL_A      RD4                      // Entrada encoder canal A
#define ENC_CANAL_B      RD5                      // Entrada encoder canal B
#define BOTON_AJUSTE     RD6                      // Botón de ajuste grueso/fijo
#define PIEZO            RD7                      // Buzzer de corte de corriente

// ** ENTRADAS **

#define BOTON_VOLTAJE    RC3                      // Botón de selección de voltaje
#define BOTON_MEMORIA    RC4                      // Botón de selección de memorias
#define BOTON_AMPERES    RC5                      // Botón de selección de amperaje

// ** MODOS DE OPERACION **
#define MODO_NORMAL      0
#define MODO_VOLTAJE     1
#define MODO_MEMORIA     2
#define MODO_AMPERES     3
#define MODO_TEMPERATURA 4

unsigned char modo;                               // Modo de operación (1=Voltaje, 2=Amperaje, 3=Memoria)
unsigned char cortecorriente;                     // Indicador de corte por alta corriente
unsigned char grabarmemoria;                      // Indicador de grabado de valores en memoria EEPROM

unsigned int pwmduty;                             // Valor de CCPR1L:CCP1CON<5,4> para variar el duty cycle
                                                  // de acuerdo a los movimientos del encoder (modo voltaje).

unsigned char incremento_enc;                     // valor del incremento (1=100 mv , 40=1v)
unsigned char i;
unsigned int temporizador;
unsigned char temporizador_luz;

unsigned char valor_viejo_canalA;                 // valor anterior del canal A del encoder (LSB)
unsigned char valor_viejo_canalB;                 // valor anterior del canal B del encoder (MSB)
unsigned char readA;                              // valor del encoder
unsigned char readB;                              // valor del encoder
unsigned char readenc;                            // Bit 4 y 5 del puerto D (datos del encoder).

unsigned char num_memoria;                        // número de memoria (voltaje prefijado) activo.

unsigned int amperes;                             // amperes de corte seleccionado.

unsigned int adc_corriente;                       // valor de corriente leido
unsigned int adc_tension;
unsigned int adc_temperatura;
unsigned int adc_temp;
unsigned char adc_channel;

unsigned char max_memoria = 6;

// Inicia el temporizador
void iniciar_tempo(void)
{
  LCD_BACKLIGHT = 1;
  temporizador_luz = 5;                           // 50 segundos
  temporizador = 4580;                            // 10 segundos
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

/*
  Rutina de Interrupciones
*/
void intr() __interrupt 0
{
  if (T0IF)                                        // si se trata de una int. por desborde del timer 0.
  {
    T0IF = 0;                                      // Borro Bandera de Desborde
    TMR0 = 0;                                      // cargo valor en TMR0

    if (BOTON_VOLTAJE==0)
    {
      delay_ms(50);
      if (BOTON_VOLTAJE==0)
      {
        while(BOTON_VOLTAJE==0);
        PIEZO = 1;
        delay_us(500);
        PIEZO = 0;
        modo = MODO_VOLTAJE;
        iniciar_tempo();
      }
    }

    if (BOTON_MEMORIA==0)
    {
      delay_ms(50);
      if (BOTON_MEMORIA==0)
      {
        while(BOTON_MEMORIA==0);
        PIEZO = 1;
        delay_us(500);
        PIEZO = 0;
        if (modo == MODO_MEMORIA)
          modo = MODO_TEMPERATURA;
        else
            modo = MODO_MEMORIA;
        iniciar_tempo();
      }
    }

    if (BOTON_AMPERES==0)
    {
      delay_ms(50);
      if (BOTON_AMPERES==0)
      {
        while(BOTON_AMPERES==0);
        PIEZO = 1;
        delay_us(500);
        PIEZO = 0;
        modo = MODO_AMPERES;
        iniciar_tempo();
      }
    }

    if (BOTON_AJUSTE == 0)
    {
      delay_ms(50);
      if (BOTON_AJUSTE==0)
      {
        while(BOTON_AJUSTE==0);
        if (incremento_enc == 40)
          incremento_enc = 1;
        else
          incremento_enc = 40;
        PIEZO = 1;
        delay_ms(incremento_enc);
        PIEZO = 0;
      }
    }

    if (modo == MODO_NORMAL)
      goto continua;

#ifdef USAR_PULSADORES

    // lectura de pulsadores o encoder mecánico.
    if (ENC_CANAL_A == 0)
    {
      while(ENC_CANAL_A == 0);
      delay_ms(50);
      goto incvalor;
    }
    if (ENC_CANAL_B == 0)
    {
      while(ENC_CANAL_B == 0);
      delay_ms(50);
      goto decvalor;
    }
    goto continua;

#else

    // lectura de encoder óptico (ruedita del mouse, por ej.)
    readenc = PORTD;
    readA = ((readenc & 0b00100000) >> 5) ^ 0xFF;
    readB = ((readenc & 0b00010000) >> 4) ^ 0xFF;

    //  sentido horario -->
    //     00 01 11 10 00
    //  <-- sentido anti-horario

    if (readA == valor_viejo_canalA && readB == valor_viejo_canalB)
      goto continua;

    if (readA == 0 && readB == 0 && valor_viejo_canalA == 1 && valor_viejo_canalB ==1)
      goto continua;

    if (readA == 1 && readB == 1 && valor_viejo_canalA == 0 && valor_viejo_canalB ==0)
      goto continua;

    if (readA == 0 && readB == 1 && valor_viejo_canalA == 1 && valor_viejo_canalB ==0)
      goto continua;

    if (readA == 1 && readB == 0 && valor_viejo_canalA == 0 && valor_viejo_canalB ==1)
      goto continua;

     if (readA == valor_viejo_canalA)
    {
      if (readB == readA)
        goto decvalor;
      else
        goto incvalor;
    }
    else
    {
      if (readB == readA)
        goto incvalor;
      else
        goto decvalor;
    }
#endif

incvalor:
  iniciar_tempo();
//  PIEZO = 1;
//  delay_us(incremento_enc*30);
//  PIEZO = 0;

  switch (modo)
  {
    case MODO_VOLTAJE:
      if (cortecorriente) break;
      if (pwmduty < 1023 - incremento_enc)
      {
        pwmduty += incremento_enc;
        pwm1_set_data();
      }
      else
        pwmduty = 1023 - incremento_enc;
      num_memoria = 0;
      break;
    case MODO_MEMORIA:
      if (num_memoria < max_memoria)
        num_memoria++;
      else
        num_memoria = max_memoria;
      delay_ms(50);
      break;
    case MODO_AMPERES:
      if (amperes < 1023 - incremento_enc)
        amperes += incremento_enc;
      else
        amperes = 1023 - incremento_enc;
      num_memoria = 0;
  }

  goto continua;

decvalor:
//  PIEZO = 1;
//  delay_us(incremento_enc * 30);
//  PIEZO = 0;
  iniciar_tempo();
  switch (modo)
  {
    case MODO_VOLTAJE:
      if (cortecorriente) break;
      if (pwmduty > incremento_enc)
      {
        pwmduty -= incremento_enc;
        pwm1_set_data();
      }
      else
        pwmduty = 1 + incremento_enc;
      num_memoria = 0;
      break;
    case MODO_MEMORIA:
       if (num_memoria > 1)
        num_memoria--;
      else
        num_memoria = 1;
      delay_ms(50);
      break;
    case MODO_AMPERES:
      if (amperes > incremento_enc)
        amperes -= incremento_enc;
      else
        amperes = 1 + incremento_enc;
      num_memoria = 0;
  }
continua:

    valor_viejo_canalA = readA;
    valor_viejo_canalB = readB;

    temporizador--;

    if (temporizador == 0)
    {
      modo = MODO_NORMAL;
      num_memoria = 0;
      temporizador = 4580;                        // 10 segundos
      cortecorriente = 0;                         // Sin corte de corriente
      LEDCORTE = 0;                               // Apaga led de corte de corriente
      CORTE_CORRIENTE = 1;                        // Activa salida de corriente
      temporizador_luz--;

      if (temporizador_luz == 0)
      {
         LCD_BACKLIGHT = 0;
         temporizador_luz = 5;
      }
    }

    T0IF = 0;                                      // Borro Bandera de Desborde
    return;
  }
  else if (ADIF)
  {
    switch (adc_channel)
    {
      case 0:
       adc_tension = ADRESH << 8 | ADRESL;
       adc_channel++;
      break;
      case 1:
       adc_corriente = ADRESH << 8 | ADRESL;
       adc_channel++;
     break;
      case 2:
       adc_temperatura = ADRESH << 8 | ADRESL;
       adc_channel = 0;
      break;
    }
    ADIF = 0;
    adc_startread(adc_channel);
  }
}

// Lee el conversor A/D (10bits) RA0
// luego multiplica por 25 y lo escribe en el LCD
void mostrar_tension()
{
  ADIE = 0;
  adc_temp = adc_tension * 25;
  ADIE = 1;

  ACC0 = adc_temp & 0xFF;
  ACC1 = (adc_temp >> 8) & 0xFF;

  bin2dec();

  lcd_send(LCD_COMMANDMODE, LCD_LINEA1);
  lcd_send(LCD_COMMANDMODE, LCD_CURSOR_ON);
  if (TenK == 0)
    lcd_send(LCD_CHARMODE, ' ');
  else
    lcd_send(LCD_CHARMODE, 48 + TenK);
  lcd_send(LCD_CHARMODE, 48 + Thou);
  lcd_send(LCD_CHARMODE, ',');
  lcd_send(LCD_CHARMODE, 48 + Hund);
  lcd_send(LCD_CHARMODE, 48 + Tens);
  lcd_send(LCD_CHARMODE, 48 + Ones);

  lcd_send(LCD_CHARMODE, 'V');
  if (modo == MODO_VOLTAJE)
    lcd_send(LCD_CHARMODE, 0b01111111);                // muestra flecha <
  else
    lcd_send(LCD_CHARMODE, ' ');
  lcd_send(LCD_COMMANDMODE, LCD_CURSOR_OFF);
}

void mostrar_corriente()
{
  ADIE = 0;
  adc_temp = adc_corriente;
  ADIE = 1;

  ACC0 = adc_temp & 0xFF;
  ACC1 = (adc_temp >> 8) & 0xFF;

  multi2_5();                                         // multiplica el valor del A/D (0-1024) por 2.5
                                                      // generando valores de 0 a 2560

  bin2dec();

  lcd_send(LCD_COMMANDMODE, (LCD_LINEA1 | 10));       // posiciona en la fila 1 columna 10

  lcd_send(LCD_COMMANDMODE, LCD_CURSOR_ON);
  lcd_send(LCD_CHARMODE, 48 + Thou);
  lcd_send(LCD_CHARMODE, ',');
  lcd_send(LCD_CHARMODE, 48 + Hund);
  lcd_send(LCD_CHARMODE, 48 + Tens);
  lcd_send(LCD_CHARMODE, 48 + Ones);
  lcd_send(LCD_CHARMODE, 'A');
  lcd_send(LCD_COMMANDMODE, LCD_CURSOR_OFF);
}

#ifndef MOSTRARPWM
void mostrar_memoria()
{
  if (modo != MODO_TEMPERATURA && modo != MODO_NORMAL && LCD_BACKLIGHT == 1)
  {
    lcd_send(LCD_COMMANDMODE, LCD_LINEA2);
    lcd_send(LCD_COMMANDMODE, LCD_CURSOR_ON);

    lcd_send(LCD_CHARMODE, 'M');
    lcd_send(LCD_CHARMODE, ':');

    if (num_memoria>0)
    {
      lcd_send(LCD_CHARMODE, 48 + num_memoria);
    }
    else
      lcd_send(LCD_CHARMODE, ' ');

    if (modo == MODO_MEMORIA)
      lcd_send(LCD_CHARMODE, 0b01111111);               // muestra flecha <
    else
      lcd_send(LCD_CHARMODE, ' ');
    lcd_send(LCD_CHARMODE, ' ');
    lcd_send(LCD_CHARMODE, ' ');
    lcd_send(LCD_CHARMODE, ' ');
    lcd_send(LCD_CHARMODE, ' ');
    lcd_send(LCD_COMMANDMODE, LCD_CURSOR_OFF);

    if (num_memoria > 0 && modo == MODO_MEMORIA)         // si está en modo memoria permite
    {                                                    // la selección de voltajes y amperes prefijados
      switch(num_memoria)
      {
        case 1:
          pwmduty = 105;                                 // 2.5v
          amperes = 200;                                 // 0.5A
        break;
        case 2:
          pwmduty = 123;                                 // 3v
          amperes = 200;                                 // 0.5A
        break;
        case 3:
          pwmduty = 135;                                 // 3.3v
          amperes = 200;                                 // 0.5A
        break;
        case 4:
          pwmduty = 205;                                 // 5v
          amperes = 410;                                 // 1A
        break;
        case 5:
          pwmduty = 364;                                 // 9v
          amperes = 410;                                 // 1A
        break;
        case 6:
          pwmduty = 484;                                 // 12v
          amperes = 1024;                                // 2.5A
        break;
      }
      pwm1_set_data();

    }
  }
  else
  {
    ADIE = 0;
    adc_temp = adc_temperatura * 357;
    ADIE = 1;

    ACC0 = adc_temp & 0xFF;
    ACC1 = (adc_temp >> 8) & 0xFF;

    bin2dec();

    lcd_send(LCD_COMMANDMODE, LCD_LINEA2);
    lcd_send(LCD_COMMANDMODE, LCD_CURSOR_ON);
    lcd_send(LCD_CHARMODE, 'T');
    lcd_send(LCD_CHARMODE, ':');
    lcd_send(LCD_CHARMODE, 48 + TenK);
    lcd_send(LCD_CHARMODE, 48 + Thou);
    lcd_send(LCD_CHARMODE, ',');
    lcd_send(LCD_CHARMODE, 48 + Hund);
    lcd_send(LCD_CHARMODE, 0b11010010);                  // Código CGROM de "º" en WH1602L-YGB-ST
  //  lcd_send(LCD_CHARMODE, 0b11011111);                 // Código CGROM de "º" para otros
    lcd_send(LCD_CHARMODE, ' ');
    lcd_send(LCD_COMMANDMODE, LCD_CURSOR_OFF);
  }
}
#else
void mostrar_pwm()
{
  ACC0 = pwmduty & 0xFF;
  ACC1 = (pwmduty >> 8) & 0xFF;

  bin2dec();

  lcd_send(LCD_COMMANDMODE, LCD_LINEA2);
  lcd_send(LCD_COMMANDMODE, LCD_CURSOR_ON);
  lcd_send(LCD_CHARMODE, 'D');
  lcd_send(LCD_CHARMODE, ':');
  lcd_send(LCD_CHARMODE, 48 + TenK);
  lcd_send(LCD_CHARMODE, 48 + Thou);
  lcd_send(LCD_CHARMODE, 48 + Hund);
  lcd_send(LCD_CHARMODE, 48 + Tens);
  lcd_send(LCD_CHARMODE, 48 + Ones);
  lcd_send(LCD_CHARMODE, ' ');
  lcd_send(LCD_COMMANDMODE, LCD_CURSOR_OFF);
}
#endif

void controlar_corriente()
{
  //Verificar la corriente
  ADIE = 0;
  if (adc_corriente > amperes && cortecorriente==0)
  {
    cortecorriente = 1;
    CORTE_CORRIENTE = 0;                               // activa el corte por sobrecorriente
    LEDCORTE = 1;                                      // activa luz de corte
    PIEZO = 1;
    delay_ms(300);
    PIEZO = 0;
    delay_ms(100);
    PIEZO = 1;
    delay_ms(300);
    PIEZO = 0;
  }

  // Verificar temperatura
  if (adc_temperatura > 84 || adc_corriente > 400
  || (adc_corriente > 200 && CONTROL_FAN == 1))        // enciende el ventilador si la temperatura es mayor a 30 grados (84*357=29.988grados)
                                                       // o si se supera 1 A o bien, si ya superó 1A, espera a que baje de 500mA.
    CONTROL_FAN = 1;
  else
    CONTROL_FAN = 0;
  ADIE = 1;

  ACC0 = amperes & 0xFF;
  ACC1 = (amperes >> 8) & 0xFF;

  multi2_5();

  bin2dec();

  lcd_send(LCD_COMMANDMODE, (LCD_LINEA2 | 9));         // posiciona en la fila 2 columna 10
  lcd_send(LCD_COMMANDMODE, LCD_CURSOR_ON);

  if (modo == MODO_AMPERES)
    lcd_send(LCD_CHARMODE, 0b01111110);                // muestra la flecha >
  else
    lcd_send(LCD_CHARMODE, ' ');
  lcd_send(LCD_CHARMODE, 48 + Thou);
  lcd_send(LCD_CHARMODE, ',');
  lcd_send(LCD_CHARMODE, 48 + Hund);
  lcd_send(LCD_CHARMODE, 48 + Tens);
  lcd_send(LCD_CHARMODE, 48 + Ones);
  lcd_send(LCD_CHARMODE, 'A');
  lcd_send(LCD_COMMANDMODE, LCD_CURSOR_OFF);
}

int main (void)
{
  char intro[] = "Felixls 2009 :)";
  char serie_cmd;

  GIE = 0;                               // Deshabilita interrupciones

  //inicializar_variables
  modo = MODO_NORMAL;
  cortecorriente = 0;
  grabarmemoria = 0;
  num_memoria = 0;
  pwmduty = 1;
  incremento_enc = 1;
  valor_viejo_canalA = 0;
  valor_viejo_canalB = 0;
  amperes = 1;

  TRISA=0xFF;                           // PORTA como entrada.
  TRISB=0x00;                           // PORTB como salida.
  TRISC=0b00111000;                     // PORTC = 00111000 --> botones como entradas.
                                        //                      PWM (RC2) y el resto, como salidas.
  TRISD=0b01110000;                     // PORTD = 01110000 --> botones como entradas RD4/5/6
  TRISE=0x00;

  delay_ms(500);

  // Limpieza de registros
  PORTA=0x00;
  PORTB=0x00;
  PORTC=0x00;
  PORTD=0x00;
  PORTE=0x00;

  init_serie();
  print(intro);
  print("\n");


  adc_init(FOSC_32, A5_R0, INT_ON);    // configura módulo A/D con clock Fosc/32, resultados justificados a derecha
                                        // y configura los bits del control de puertos:
                                        // AN4-7 y AN2: Digital I/O
                                        // AN0-3: Entradas analógicas
                                        // VDD como Vref+
                                        // VSS como Vref-

  // PWM_PERIODO = (PR2+1)*4*PrescalerTMR2/FOSC_HZ
  // PWM_DutyCycle = (CCPR1L:CCP1CON<5,4>)*PrescalerTMR2/FOSC_HZ
  // Frecuencia = 1/PeriodoPWM
  // PR2 249 con prescaler 4 --> PWM Periodo = 0.00025  --> Frecuencia = 1khz

//  PR2 = 249;
//  T2CON = 0b000000111;
//  CCPR1L = 0b00000000;
//  CCP1CON = 0b00001100;                                  //CCP1/RC2/Control de Tensión  (inicio con duty en 0 por seguridad).

  PR2 = 0b11111001 ;
  T2CON = 0b00000101 ;
  CCPR1L = 0b00000000 ;
  CCP1CON = 0b00001100 ;

  lcd_init();

  LCD_BACKLIGHT = 1;
  lcd_message(intro);

  delay_ms(1000);

  lcd_send(LCD_COMMANDMODE, LCD_CLEAR);

  delay_ms(250);

// Configuración de TMR0 como temporizador
// Temporizacion = Tcm * Prescaler * (256 - Carga TMR0)

// Ejemplo: para un intervalo de 500us con un prescaler de 2 a 4mhz
// 500us = 1 * 2 * (256 - Carga TMR0)  -->   Carga TMR0 = 6
  OPTION_REG = 0b00000010;                               // Establece el prescaler a instrucciones/8 para timer0.

  T0IE = 1;                                              // activa interrupción por Tmr0
  T0IF = 0;                                              // borro bandera de desborde
  GIE=1;                                                 // Habilito Interrupciones


  LEDCORTE = 0;                                          // Apaga led de corte de corriente
  CORTE_CORRIENTE = 1;                                   // Activa salida de corriente

  iniciar_tempo();

  adc_channel = 0;
  adc_startread(adc_channel);

  // bucle infinito de la fuente.
  while(1)
  {
    mostrar_tension();
    mostrar_corriente();
#ifndef MOSTRARPWM
    mostrar_memoria();
#else
    mostrar_pwm();
#endif
    controlar_corriente();

    serie_cmd = leccar();
    switch(serie_cmd)
    {
      case 'd':                                             // duty, devuelve el duty (0-1024)
        ACC0 = pwmduty & 0xFF;
        ACC1 = (pwmduty >> 8) & 0xFF;

        bin2dec();

        print("duty = ");
        putchar(48 + TenK);
        putchar(48 + Thou);
        putchar(48 + Hund);
        putchar(48 + Tens);
        putchar(48 + Ones);
        print("\r\n");
      break;
      case 'a':                                             // amperes, devuelve el amperaje sensado actual
        ADIE = 0;
        ACC0 = adc_corriente & 0xFF;
        ACC1 = (adc_corriente >> 8) & 0xFF;
        ADIE = 1;

        multi2_5();                                         // multiplica el valor del A/D (0-1024) por 2.5
        bin2dec();

        print("amperes = ");
        putchar(48 + Thou);
        putchar(',');
        putchar(48 + Hund);
        putchar(48 + Tens);
        putchar(48 + Ones);
        print("\r\n");
      break;
      case 't':                                            // tensión, devuelve el voltaje actual
        ADIE = 0;
        ACC0 = adc_tension & 0xFF;
        ACC1 = (adc_tension >> 8) & 0xFF;
        ADIE = 1;

        bin2dec();

        print("tension = ");
        putchar(48 + TenK);
        putchar(48 + Thou);
        putchar(',');
        putchar(48 + Hund);
        putchar(48 + Tens);
        putchar(48 + Ones);
        print("\r\n");
      break;
      case 'c':                                            // temperatura, devuelve la temperatura actual
        ADIE = 0;
        ACC0 = adc_temperatura & 0xFF;
        ACC1 = (adc_temperatura >> 8) & 0xFF;
        ADIE = 1;

        bin2dec();

        print("temperatura = ");
        putchar(48 + TenK);
        putchar(48 + Thou);
        putchar(',');
        putchar(48 + Hund);
        print("\r\n");
      break;
      case 's':                                            // stop: para la actividad de la fuente, pone el duty en 1.
        pwmduty = 1;
        amperes = 0;
        pwm1_set_data();
      break;
    }

    delay_ms(100);
  }
}
