#ifndef ADC_H
#define ADC_H

/* flag de interrupciones on/off */
#define INT_OFF   0x00
#define INT_ON   0x01

/* frequencia de oscillador */
#define FOSC_2   0x00
#define FOSC_4   0x04
#define FOSC_8   0x01
#define FOSC_16   0x05
#define FOSC_32   0x02
#define FOSC_64   0x06
#define FOSC_RC   0x07

/* configuracion de entradas y Vref(PCFG en ADCON1) */
#define A8_R0   0x00
#define A7_R1   0x01
#define A5_R0   0x02
#define A4_R1   0x03
#define A3_R0   0x04
#define A2_R1   0x05
#define A0_R0   0x06
#define A6_R2   0x08
#define A6_R0   0x09
#define A5_R1   0x0a
#define A4_R2   0x0b
#define A3_R2   0x0c
#define A2_R2   0x0d
#define A1_R0   0x0e
#define A1_R2   0x0f

void adc_init(unsigned char fosc, unsigned char pcfg, unsigned char config);
void adc_close(void);
unsigned int adc_read(unsigned char canal);
void adc_startread(unsigned char canal);

#endif
