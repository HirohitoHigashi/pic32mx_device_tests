
// DEVCFG3
#pragma config FSRSSEL = PRIORITY_7    // Shadow Register Set Priority Select->SRS Priority 7
#pragma config PMDL1WAY = ON    // Peripheral Module Disable Configuration->Allow only one reconfiguration
#pragma config IOL1WAY = ON    // Peripheral Pin Select Configuration->Allow only one reconfiguration

// DEVCFG2
#pragma config FPLLIDIV = DIV_2    // PLL Input Divider->2x Divider
#pragma config FPLLMUL = MUL_20    // PLL Multiplier->20x Multiplier
#pragma config FPLLODIV = DIV_2    // System PLL Output Clock Divider->PLL Divide by 2

// DEVCFG1
#pragma config FNOSC = FRCPLL    // Oscillator Selection Bits->Fast RC Osc with PLL
#pragma config FSOSCEN = OFF    // Secondary Oscillator Enable->Disabled
#pragma config IESO = ON    // Internal/External Switch Over->Enabled
#pragma config POSCMOD = OFF    // Primary Oscillator Configuration->Primary osc disabled
#pragma config OSCIOFNC = OFF    // CLKO Output Signal Active on the OSCO Pin->Disabled
#pragma config FPBDIV = DIV_4    // Peripheral Clock Divisor->Pb_Clk is Sys_Clk/4
#pragma config FCKSM = CSDCMD    // Clock Switching and Monitor Selection->Clock Switch Disable, FSCM Disabled
#pragma config WDTPS = PS1048576    // Watchdog Timer Postscaler->1:1048576
#pragma config WINDIS = OFF    // Watchdog Timer Window Enable->Watchdog Timer is in Non-Window Mode
#pragma config FWDTEN = OFF    // Watchdog Timer Enable->WDT Disabled (SWDTEN Bit Controls)
#pragma config FWDTWINSZ = WINSZ_25    // Watchdog Timer Window Size->Window Size is 25%

// DEVCFG0
#pragma config DEBUG = OFF    // Background Debugger Enable->Debugger is Disabled
#pragma config JTAGEN = OFF    // JTAG Enable->JTAG Disabled
#pragma config ICESEL = ICS_PGx1    // ICE/ICD Comm Channel Select->Communicate on PGEC1/PGED1
#pragma config PWP = OFF    // Program Flash Write Protect->Disable
#pragma config BWP = OFF    // Boot Flash Write Protect bit->Protection Disabled
#pragma config CP = OFF    // Code Protect->Protection Disabled


void system_init( void )
{
  // Setting the Output Latch SFR(s)
  LATB = 0x0000;
  LATC = 0x0000;
  LATD = 0x0000;
  LATE = 0x0000;
  LATF = 0x0000;
  LATG = 0x0000;

  // Setting the GPIO Direction SFR(s)
  TRISB = 0xFFFF;
  TRISC = 0xF000;
  TRISD = 0x0FFF;
  TRISE = 0x00FF;
  TRISF = 0x007F;
  TRISG = 0x03CC;

  // Setting the Weak Pull Up and Weak Pull Down SFR(s)
  CNPDB = 0x0000;
  CNPDC = 0x0000;
  CNPDD = 0x0000;
  CNPDE = 0x0000;
  CNPDF = 0x0000;
  CNPDG = 0x0000;
  CNPUB = 0x0000;
  CNPUC = 0x0000;
  CNPUD = 0x0000;
  CNPUE = 0x0000;
  CNPUF = 0x0000;
  CNPUG = 0x0000;

  // Setting the Open Drain SFR(s)
  ODCB = 0x0000;
  ODCC = 0x0000;
  ODCD = 0x0000;
  ODCE = 0x0000;
  ODCF = 0x0000;
  ODCG = 0x0000;

  // Setting the Analog/Digital Configuration SFR(s)
  ANSELB = 0xFFFC;	// RB0,RB1 assign programming port.
  ANSELD = 0x000E;
  ANSELE = 0x00F4;
  ANSELG = 0x03C0;


  // Enable the multi vector
  INTCONbits.MVEC = 1;
  // Enable Global Interrupts
  __builtin_mtc0(12,0,(__builtin_mfc0(12,0) | 0x0001));
}
