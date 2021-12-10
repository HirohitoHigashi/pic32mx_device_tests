/*
  UART test

  UARTを2つと割り込みを使ったテスト。

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xc.h"

#if defined(__32MX270F256B__) || defined(__PIC32MX270F256B__)
# include "init_32mx270f256b.c"
#else
# error No initialize file found.
#endif
#include "clock_freq.h"
#include "common_pic32mx.c"
#include "uart.h"


/*
  for stdout
*/
void _mon_putc( char c )
{
  uart_write( &uart1_handle, &c, 1 );
}


int main(void) {
  system_init();	// Initialize I/O, OSC and Interrupt.

  // GPIO
  ANSELAbits.ANSA0 = 0;
  ANSELAbits.ANSA1 = 0;
  TRISAbits.TRISA0 = 0;
  TRISAbits.TRISA1 = 0;

  uart_init();
  uart_set_baudrate( &uart1_handle, 19200 );
  uart_set_baudrate( &uart2_handle, 19200 );
  printf("\r\n\x1b(B\x1b)B\x1b[0m\x1b[2JSystem start.\n");

  // LED
  LATAbits.LATA0 = 1;
  LATAbits.LATA1 = 1;
  __delay_ms( 200 ); LATAbits.LATA0 = 0;
  __delay_ms( 200 ); LATAbits.LATA1 = 0;


  while( 1 ) {
    char buf[100];

    // UART1から読み込んだデータを、UART2へ送信
    if( uart_is_readable( &uart1_handle ) ) {
      int n = uart_read( &uart1_handle, buf, sizeof(buf) );
      uart_write( &uart2_handle, buf, n );
    }

    // UART2から読み込んだデータを、UART1へ送信
    if( uart_is_readable( &uart2_handle ) ) {
      int n = uart_read( &uart2_handle, buf, sizeof(buf) );
      uart_write( &uart1_handle, buf, n );
    }

    // オーバーフローエラー確認
    if( uart_is_rx_overflow( &uart1_handle )) {
      //      uart_puts( &uart1_handle, "\nUART1 Overflow!\n" );
      LATAbits.LATA0 = 1;	// LED1
      uart_clear_rx_buffer( &uart1_handle );
    }
    if( uart_is_rx_overflow( &uart2_handle )) {
      //      uart_puts( &uart1_handle, "\nUART2 Overflow!\n" );
      LATAbits.LATA1 = 1;	// LED2
      uart_clear_rx_buffer( &uart2_handle );
    }
  }

  return 0;
}
