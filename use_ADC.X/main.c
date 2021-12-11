/*
  ADC test

  ADC1: PIN25 RB14 AN10
  ADC2: PIN26 RB15 AN9

  2つの入力チャネルを交互にサンプリングする
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


void adc_init( void )
{
  // init pins.
  ANSELBbits.ANSB15 = 1;	// set analog input
  TRISBbits.TRISB15 = 1;
  ANSELBbits.ANSB14 = 1;
  TRISBbits.TRISB14 = 1;

  // init ADC.
  AD1CON1 = 0x00e0;	// SSRC=111 CLRASAM=0 ASAM=0 SAMP=0
  AD1CON2 = 0x0000;
  AD1CON3 = 0x1e09;	// SAMC=1e(60us) ADCS=09(TAD=2us)
  AD1CHS  = 0x0000;
  AD1CSSL = 0x0000;

  // Enable ADC
  AD1CON1bits.ADON = 1;
}


double adc_sample( int ch )
{
  // Set channel.
  AD1CHSbits.CH0SA = ch;

  AD1CON1bits.SAMP = 1;
  while( !AD1CON1bits.DONE )
    ;
  AD1CON1bits.DONE = 0;

  return ADC1BUF0 * 3.3 / 1023;
}


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


  adc_init();

  while( 1 ) {
    double v1 = adc_sample( 9 );
    double v2 = adc_sample( 10 );

    printf("v = %5.3f  %5.3f\n", v1, v2 );

    __delay_ms( 500 );
  }

  return 0;
}



#if 0
  /*
    フルスピードテスト
  */
  volatile uint16_t v;

  while( 1 ) {
    AD1CON1bits.SAMP = 1;
    while( !AD1CON1bits.DONE )
      ;
    AD1CON1bits.DONE = 0;
    v = ADC1BUF0;

    LATAbits.LATA0 = !PORTAbits.RA0;
  }
#endif


#if 0
  /*
    ステータスフラグの挙動テスト
   */
#define SSTS 1024
  uint8_t sts[SSTS];
  volatile uint16_t v;
  int n = 0;

  while( 1 ) {
    sts[n++] = AD1CON1 & 0x0f | 0x80;
    AD1CON1bits.SAMP = 1;
    int s;
    do {
      s = AD1CON1 & 0x0f;
      sts[n++] = s;
    } while( !(s & 0x01) );
    AD1CON1bits.DONE = 0;

    v = ADC1BUF0;
    LATAbits.LATA0 = !PORTAbits.RA0;
    if( n > SSTS/2 ) break;
  }

  int i;
  for( i = 0; i < n; i++ ) {
    printf("%3d: %d  %d %d %d\n",i,
	   !!(sts[i]&0x80), !!(sts[i]&0x04), !!(sts[i]&0x02), !!(sts[i]&0x01) );
  }
#endif
