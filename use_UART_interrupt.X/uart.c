/*! @file
 @brief
 PIC32 UARTを2つ使った送受信関数

 UART1
      TxD: RPB4(11pin)
      RxD: RPA4(12pin)

 UART2
      TxD: RPB9(18pin)
      RxD: RPB8(17pin)

 @author hirohito
 @version 1.5
 @date 2021/07/11 12:15:16
 @note
 <pre>
 This file is destributed under 3-Cause BSD License.
 </pre>
*/

/***** Feature test switches ************************************************/
/***** System headers *******************************************************/
#include "xc.h"
#include <sys/attribs.h>

/***** Local headers ********************************************************/
#include "clock_freq.h"
#include "uart.h"

/***** Constat values *******************************************************/
/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
/***** Function prototypes **************************************************/
static void uart_push_rxfifo( UART_HANDLE *uh, uint8_t ch );

/***** Local variables ******************************************************/
/***** Global variables *****************************************************/
//! UART handle
UART_HANDLE uart1_handle, uart2_handle;

/***** Interrupt functions **************************************************/
//================================================================
/*! UART1 interrupt handler.
*/
void __ISR(_UART_1_VECTOR, IPL1AUTO) uart1_isr( void )
{
  UART_HANDLE *uh = &uart1_handle;

  if( IFS1bits.U1RXIF ) {
    do {
      uart_push_rxfifo( uh, U1RXREG );
    } while( U1STAbits.URXDA );
    IFS1CLR = (1 << _IFS1_U1RXIF_POSITION);
  }
  else if( IFS1bits.U1TXIF ) {
    if( !uh->p_txbuf ) {			// fail safe.
      IEC1CLR = (1 << _IEC1_U1TXIE_POSITION);	// disable TX interrupt.
    } else {
      do {
	if( uh->idx_txbuf >= uh->size_txbuf ) {
	  IEC1CLR = (1 << _IEC1_U1TXIE_POSITION); // disable TX interrupt.
	  uh->p_txbuf = 0;
	  break;
	}
	U1TXREG = uh->p_txbuf[ uh->idx_txbuf++ ];
      } while( !U1STAbits.UTXBF );		// fill the transmit FIFO
    }
    IFS1CLR = (1 << _IFS1_U1TXIF_POSITION);
  }

  uint32_t sta = U1STA;
  if( sta & (1 << _U1STA_FERR_POSITION) ) {
    U1STACLR = (1 << _U1STA_FERR_POSITION);
  }
  if( sta & (1 << _U1STA_OERR_POSITION) ) {
    U1STACLR = (1 << _U1STA_OERR_POSITION);
    uh->rx_overflow = 1;
  }
}

/*! UART2 interrupt handler.
*/
void __ISR(_UART_2_VECTOR, IPL1AUTO) uart2_isr( void )
{
  UART_HANDLE *uh = &uart2_handle;

  if( IFS1bits.U2RXIF ) {
    do {
      uart_push_rxfifo( uh, U2RXREG );
    } while( U2STAbits.URXDA );
    IFS1CLR = (1 << _IFS1_U2RXIF_POSITION);
  }
  else if( IFS1bits.U2TXIF ) {
    if( !uh->p_txbuf ) {			// fail safe.
      IEC1CLR = (1 << _IEC1_U2TXIE_POSITION);	// disable TX interrupt.
    } else {
      do {
	if( uh->idx_txbuf >= uh->size_txbuf ) {
	  IEC1CLR = (1 << _IEC1_U2TXIE_POSITION); // disable TX interrupt.
	  uh->p_txbuf = 0;
	  break;
	}
	U2TXREG = uh->p_txbuf[ uh->idx_txbuf++ ];
      } while( !U2STAbits.UTXBF );		// fill the transmit FIFO
    }
    IFS1CLR = (1 << _IFS1_U2TXIF_POSITION);
  }

  uint32_t sta = U2STA;
  if( sta & (1 << _U2STA_FERR_POSITION) ) {
    U2STACLR = (1 << _U2STA_FERR_POSITION);
  }
  if( sta & (1 << _U2STA_OERR_POSITION) ) {
    U2STACLR = (1 << _U2STA_OERR_POSITION);
    uh->rx_overflow = 1;
  }
}

//================================================================
/*! push byte to rx queue

  @param	uh	UART handle
  @param	ch	data byte
*/
static void uart_push_rxfifo( UART_HANDLE *uh, uint8_t ch )
{
  uh->rxfifo[uh->rx_wr++] = ch;

  // check rollover write index.
  if( uh->rx_wr < sizeof(uh->rxfifo)) {
    if( uh->rx_wr == uh->rx_rd ) {
      --uh->rx_wr;   // buffer full
      uh->rx_overflow = 1;
    }
  }
  else {
    if( uh->rx_rd == 0 ) {
      --uh->rx_wr;   // buffer full
      uh->rx_overflow = 1;
    }
    else {
      uh->rx_wr = 0; // roll over.
    }
  }
}



/***** Local functions ******************************************************/
//================================================================
/*! set baud-rate

  @param  baudrate	baudrate.
*/
static void uart1_set_baudrate( int baudrate )
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

static void uart2_set_baudrate( int baudrate )
{
  uint32_t brg_x16 = ((uint32_t)PBCLK << 2) / baudrate;
  U2BRG = (brg_x16 >> 4) + ((brg_x16 & 0xf) >> 3) - 1;
}


//================================================================
/*! clear rx buffer and hardware FIFO
*/
static void uart1_clear_rx_buffer(void)
{
  UART_HANDLE *uh = &uart1_handle;

  IEC1bits.U1RXIE = 0;	// Disable Rx interrupt

  while( U1STAbits.URXDA ) {
    volatile uint8_t ch = U1RXREG; (void)ch;
  }
  uh->rx_rd = 0;
  uh->rx_wr = 0;
  uh->rx_overflow = 0;
  IEC1bits.U1RXIE = 1;	// Enable Rx interrupt.
}

static void uart2_clear_rx_buffer(void)
{
  UART_HANDLE *uh = &uart2_handle;

  IEC1bits.U2RXIE = 0;	// Disable Rx interrupt

  while( U2STAbits.URXDA ) {
    volatile uint8_t ch = U2RXREG; (void)ch;
  }
  uh->rx_rd = 0;
  uh->rx_wr = 0;
  uh->rx_overflow = 0;
  IEC1bits.U2RXIE = 1;	// Enable Rx interrupt.
}


//================================================================
/*! Send out binary data without blocking.

  @param  buffer        Pointer of buffer.
  @param  size          Size of buffer.
  @return               Size of transmitted.
*/
static void uart1_write( const void *buffer, int size )
{
  uart1_handle.p_txbuf = buffer;
  uart1_handle.size_txbuf = size;
  uart1_handle.idx_txbuf = 0;
  IEC1SET = (1 << _IEC1_U1TXIE_POSITION);	// enable TX interrupt.
}

static void uart2_write( const void *buffer, int size )
{
  uart2_handle.p_txbuf = buffer;
  uart2_handle.size_txbuf = size;
  uart2_handle.idx_txbuf = 0;
  IEC1SET = (1 << _IEC1_U2TXIE_POSITION);	// enable TX interrupt.
}


/***** Global functions *****************************************************/
//================================================================
/*! initializer
*/
void uart_init(void)
{
  /* UART1
      TxD: RPB4(11pin)
      RxD: RPA4(12pin)
  */
  uart1_handle.p_txbuf = 0;
  uart1_handle.rx_overflow = 0;
  uart1_handle.delimiter = '\n';
  uart1_handle.rx_rd = 0;
  uart1_handle.rx_wr = 0;
  uart1_handle.set_baudrate = uart1_set_baudrate;
  uart1_handle.clear_rx_buffer = uart1_clear_rx_buffer;
  uart1_handle.write = uart1_write;

  RPB4Rbits.RPB4R = 0x0001;	// RB4->UART1:U1TX
  U1RXRbits.U1RXR = 0x0002;	// RA4->UART1:U1RX

  // UART1 parameter.
  U1MODE = 0x0008;		// enable BRGH
  U1STA = 0x8000;		// UTXISEL=0b10
  uart1_set_baudrate( 9600 );

  // interrupt.
  IPC8bits.U1IP = 1;		// interrupt level
  IPC8bits.U1IS = 0;
  IEC1bits.U1RXIE = 1;		// Rx interrupt

  // Enabling UART1
  U1STAbits.UTXEN = 1;		// TX enable
  U1STAbits.URXEN = 1;		// RX enable
  U1MODEbits.ON = 1;		// UART1 enable

  /* UART2
      TxD: RPB9(18pin)
      RxD: RPB8(17pin)
  */
  uart2_handle.p_txbuf = 0;
  uart2_handle.rx_overflow = 0;
  uart2_handle.delimiter = '\n';
  uart2_handle.rx_rd = 0;
  uart2_handle.rx_wr = 0;
  uart2_handle.set_baudrate = uart2_set_baudrate;
  uart2_handle.clear_rx_buffer = uart2_clear_rx_buffer;
  uart2_handle.write = uart2_write;

  RPB9Rbits.RPB9R = 0x0002;	// RB9->UART2:U2TX
  U2RXRbits.U2RXR = 0x0004;	// RB8->UART2:U2RX

  // UART2 parameter.
  U2MODE = 0x0008;		// enable BRGH
  U2STA = 0x8000;		// UTXISEL=0b10
  uart2_set_baudrate( 9600 );

  // interrupt.
  IPC9bits.U2IP = 1;		// interrupt level
  IPC9bits.U2IS = 0;
  IEC1bits.U2RXIE = 1;		// Rx interrupt

  // Enabling UART2
  U2STAbits.UTXEN = 1;		// TX enable
  U2STAbits.URXEN = 1;		// RX enable
  U2MODEbits.ON = 1;		// UART2 enable
}


//================================================================
/*! Send out binary data.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
  @param  buffer        Pointer of buffer.
  @param  size          Size of buffer.
  @return               Size of transmitted.
*/
int uart_write( UART_HANDLE *uh, const void *buffer, int size )
{
  // Wait for transfer complete.
  while( !uart_is_write_finished( uh ) ) {
    Nop(); Nop(); Nop(); Nop();
  }

  uart_write_non_block( uh, buffer, size );

  // Wait for transfer complete.
  while( !uart_is_write_finished( uh ) ) {
    Nop(); Nop(); Nop(); Nop();
  }

  return size;
}


//================================================================
/*! Receive binary data.

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @param  buffer	Pointer of buffer.
  @param  size		Size of buffer.
  @return int		Num of received bytes.

  @note			If no data received, it blocks execution.
*/
int uart_read( UART_HANDLE *uh, void *buffer, size_t size )
{
  if( size == 0 ) return 0;

  // wait for data.
  while( !uart_is_readable(uh) ) {
    Nop(); Nop(); Nop(); Nop();
  }

  // copy fifo to buffer
  uint8_t *buf = buffer;
  size_t   cnt = size;
  uint16_t rx_rd;

  do {
    rx_rd = uh->rx_rd;
    *buf++ = uh->rxfifo[rx_rd++];
    if( rx_rd >= sizeof(uh->rxfifo) ) rx_rd = 0;
    uh->rx_rd = rx_rd;
  } while( --cnt != 0 && rx_rd != uh->rx_wr );

  return size - cnt;
}


//================================================================
/*! Receive string.

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @param  buffer	Pointer of buffer.
  @param  size		Size of buffer.
  @return int		string length.

  @note			If no data received, it blocks execution.
*/
int uart_gets( UART_HANDLE *uh, void *buffer, size_t size )
{
  int len;

  do {
    len = uart_can_read_line( uh );
    if( len < 0 ) return 0;
  } while( len == 0 );

  if( len >= size ) {
    len = size - 1;
  }

  uart_read( uh, buffer, len );
  ((char *)buffer)[len] = 0;
  return len;
}


//================================================================
/*! check data length can be read.

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @return int		result (bytes)
*/
int uart_bytes_available( UART_HANDLE *uh )
{
  uint16_t rx_wr = uh->rx_wr;

  if( uh->rx_rd <= rx_wr ) {
    return rx_wr - uh->rx_rd;
  }
  else {
    return sizeof(uh->rxfifo) - uh->rx_rd + rx_wr;
  }
}


//================================================================
/*! check data can be read a line.

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @return int		string length.
  @note
   If RX-FIFO buffer is full, return -1.
*/
int uart_can_read_line( UART_HANDLE *uh )
{
  uint16_t idx   = uh->rx_rd;
  uint16_t rx_wr = uh->rx_wr;

  if( uh->rx_overflow ) return -1;

  while( idx != rx_wr ) {
    if( uh->rxfifo[idx++] == uh->delimiter ) {
      if( uh->rx_rd < idx ) {
	return idx - uh->rx_rd;
      } else {
	return sizeof(uh->rxfifo) - uh->rx_rd + idx;
      }
    }
    if( idx >= sizeof(uh->rxfifo)) idx = 0;
  }

  return 0;
}
