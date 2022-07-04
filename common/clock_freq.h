#if !defined(_XTAL_FREQ)
#define _XTAL_FREQ  40000000UL
#endif

#if !defined(PBCLK)
#define PBCLK (_XTAL_FREQ / 4)
#endif
