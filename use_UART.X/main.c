/*
  UART test

  UARTを1つ使った最低限のテスト。割り込み無し。
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


int main(void) {
  __XC_UART = 1;

  system_init();	// Initialize I/O, OSC and Interrupt.

  // GPIO
  ANSELAbits.ANSA0 = 0;
  ANSELAbits.ANSA1 = 0;
  TRISAbits.TRISA0 = 0;
  TRISAbits.TRISA1 = 0;

  uart_init();
  printf("\r\n\x1b(B\x1b)B\x1b[0m\x1b[2JSystem start.\n");


  // echo back.
  while(1) {
    char buf[100];

    int n = uart1_read( buf, sizeof(buf) );
    uart1_write( buf, n );

    LATAbits.LATA0 = !PORTAbits.RA0;
  }

  return 0;
}
