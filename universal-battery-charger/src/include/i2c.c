#include <pic/pic16f877a.h>

#include "i2c.h"
#include "delay.h"

void i2c_delay(void)
{
  delay_us(133);
}

void SDALow()
{
  TSDA = 0;
  SDA = 0;  
}

void SDAHigh()
{
  TSDA = 1;
}

void SCLLow()
{
  TSCL = 0;
  SCL = 0;
}

void SCLHigh()
{
  TSCL = 1;
  while(SCL==0);
}

void i2c_clock(void)
{
	i2c_delay();

	SCLHigh();	    	/* Start clock */

	i2c_delay();    

	SCLLow();		      /* Clear SCL */

	i2c_delay();    
}

void i2c_start(void)
{
	if(SCL)
  	SCLLow();		   /* Clear SCL */

	SDAHigh();        /* Set SDA */
	SCLHigh();		   /* Set SCL */

	i2c_delay(); 

	SDALow();        /* Clear SDA */

	i2c_delay(); 

	SCLLow();        /* Clear SCL */

	i2c_delay(); 
}

void i2c_stop(void)
{
	if(SCL)	
  	SCLLow();			/* Clear SCL */

	SDALow();			/* Clear SDA */
	SCLHigh();			/* Set SCL */

	i2c_delay();

	SDAHigh();			/* Set SDA */

	i2c_delay();
}

unsigned char i2c_write(unsigned char dat)
{
	unsigned char data_bit;		
	unsigned char i;	

	for(i=0;i<8;i++)				/* For loop 8 time(send data 1 byte) */
	{
		data_bit = dat & 0x80;		/* Filter MSB bit keep to data_bit */

    if (data_bit==0)
      SDALow();            /* Send data_bit to SDA */
    else
      SDAHigh();

		
		dat = dat<<1;  
	  SCLHigh();
	  i2c_delay();	
	  SCLLow();
	  i2c_delay();	
	}

	SDAHigh();
	SCLHigh();
	i2c_delay();	

	data_bit = SDA;   	/* Check acknowledge */
	SCLLow();      			/* Clear SCL */

	i2c_delay();

	return data_bit;	  /* If send_bit = 0 i2c is valid */		 	
}

unsigned char i2c_read(void)
{
	unsigned char i, dat;

	dat = 0x00;	

  SDAHigh();
  
	for(i=0;i<8;i++)		/* For loop read data 1 byte */
	{
		SCLHigh();			/* Set SCL */

		dat = dat<<1;		
		
		if (SDA)
			dat = dat | 0x01; // if port pin = 1, set LSB (bit position)
		else
			dat = dat & 0xFE; // else port pin = ,clear LSB (bit position)

    SCLLow();
	  i2c_delay();	
    
	}
	return dat;
}

void i2c_ack()
{
	SDALow();		/* Clear SDA */

	i2c_delay();    

	i2c_clock();	/* Call for send data to i2c bus */
}

void i2c_noack()
{
	SDAHigh();		/* Set SDA */

	i2c_delay();

	i2c_clock();	/* Call for send data to i2c bus */
}

