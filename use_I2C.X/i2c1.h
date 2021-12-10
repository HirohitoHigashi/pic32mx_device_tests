/*
  I2C test ポーリング版

  use I2C1
	SCL: 17pin (RB8)
	SDA: 18pin (RB9)
*/

#include <stdint.h>

enum {
  I2C_RESTART = 0x1000,
  I2C_NOSTART = 0x2000,
  I2C_NOSTOP  = 0x4000,
};


int i2c1_init(void);
int i2c1_start(void);
int i2c1_restart(void);
int i2c1_stop(void);
int i2c1_write1(uint8_t data);
int i2c1_read1(int ack_nack);
int i2c1_write(int i2c_adrs_7, const void *buffer, int size);
int i2c1_read(int i2c_adrs_7, void *buffer, int size);
int i2c1_trans(int i2c_adrs_7, const void *wbuf, int wsize, void *rbuf, int rsize);
