/*
  PWM test

  use OC1, T2(Timer2)
*/

#include <string.h>
#include <stdio.h>
#include <sys/attribs.h>

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

  // Timer2 initialize and start.
  TMR2 = 0;
  PR2 = 50000;		// PBCLK=10MHz => 5ms

  // PWM initialize and start.
  ANSELBbits.ANSB15 = 0;
  TRISBbits.TRISB15 = 0;	// RB15 output mode
  RPB15Rbits.RPB15R = 0x0005;	// RB15->OC1
  OC1CON = 0x0006;		// 0000_0000_0000_0110 : PWM mode
  OC1R = OC1RS = 5000;		// duty 1/10
  OC1CONbits.ON = 1;

  T2CON = 0x8000;	// Start timer.

  while(1) {
    LATAbits.LATA0 ^= 1;
    __delay_ms(1000);
  }

  return 0;
}
