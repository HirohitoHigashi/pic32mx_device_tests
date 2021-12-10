#include "xc.h"

#if defined(__32MX270F256B__) || defined(__PIC32MX270F256B__)
# include "init_32mx270f256b.c"
#else
# error No initialize file found.
#endif
#include "clock_freq.h"
#include "common_pic32mx.c"


int main(void) {
  system_init();	// Initialize I/O, OSC and Interrupt.

  /* GPIO settings.

    Pin Reg  I/O   note
  ------------------------
     2  RA0  OUT
     3  RA1  OUT
    16  RB7  IN    No analog pin.
  */
  ANSELAbits.ANSA0 = 0;
  ANSELAbits.ANSA1 = 0;

  TRISAbits.TRISA0 = 0;
  TRISAbits.TRISA1 = 0;
  TRISBbits.TRISB7 = 1;

  CNPUBbits.CNPUB7 = 1;

  while(1) {
    LATAbits.LATA0 = 0;
    __delay_ms( 100 );
    LATAbits.LATA0 = 1;
    __delay_ms( 100 );

    int sw = PORTBbits.RB7;
    if( sw == 0 ) {
      LATAbits.LATA1 = 1;
    } else {
      LATAbits.LATA1 = 0;
    }
  }

  return 0;
}
