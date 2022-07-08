/*
  SPI test  ポーリング版

  use SPI1
    SCK:	25pin (RB14) out
    SDI(MISO):	22pin (RB11) in
    SDO(MOSI):	24pin (RB13) out
    SS:
*/

#include <stdint.h>
#include "xc.h"
#include "spi.h"

//================================================================
/*! initialize
*/
int spi1_init(void)
{
  // pin assign
  TRISBbits.TRISB14 = 0;
  TRISBbits.TRISB11 = 1;
  TRISBbits.TRISB13 = 0;

  ANSELBbits.ANSB14 = 0;
  ANSELBbits.ANSB13 = 0;

				// SCK1->RB14 固定
  SDI1Rbits.SDI1R = 0x0003;	// SDI1->RB11
  RPB13Rbits.RPB13R = 0x0003;	// SDO1->RB13


  // FRMERR disabled;
  SPI1STAT = 0x0;

  // bard rate 100kHz @ PBCLK 10MHz
  SPI1BRG = 0x31;

  SPI1CON2 = 0x0;


  SPI1CON = 0x00018120;		// 0: CKP(CPOL)=0,CKE(/CPHA)=1
  //SPI1CON = 0x00018020;		// 1: CKP(CPOL)=0,CKE(/CPHA)=0
  //SPI1CON = 0x00018160;		// 2: CKP(CPOL)=1,CKE(/CPHA)=1
  //SPI1CON = 0x00018060;		// 3: CKP(CPOL)=1,CKE(/CPHA)=0

  return 0;
}

//================================================================
/*! Perform SPI data transfer. (send and receive)
  @param  send_buf	pointer to send data buffer. or NULL.
  @param  send_size	send data size (bytes).
  @param  recv_buf	pointer to receive data buffer. or NULL.
  @param  recv_size	receive data size (bytes).
  @param  flag_include	if this flag true, including receive data when sending data
*/
int spi1_transfer( const void *send_buf, int send_size,
		         void *recv_buf, int recv_size, int flag_include )
{
  // FIFO EMPTYを確実にする
  while( !SPI1STATbits.SPITBE ) {
  }

  while( !SPI1STATbits.SPIRBE ) {
    int dummy = SPI1BUF;
  }

  const uint8_t *p_send = send_buf;
  uint8_t *p_recv = recv_buf;
  int s_count = send_size;
  int r_count = send_size;

  // data send process.
  while( r_count > 0 ) {
    // write data to FIFO
    if( s_count > 0 && !SPI1STATbits.SPITBF ) {
      SPI1BUF = *p_send++;
      s_count--;
    }

    // read data from FIFO
    while( !SPI1STATbits.SPIRBE ) {
      uint8_t data = SPI1BUF;
      if( flag_include ) *p_recv++ = data;
      r_count--;
    }
  }

  // data receive process.
  s_count = recv_size;
  r_count = recv_size;
  while( r_count > 0 ) {
    // write dummy data to FIFO
    if( s_count > 0 && !SPI1STATbits.SPITBF ) {
      SPI1BUF = 0;
      s_count--;
    }

    // read data from FIFO
    while( !SPI1STATbits.SPIRBE ) {
      *p_recv++ = SPI1BUF;
      r_count--;
    }
  }

  return 0;
}
