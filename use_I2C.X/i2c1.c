/*
  I2C test ポーリング版

  use I2C1
	SCL: 17pin (RB8)
	SDA: 18pin (RB9)
*/

#include <stdint.h>
#include "xc.h"
#include "i2c1.h"


//================================================================
/*! initialize
*/
int i2c1_init(void)
{
  CNPUBbits.CNPUB8 = 1;	// Pull up
  CNPUBbits.CNPUB9 = 1;

  I2C1BRG = 0x31;	// 0x31 : see DS60001116G Table 24-2
  I2C1CON = 0x8200;	// Enable, 100kHz
  I2C1STAT = 0x0;
}


//================================================================
/*! I2C Send START condition

  @return	not zero if error.
*/
int i2c1_start(void)
{
  I2C1CONSET = (1 << _I2C1ACON_SEN_POSITION);

  // Normally loops about 20 times at clock 40MHz.
  // thus, for safety 2000 loops.
  int i;
  for( i = 0; i < 2000; i++ ) {
    if( I2C1CONbits.SEN == 0 ) return 0;
  }
  return -1;
}


//================================================================
/*! I2C Send RESTART condition

  @return	not zero if error.
*/
int i2c1_restart(void)
{
  I2C1CONSET = (1 << _I2C1ACON_RSEN_POSITION);

  int i;
  for( i = 0; i < 2000; i++ ) {
    if( I2C1CONbits.RSEN == 0 ) return 0;
  }
  return -1;
}


//================================================================
/*! I2C Send STOP condition

  @return	not zero if error.
*/
int i2c1_stop(void)
{
  I2C1CONSET = (1 << _I2C1CON_PEN_POSITION);

  int i;
  for( i = 0; i < 2000; i++ ) {
    if( I2C1CONbits.PEN == 0 ) return 0;
  }
  return -1;
}


//================================================================
/*! I2C write 1 byte.

  @param  data	data byte.
  @return	0: ACK / 1: NACK / Other: error.
*/
int i2c1_write1( uint8_t data )
{
  I2C1TRN = data;

  // about 100us/byte at 100kHz mode.
  // thus, for safety about 10ms.
  int i;
  for( i = 0; i < 1000; i++ ) {
    if( (I2C1STAT & (_I2C1STAT_TBF_MASK|_I2C1STAT_TRSTAT_MASK)) == 0 ) {
      return I2C1STATbits.ACKSTAT;
    }
    __delay_us( 10 );
  }

  return -1;
}


//================================================================
/*! I2C read (receive) 1 byte.

  @param  ack_nack	ack = 0 / nack = 1
  @return		received data byte. or minus value if error.
*/
int i2c1_read1( int ack_nack )
{
  I2C1CONbits.RCEN = 1;			// receive start

  int i;
  for( i = 0; I2C1CONbits.RCEN; i++ ) {	// Wait for receive 1 byte.
    if( i > 1000 ) goto TIMEOUT_ERROR;
    __delay_us( 10 );
  }

  int ret = I2C1RCV;			// get received data.

  I2C1CONbits.ACKDT = !!ack_nack;	// send ack=0 or nack=1
  I2C1CONbits.ACKEN = 1;
  for( i = 0; I2C1CONbits.ACKEN; i++ ) {
    if( i > 2000 ) goto TIMEOUT_ERROR;
  }

  return ret;

 TIMEOUT_ERROR:
  return -1;
}


//================================================================
/*! I2C write

  @param  i2c_adrs_7	I2C 7bit address + START/STOP condition flag.
  @param  buffer	Pointer to data.
  @param  size		buffer size.
  @return		sent size or minus value if error.

  (I2C Sequence)
  S - ADRS W A data_1 A... data_n A|N - P
    S : Start condition
    P : Stop condition
    A : Ack
    N : Nack
*/
int i2c1_write( int i2c_adrs_7, const void *buffer, int size )
{
  int ret;
  uint16_t flag_s_p = i2c_adrs_7 & 0xf000;
  i2c_adrs_7 &= 0x7f;

  if( !(flag_s_p & I2C_NOSTART) ) {
    // start condition
    ret = (flag_s_p & I2C_RESTART) ? i2c1_restart() : i2c1_start();
    if( ret != 0 ) {
      ret = -1;
      goto STOP;
    }

    // send I2C address + R/W bit.
    ret = i2c1_write1( i2c_adrs_7 << 1 );	// address + r/w=0 (write).
    if( ret != 0 ) {
      ret = -2;
      goto STOP;
    }
  }

  // send data.
  const uint8_t *p = (const uint8_t *)buffer;
  int i;
  for( i = size; i > 0; i-- ) {
    ret = i2c1_write1( *p++ );
    if( ret != 0 ) {
      break;
    }
  }
  ret = size - i;

  // send stop condition.
 STOP:
  if( !(flag_s_p & I2C_NOSTOP) ) {
    i2c1_stop();
  }

  return ret;
}


//================================================================
/*! I2C read

  @param  i2c_adrs_7	I2C 7bit address + START/STOP condition flag.
  @param  buffer	Pointer to read buffer.
  @param  size		buffer size.
  @return		received size or minus value if error.

  (I2C Sequence)
  S|Sr - ADRS R A - data_1 A... data_n A|N - P
    S : Start condition
    P : Stop condition
    Sr: Repeated start condition
    A : Ack
    N : Nack
*/
int i2c1_read( int i2c_adrs_7, void *buffer, int size )
{
  int ret;
  uint16_t flag_s_p = i2c_adrs_7 & 0xf000;
  i2c_adrs_7 &= 0x7f;

  if( !(flag_s_p & I2C_NOSTART) ) {
    // start / repeated start condition
    ret = (flag_s_p & I2C_RESTART) ? i2c1_restart() : i2c1_start();
    if( ret != 0 ) {
      ret = -3;
      goto STOP;
    }

    // send I2C address.
    ret = i2c1_write1( (i2c_adrs_7 << 1) | 1 );	// address + r/w=1 (read).
    if( ret != 0 ) {
      ret = -4;
      goto STOP;
    }
  }

  // receive data.
  uint8_t *p = (uint8_t *)buffer;
  int i;
  for( i = size-1; i >= 0; i-- ) {
    ret = i2c1_read1( !(flag_s_p & I2C_NOSTOP) && (i == 0) );
    if( ret < 0 ) {
      ret = size - i - 1;
      goto STOP;
    }
    *p++ = ret;
  }
  ret = size;

  // send stop condition.
 STOP:
  if( !(flag_s_p & I2C_NOSTOP) ) {
    i2c1_stop();
  }

  return ret;
}



//================================================================
/*! I2C read after write transaction.

  @param  i2c_adrs_7	I2C 7bit address + START/STOP condition flag.
  @param  wbuf		write data.
  @param  wsize		write data size
  @param  rbuf		read buffer.
  @param  rsize		buffer size.
  @return		received size or minus value if error.

  (I2C Sequence)
  S - ADRS W A - [pre A...] - Sr - ADRS R A - data_1 A... data_n A|N - P
    S : Start condition
    P : Stop condition
    Sr: Repeated start condition
    A : Ack
    N : Nack
*/
int i2c1_trans( int i2c_adrs_7, const void *wbuf, int wsize, void *rbuf, int rsize )
{
  int ret;

  // write data.
  ret = i2c1_write( i2c_adrs_7 | I2C_NOSTOP, wbuf, wsize );
  if( ret != wsize ) {
    return (ret > 0)? 0 : ret;
  }

  // read data.
  return i2c1_read( i2c_adrs_7 | I2C_RESTART, rbuf, rsize );
}
