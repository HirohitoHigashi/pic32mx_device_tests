/*
  Timer1 のテスト
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


// Timer1 interrupt handler.
void __ISR(_TIMER_1_VECTOR, IPL1AUTO) timer1_isr( void )
{
  static int n;

  if( ++n >= 1000 ) {
    LATAbits.LATA1 ^= 1;
    n = 0;
  }

  IFS0CLR = (1 << _IFS0_T1IF_POSITION);
}



int main(void) {
  system_init();	// Initialize I/O, OSC and Interrupt.

  // GPIO
  ANSELAbits.ANSA0 = 0;
  ANSELAbits.ANSA1 = 0;
  TRISAbits.TRISA0 = 0;
  TRISAbits.TRISA1 = 0;

  // Timer1 initialize and start.
  TMR1 = 0;
  PR1 = 10000;		// PBCLK=10MHz => 1ms
  IPC1bits.T1IP = 1;	// Interrupt priority.
  IPC1bits.T1IS = 0;
  IFS0bits.T1IF = 0;	// Clear interrupt
  IEC0bits.T1IE = 1;	// Enable interrupt
  T1CON = 0x8000;	// Start timer.

  while(1) {
    LATAbits.LATA0 ^= 1;
    __delay_ms(1000);
  }

  return 0;
}
