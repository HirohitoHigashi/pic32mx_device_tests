/*
  Flash Test

  実行時自己プログラミング (RTSP)動作のテスト
*/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/kmem.h>

#include "xc.h"

#if defined(__32MX270F256B__) || defined(__PIC32MX270F256B__)
# include "init_32mx270f256b.c"
#else
# error No initialize file found.
#endif
#include "clock_freq.h"
#include "common_pic32mx.c"
#include "uart1.c"


#define FLASH_SAVE_ADDR (0x9D000000 + 0x8000)
#define FLASH_END_ADDR  (0x9D03EFFF)
#define PAGE_SIZE 1024
#define ROW_SIZE (PAGE_SIZE / 8)
/* (note)
 SAVE_ADDRは、ProgramFlashの先頭アドレス 0x9d00_0000に、本プログラムサイズを
 十分な余裕を見込んで 0x8000 オフセットした値とした。
 mapファイルによると、ProgramFlashの終了アドレス(0x9D03FFFF)付近に
 割り込みベクタが配置され、その先頭は 0x9d03_f180からである。
 END_ADDRは、切りの良いところで 0x9d03_efff をエンドアドレスとした。
*/


//================================================================
/*! NVM unlock and execute nvm operation.

  @param nvmop	NVM Operation bits. (see: DS60001121G, Sect 5.2.1)
  @return	not zero if errors.
*/
unsigned int NVMUnlock(unsigned int nvmop)
{
  unsigned int status;

  // Suspend or Disable all Interrupts
  asm volatile ("di %0" : "=r" (status));

  // clearing error bits before performing an NVM operation
  NVMCONCLR = 0x0f;

  // Enable Flash Write/Erase Operations and Select
  // Flash operation to perform
  NVMCON = (0x4000 | nvmop);

  // Write Keys
  NVMKEY = 0xAA996655;
  NVMKEY = 0x556699AA;

  // Start the operation using the Set Register
  NVMCONSET = 0x8000;

  // Wait for operation to complete
  while (NVMCON & 0x8000);

  // Restore Interrupts
  if (status & 0x00000001) {
    asm volatile ("ei");
  } else {
    asm volatile ("di");
  }

  // Disable NVM write enable
  NVMCONCLR = 0x4000;

  // Return WRERR and LVDERR Error Status Bits
  return (NVMCON & 0x3000);
}


//================================================================
/*! Erase one *PAGE* of FLASH ROM

  @param  address	flash rom address
  @return		not zero if errors.
*/
unsigned int NVMErasePage(void* address)
{
  // Set NVMADDR to the Start Address of page to erase
  NVMADDR = KVA_TO_PA(address);

  // Unlock and Erase Page
  return NVMUnlock( 0x4 );	// 0x4 = Page Erase Operation
}


//================================================================
/*! Write one *WORD* of FLASH ROM

  @param  address	flash rom address
  @param  data		write data
  @return		not zero if errors.
*/
unsigned int NVMWriteWord(void *address, uint32_t data)
{
  // Load data into NVMDATA register
  NVMDATA = data;

  // Load address to program into NVMADDR register
  NVMADDR = KVA_TO_PA(address);

  // Unlock and Write Word
  return NVMUnlock( 0x1 );	// 0x1 = Word Program Operation
}


//================================================================
/*! Write one *ROW* of FLASH ROM

  @param  address	flash rom address
  @param  data		write data address
  @return		not zero if errors.
*/
unsigned int NVMWriteRow(void *address, void *data)
{
  unsigned int res;

  // Set NVMADDR to Start Address of row to program
  NVMADDR = KVA_TO_PA(address);

  // Set NVMSRCADDR to the SRAM data buffer Address
  NVMSRCADDR = KVA_TO_PA(data);

  // Unlock and Write Row
  return NVMUnlock( 0x3 );	// 0x3 = Row Program Operation
}


void hexdump( uint8_t *address )
{
  int i;
  for( i = 0; i < 16; i++ ) {
    printf("%02x ", address[i] );
  }
  printf("\n");
}


uint8_t row_buffer[ROW_SIZE];


int main(void) {
  __XC_UART = 1;

  system_init();	// Initialize I/O, OSC and Interrupt.

  uart_init();
  printf("\r\n\x1b(B\x1b)B\x1b[0m\x1b[2JSystem start.\n");

  printf("RAM address = %p\nROM address = %p\n", row_buffer, main );
  printf("Start\n");
  hexdump( (uint8_t *)FLASH_SAVE_ADDR );

  printf("\nErase\n");
  NVMErasePage( (uint8_t *)FLASH_SAVE_ADDR );
  hexdump( (uint8_t *)FLASH_SAVE_ADDR );

  printf("\nWord Write\n");
  NVMWriteWord( (uint8_t *)FLASH_SAVE_ADDR+4, 0x01234567 );
  hexdump( (uint8_t *)FLASH_SAVE_ADDR );
  NVMWriteWord( (uint8_t *)FLASH_SAVE_ADDR+8, 0x89abcdef );
  hexdump( (uint8_t *)FLASH_SAVE_ADDR );

  printf("\nPage Write\n");
  int i;
  for( i = 0; i < sizeof(row_buffer); i++ ) {
    row_buffer[i] = i;
  }
  NVMWriteRow( (uint8_t *)FLASH_SAVE_ADDR, row_buffer );
  hexdump( (uint8_t *)FLASH_SAVE_ADDR );


  return 0;
}
