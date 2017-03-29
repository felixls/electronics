#include "m24xx256.h"

unsigned char EEPROM_get(unsigned int addr)
{
	unsigned char dat;	

	i2c_start();            /* Start i2c bus */

	i2c_write(EEPROMS_ID);   /* Connect to EEPROM */
	i2c_write(addr&0xF0);	 /* Request RAM address (Hight byte) */
	i2c_write(addr&0x0F);	 /* Request RAM address (Low byte) */
	
	i2c_start();			/* Start i2c bus */

	i2c_write(EEPROMS_ID+1);/* Connect to EEPROM for Read */
	dat = i2c_read();		/* Receive data */

	i2c_noack();
	
	i2c_stop();				/* Stop i2c bus */

   return dat;			
}

void EEPROM_set(unsigned int addr, unsigned char val)
{
	i2c_start(); 

	i2c_write(EEPROMS_ID);   /* Connect to EEPROM */
	i2c_write(addr&0xF0);	 /* Request RAM address (Hight byte) */
	i2c_write(addr&0x0F);	 /* Request RAM address (Low byte) */

	i2c_write(val);			/* Write sec on RAM specified address */

	i2c_stop();           	/* Stop i2c bus */
}

