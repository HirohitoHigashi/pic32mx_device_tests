/*
  I2C test ポーリング版

  use I2C1
	SCL: 17pin (RB8)
	SDA: 18pin (RB9)
*/

#include <stdio.h>
#include <stdint.h>

#include "xc.h"

#if defined(__32MX270F256B__) || defined(__PIC32MX270F256B__)
# include "init_32mx270f256b.c"
#else
# error No initialize file found.
#endif
#include "clock_freq.h"
#include "common_pic32mx.c"
#include "uart1.c"

#include "i2c1.h"



int main(void)
{
  __XC_UART = 1;

  system_init();	// Initialize I/O, OSC and Interrupt.

  // GPIO
  ANSELAbits.ANSA0 = 0;
  ANSELAbits.ANSA1 = 0;
  TRISAbits.TRISA0 = 0;
  TRISAbits.TRISA1 = 0;

  // UART
  uart_init();
  printf("\r\n\x1b(B\x1b)B\x1b[0m\x1b[2JSystem start.\n");

  // I2C
  i2c1_init();

  //  __delay_ms(100);

#define ADRS_HDC1000 0x40
#define ADRS_LPS25H  0x5c
  unsigned char buf[20];
  int ret;

  /*
    LPS25H
  */
  ret = i2c1_trans( ADRS_LPS25H, "\x0f", 1, buf, 1 );	// check
  printf("LPS25H result. ret = %d %02x\n", ret, buf[0] );

  ret = i2c1_write( ADRS_LPS25H, "\x20\x90", 2 );	// init
  printf("LPS25H write init. ret = %d\n", ret );


  /*
    HDC1000
  */
  i2c1_write( ADRS_HDC1000, "\x02\x16\x00", 3 );	// init


  /*
    read every 1 sec
  */
  while( 1 ) {
    // LPS25H
    ret = i2c1_trans( ADRS_LPS25H, "\xa8", 1, buf, 5 );
    if( ret == 5 ) {
      double pres = ((uint32_t)buf[2] << 16 | buf[1] << 8 | buf[0]) / 4096.0;
      double temp = (int16_t)(buf[4] << 8 | buf[3]) / 480.0 + 42.5;
      printf("LPS25H  %5.1f C  %7.2f hPa\n", temp, pres );
    }

    // HDC1000
    i2c1_write( ADRS_HDC1000 | I2C_NOSTOP, "\x00", 1 );
    __delay_ms( 7 );
    ret = i2c1_read( ADRS_HDC1000 | I2C_RESTART, buf, 4 );
    if( ret == 4 ) {
      double temp = (buf[0] << 8 | buf[1]) / 65536.0 * 165 - 40;
      int humi = (uint32_t)(buf[2] << 8 | buf[3]) * 100 / 65536;
      printf("HDC1000 %5.1f C  %7d %%\n", temp, humi );
    }

    __delay_ms( 1000 );
  }

  return 0;
}
