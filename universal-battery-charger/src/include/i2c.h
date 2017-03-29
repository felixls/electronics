#ifndef I2C_H
#define I2C_H

#define SDA RC4
#define SCL RC3

#define TSDA TRISC4
#define TSCL TRISC3

void i2c_delay(void);
void i2c_clock(void);
void i2c_start(void);
void i2c_stop(void);
unsigned char i2c_write(unsigned char dat);
unsigned char i2c_read(void);
void i2c_ack();
void i2c_noack();


#endif

