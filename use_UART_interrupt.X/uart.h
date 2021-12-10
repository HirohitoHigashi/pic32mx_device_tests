/*! @file
 @brief
 PIC32 UARTを2つ使った送受信関数

 @author hirohito
 @version 1.5
 @date 2021/07/11 12:15:16
 @note
 <pre>
 This file is destributed under 3-Cause BSD License.
 </pre>
*/

/***** Feature test switches ************************************************/
#ifndef	__PIC32MX_UART2_H_proto_
#define	__PIC32MX_UART2_H_proto_


/***** System headers *******************************************************/
#include <stdint.h>
#include <string.h>

/***** Local headers ********************************************************/
/***** Constat values *******************************************************/
#ifndef UART_SIZE_RXFIFO
# define UART_SIZE_RXFIFO 128
#endif

/***** Macros ***************************************************************/
/***** Typedefs *************************************************************/
//================================================================
/*!@brief
  UART Handle
*/
typedef struct UART_HANDLE {
  //! @privatesection
  // for transmit
  volatile const uint8_t *p_txbuf;		// pointer to tx buffer.
  uint16_t size_txbuf;				// size of tx buffer.
  volatile uint16_t idx_txbuf;			// index of sendout bytes.

  // for receive
  uint8_t rx_overflow;	// buffer overflow flag.
  uint8_t delimiter;	//!<@public read line delimiter. default '\\n'.

  volatile uint16_t rx_rd;			// index of rxfifo for read.
  volatile uint16_t rx_wr;			// index of rxfifo for write.
  volatile uint8_t rxfifo[UART_SIZE_RXFIFO];	// FIFO for received data.

  void (*set_baudrate)(int baudrate);
  void (*clear_rx_buffer)(void);
  void (*write)(const void *buffer, int size);

} UART_HANDLE;

/***** Global variables *****************************************************/
extern UART_HANDLE uart1_handle, uart2_handle;

/***** Function prototypes **************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void uart_init(void);
int uart_write(UART_HANDLE *uh, const void *buffer, int size);
int uart_read(UART_HANDLE *uh, void *buffer, size_t size);
int uart_gets(UART_HANDLE *uh, void *buffer, size_t size);
int uart_bytes_available(UART_HANDLE *uh);
int uart_can_read_line(UART_HANDLE *uh);


/***** Inline functions *****************************************************/
//================================================================
/*! set baud-rate

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @param  baudrate	baudrate.
*/
static inline void uart_set_baudrate( const UART_HANDLE *uh, int baudrate )
{
  uh->set_baudrate( baudrate );
}


//================================================================
/*! Send out binary data without blocking.

  @memberof UART_HANDLE
  @param  uh            Pointer of UART_HANDLE.
  @param  buffer        Pointer of buffer.
  @param  size          Size of buffer.
*/
static inline void uart_write_non_block( UART_HANDLE *uh, const void *buffer, int size )
{
  uh->write( buffer, size );
}


//================================================================
/*! Transmit string.

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @param  s		Pointer of buffer.
  @return		Size of transmitted.
*/
static inline int uart_puts( UART_HANDLE *uh, const void *s )
{
  return uart_write( uh, s, strlen(s) );
}


//================================================================
/*! Transmit a character. (1 byte)

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @param  ch		character
  @return		Size of transmitted.
*/
static inline int uart_putc(UART_HANDLE *uh, int ch)
{
  uint8_t buf[1];

  buf[0] = ch;
  return uart_write(uh, buf, 1);
}


//================================================================
/*! Receive a character. (1 byte)

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @return int		Received character.
*/
static inline int uart_getc(UART_HANDLE *uh)
{
  uint8_t buf[1];

  uart_read(uh, buf, 1);
  return buf[0];
}


//================================================================
/*! check write finished?

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @return int		result (bool)
*/
static inline int uart_is_write_finished(const UART_HANDLE *uh)
{
  return uh->p_txbuf == 0;
}


//================================================================
/*! check data can be read.

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
  @return int		result (bool)
*/
static inline int uart_is_readable( const UART_HANDLE *uh )
{
  return uh->rx_rd != uh->rx_wr;
}


//================================================================
/*! check Rx buffer overflow?

  @memberof UART_HANDLE
  @return int		result (bool)
*/
static inline int uart_is_rx_overflow( const UART_HANDLE *uh )
{
  return uh->rx_overflow;
}


//================================================================
/*! Clear receive buffer.

  @memberof UART_HANDLE
  @param  uh		Pointer of UART_HANDLE.
*/
static inline void uart_clear_rx_buffer( UART_HANDLE *uh )
{
  uh->clear_rx_buffer();
}


#ifdef __cplusplus
}
#endif
#endif
