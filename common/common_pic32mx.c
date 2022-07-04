/*
  common functions
*/

#include "clock_freq.h"


//================================================================
/*! spin lock delay functions.
*/
static void delay_ticks( uint32_t ticks )
{
  uint32_t t_start = _CP0_GET_COUNT();

  while( (_CP0_GET_COUNT() - t_start) < ticks )
    ;
}

// emulate PIC16/24 delay function.
void __delay_us( uint32_t us )
{
  delay_ticks( us * (_XTAL_FREQ/2/1000000) );
}

void __delay_ms( uint32_t ms )
{
  delay_ticks( ms * (_XTAL_FREQ/2/1000) );
}


//================================================================
/*! system register lock / unlock
*/
void system_register_lock(void)
{
  SYSKEY = 0x0;
}

void system_register_unlock(void)
{
  unsigned int status;

  // Suspend or Disable all Interrupts
  asm volatile ("di %0" : "=r" (status));

  // starting critical sequence
  SYSKEY = 0x33333333; // write invalid key to force lock
  SYSKEY = 0xAA996655; // write key1 to SYSKEY
  SYSKEY = 0x556699AA; // write key2 to SYSKEY

  // Restore Interrupts
  if (status & 0x00000001) {
    asm volatile ("ei");
  } else {
    asm volatile ("di");
  }
}


//================================================================
/*! software reset	DS60001118H
*/
void system_reset(void)
{
  system_register_unlock();

  RSWRSTSET = 1;

  // read RSWRST register to trigger reset
  uint32_t dummy = RSWRST;
  (void)dummy;

  while(1)
    ;
}
