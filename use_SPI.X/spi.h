/*
  SPI test  ポーリング版

  use SPI1
    SCK:	25pin (RB14)
    SDI(MISO):	22pin (RB11)
    SDO(MOSI):	24pin (RB13)
*/

#include <stdint.h>

int spi1_init(void);
int spi1_transfer(const void *send_buf, int send_size, void *recv_buf, int recv_size, int flag_include);
