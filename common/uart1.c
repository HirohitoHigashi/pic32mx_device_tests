/*! @file
  @brief

  PIC32MX UART I/O no interrupt light weight version.

  @author hirohito higashi
  @version v1.00
  @date 2021/08/01
  @note
  <pre>
  Copyright (c) 2021 Hirohito Higashi All Rights Reserved.
  This file is distributed under BSD 3-Clause License.

  (usage)
  uart_init();

  char buf[100];
  int n = uart1_read( buf, sizeof(buf) );
  uart1_write( buf, n );

  </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
/***** Local headers ********************************************************/
/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
void uart_init(void);
void uart1_set_baudrate(int baudrate);
int uart1_write(const void *buffer, int size);
int uart1_read(void *buffer, int size);

/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
/***** Signal catching functions ********************************************/
/***** Local functions ******************************************************/
/***** Global functions *****************************************************/

//================================================================
/*! initialize uart
*/
void uart_init(void)
{
  /* UART1
      TxD: RPB4(11pin)
      RxD: RPA4(12pin)
  */
  RPB4Rbits.RPB4R = 0x0001;	// RB4->UART1:U1TX
  U1RXRbits.U1RXR = 0x0002;	// RA4->UART1:U1RX

  // UART1 parameter.
  U1MODE = 0x0008;
  U1STA = 0x0;
  uart1_set_baudrate( 19200 );

  // Enabling UART1
  U1STAbits.UTXEN = 1;		// TX_enable
  U1STAbits.URXEN = 1;		// RX_enable
  U1MODEbits.ON = 1;
}

//================================================================
/*! set baud-rate

  @param  baudrate	baudrate.
*/
void uart1_set_baudrate( int baudrate )
{
  /* データシート掲載計算式
       UxBRG = PBCLK / (4 * baudrate) - 1
     戦略
     　誤差を小さくしたい。
     　4bitシフトして計算した後3bit目を足すことで四捨五入の代わりにする。
  */
  uint32_t brg_x16 = ((uint32_t)PBCLK << 2) / baudrate;
  U1BRG = (brg_x16 >> 4) + ((brg_x16 & 0xf) >> 3) - 1;
}

//================================================================
/*! write
 */
int uart1_write( const void *buffer, int size )
{
  const uint8_t *p = (const uint8_t *)buffer;
  int n = size;

  while( n > 0 ) {
    while( U1STAbits.UTXBF ) {	// TX-FIFOに空きができるまで待つ。
      Nop(); Nop(); Nop(); Nop();
    }
    U1TXREG = *p++;
    n--;
  }

  return size;
}

//================================================================
/*! read
 */
int uart1_read( void *buffer, int size )
{
  char *p = (char *)buffer;
  uint32_t sta = U1STA;

  while( 1 ) {
    sta = U1STA;
    if( sta & (1 << _U1STA_OERR_POSITION) ) {
      U1STACLR = (1 << _U1STA_OERR_POSITION);
    }

    if( sta & (1 << _U1STA_URXDA_POSITION) ) {
      // データあり
      *p++ = U1RXREG;
      if( p - (char*)buffer >= size ) {
	break;
      }
    } else {
      // データなし
      if( p != buffer ) break;
      Nop(); Nop(); Nop(); Nop();
    }
  }

  return p - (char*)buffer;
}
