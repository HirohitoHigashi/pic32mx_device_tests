/*
  SPI test

*/

#include <string.h>
#include <stdio.h>

#include "xc.h"

#if defined(__32MX270F256B__) || defined(__PIC32MX270F256B__)
# include "init_32mx270f256b.c"
#else
# error No initialize file found.
#endif
#include "clock_freq.h"
#include "common_pic32mx.c"
#include "uart1.c"

#include "spi.h"


int main(void) {
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

  // SPI 1
  //  SS: 26pin (RB15) out. controlled by GPIO
  spi1_init();
  ANSELBbits.ANSB15 = 0;
  TRISBbits.TRISB15 = 0;
  LATBbits.LATB15 = 1;

#if 0
  uint8_t dummy;
  LATBbits.LATB15 = 0;
  SPI1_Exchange( "\x06", &dummy );  // WREN
  LATBbits.LATB15 = 1;
  __delay_ms( 1000 );

#if 0
  unsigned char buf1[] = { 0x02, 0, 0, 0,
			     0,1,2,3,4,5,6,7,8,9,10,11,12,23,14,15,
			     16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
#else
  unsigned char buf1[] = { 0x03, 0, 0, 0 };
#endif
  unsigned char buf2[100];

    //int n = uart1_read( buf, sizeof(buf) );

  LATBbits.LATB15 = 0;
  spi1_transfer( buf1, sizeof(buf1), buf2, 100, 0 );
  LATBbits.LATB15 = 1;


  return 0;
#endif



  while(1) {
    unsigned char buf[10] = { 0x03, 0, 0, 0 };
    unsigned char buf2[100];

    //int n = uart1_read( buf, sizeof(buf) );

    LATBbits.LATB15 = 0;
    //SPI1_Exchange( buf, buf2 ); SPI1_Exchange( buf+1, buf2+1 );
    spi1_transfer( buf, 4, buf2, 100, 0 );
    LATBbits.LATB15 = 1;

    printf("SPI Read $%02x %02x %02x %02x\n",buf2[0],buf2[1],buf2[2],buf2[3]);

    LATAbits.LATA0 = !PORTAbits.RA0;

    __delay_ms( 10000 );
  }

  return 0;
}
