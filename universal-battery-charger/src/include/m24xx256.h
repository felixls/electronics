#ifndef M24xx512_H
#define M24xx512_H

#include "i2c.h"

#define EEPROMS_ID 0xA0	/* Microchip 24xx256 */

unsigned char EEPROM_get(unsigned int addr);
void EEPROM_set(unsigned int addr, unsigned char val);


#endif

