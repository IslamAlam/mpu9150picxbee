/* 
 * File:   dof9_018.c
 * Author: kollmaj
 *
 * 
 */
#pragma config FOSC = IRC       // Oscillator Selection bits (Internal RC oscillator)
#pragma config PLLEN = OFF      // 4 X PLL Enable bit (PLL is under software control)
#pragma config PCLKEN = ON      // Primary Clock Enable bit (Primary clock enabled)
#pragma config FCMEN = ON      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor disabled) !ON for debug
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRTEN = OFF     // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 19        // Brown Out Reset Voltage bits (VBOR set to 1.9 V nominal)

// CONFIG2H
#pragma config WDTEN = OFF       // Watchdog Timer Enable bit (WDT is always enabled. SWDTEN bit has no effect.)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config HFOFST = ON      // HFINTOSC Fast Start-up bit (HFINTOSC starts clocking the CPU without waiting for the oscillator to stablize.)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled, RA3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config BBSIZ = OFF      // Boot Block Size Select bit (512W boot block size)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block not protected from table reads executed in other blocks)

#include <p18f13k22.h>
#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>                // Header file for I2C tools
#include <stdint.h>
// Two byte number defining the pause on the timer
uint8_t HIGHBYTE, LOWBYTE;
#define _XTAL_FREQ 20000000

//Address of the sensor for read
#define ADDRESS_R 0b11010001
//Address of the sensor for write
#define ADDRESS_W 0b11010000
// Registers
#define WHO_AM_I_R 0x75         // The Who am I register
#define SELF_TEST_X 0x0D
#define SELF_TEST_Y SELF_TEST_Z

// this function poll the sensor for all Accelerations and Gyroscope
void pollSensor(void);
// Global variable to store Accelerometer, Gyro and Temp
unsigned int LOWX = 0;      // Accelerometer X Low Byte								 // upper and lower bytes of degree/sec values from gyroscope
unsigned int HIGHX = 0;     // Accelerometer X High Byte
unsigned int LOWY = 0;      // Accelerometer Y Low Byte
unsigned int HIGHY = 0;     // Accelerometer Y High Byte
unsigned int LOWZ = 0;      // Accelerometer Z Low Byte
unsigned int HIGHZ = 0;     // Accelerometer Z High Byte
unsigned int LOWT = 0;      // Temperature Low Byte
unsigned int HIGHT = 0;     // Temperature High Byte
unsigned int LOWGX = 0;     // Gyro X Low Byte								 // upper and lower bytes of degree/sec values from gyroscope
unsigned int HIGHGX = 0;    // Gyro X High Byte
unsigned int LOWGY = 0;     // Gyro Y Low Byte
unsigned int HIGHGY = 0;    // Gyro Y High Byte
unsigned int LOWGZ = 0;     // Gyro Z Low Byte
unsigned int HIGHGZ = 0;    // Gyro Z High Byte
void initializeUART(void);
// subroutine for chk_isr to call if the interrupt is due to msg receive
void interrupt chk_isr(void);
void RC_ISR(void);
void write(unsigned char register, unsigned char value);
uint8_t read(unsigned char reg);

void interrupt chk_isr(void)
{
    if (PIR1bits.RCIF==1)           // Receive caused interrupt?
    {
         RC_ISR();
    }

    if (RCSTAbits.OERR==1)        // overflow error received, cleanup
    {
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
        uint8_t crap = RCREG;
    }

    // timer 0 interrupt
    if (INTCONbits.TMR0IF==1)
    {
       INTCONbits.TMR0IF=0;            //clear TF0
       PORTCbits.RC4 = 0;
       TMR0H=HIGHBYTE;                     //load TH0
       TMR0L=LOWBYTE;                     //load TL0
       T0CONbits.T08BIT=0;      // 16-bit timer
       T0CONbits.T0CS=0;        // internal
       T0CONbits.T0SE=0;        // on high to low
       T0CONbits.PSA = 0;       // plrescaler is assigned
       T0CONbits.T0PS2=0;
       T0CONbits.T0PS1=0;
       T0CONbits.T0PS0=1;
       T0CONbits.TMR0ON=1;             //turn on T0
    }
}
void RC_ISR()
{
   PORTCbits.RC4 = 1;              // Turn on LED to tell user
    // the byte received
   uint8_t rcvd = RCREG;

   if (rcvd == 'X')
   {
      while (PIR1bits.TX1IF ==0);  // wait until done sending
         TXREG = HIGHX;
      while (PIR1bits.TX1IF ==0);  // wait until done sending
         TXREG = LOWX;
   }
   else if (rcvd == 'x')
   {
      while (PIR1bits.TX1IF ==0);  // wait until done sending
         TXREG = HIGHGX;
      while (PIR1bits.TX1IF ==0);  // wait until done sending
         TXREG = LOWGX;
   }
   else if (rcvd == 'Y')
   {
      while (PIR1bits.TX1IF ==0);  // wait until done sending
         TXREG = HIGHY;
      while (PIR1bits.TX1IF ==0);  // wait until done sending
         TXREG = LOWY;
   }
   else if (rcvd == 'y')
   {
       while (PIR1bits.TX1IF ==0);  // wait until done sending
         TXREG = HIGHGY;
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = LOWGY;
   }
   else if (rcvd == 'Z')
   {
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = HIGHZ;
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = LOWZ;
   }
   else if (rcvd == 'z')
   {
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = HIGHGZ;
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = LOWGZ;
   }
   else if (rcvd == 'T')
   {
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = HIGHT;
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = LOWT;
   }
   else if (rcvd == 't')
   {
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = HIGHT;
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = LOWT;
   }
   else if (rcvd == 'I')
   {
       while (PIR1bits.TX1IF ==0);  // wait until done sending
       // read and transmit the who am i register
       TXREG = read(WHO_AM_I_R); // should be 68
   }
   else
   {
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = HIGHT;
       while (PIR1bits.TX1IF ==0);  // wait until done sending
          TXREG = LOWT;
   }

   //poll the sensor so there will we new data next time
   pollSensor();

   // setup timer0 which will flash the LED when Transmitting
   TMR0H=HIGHBYTE;                     //load TH0
   TMR0L=LOWBYTE;                     //load TL0
   T0CONbits.T08BIT=0;      // 16-bit timer
   T0CONbits.T0CS=0;        // internal
   T0CONbits.T0SE=0;        // on high to low
   T0CONbits.PSA = 0;       // plrescaler is assigned
   T0CONbits.T0PS2=0;
   T0CONbits.T0PS1=0;
   T0CONbits.T0PS0=1;
   T0CONbits.TMR0ON=1;             //turn on T0
}

int main(int argc, char** argv) {

    // This seems to help
    __delay_ms(10);

    // Timer setup
    HIGHBYTE = 0x90;    // High Byte for the timer clock
    LOWBYTE  = 0x00;    // Low Byte for the timer clock
    ANSEL = 0x00;       // some outputs don't work without this
    ANSELH = 0x00;      // some outputs don't work without this

   TRISCbits.RC4 = 0;   // Set RC4 is output

   initializeUART();
   TRISCbits.TRISC6 = 0;       // Configure as output;
   TRISCbits.TRISC7 = 0;       // Configure as output;
   PORTCbits.RC7 = 0;
   PORTCbits.RC6 = 1;
   TRISBbits.RB6 = 1;
   TRISBbits.RB4 = 1;

   //p18 Oscillator Control 16 MHz
   OSCCONbits.IRCF2 = 1;
   OSCCONbits.IRCF1 = 1;
   OSCCONbits.IRCF0 = 1;

   // probably get rid of here to ..
   RCONbits.SBOREN = 0;         // After this delay, disable brown-out reset

   T1CONbits.TMR1ON = 0;	//Stop Timer1 counting
   TMR1H = 0x00;                // First rollover in one second
   TMR1L = 0x00;
   PIR1bits.TMR1IF = 0;         // Clear overflow flag
   // probably get rid of this to this

   write(0x6B,0b00000000);
   write(0x1A,0b00000001);

   // poll sensor and get data from the Accelerometer and Gyroscope
   pollSensor();

   PIE1bits.RCIE=1;             // enable RCV interrupt
   PIE1bits.TXIE=1;             // enable TX interrupt
   INTCONbits.PEIE=1;           // enable peripheral interrupts
   INTCONbits.GIE=1;            // enable all interrupts globally
   INTCONbits.TMR0IE=1;         // enable Timer0 interrupt

   while(1);
}

void pollSensor(void)
{
   PORTCbits.RC7 = 1;
   OpenI2C(MASTER,SLEW_OFF);
   SSPADD=0x09;
   StartI2C();
   WriteI2C(ADDRESS_W);  		// Write slave with address
   WriteI2C(0x3B);       		// Asks for the upper byte of the x axis register
   //WriteI2C(0x75);       		// Asks for the upper byte of the x axis register
   RestartI2C();

   unsigned int status, data;
   do
   {
	status = WriteI2C(ADDRESS_R);  //write the address of slave
	if(status == -1)		//check if bus collision happened
	{
		data = SSPBUF;		//upon bus collision detection clear the buffer,
        	SSPCON1bits.WCOL=0;	// clear the bus collision status bit
	}
   }
   while(status!=0);			//write untill successful communicatio

   HIGHX = ReadI2C();      		// Read the Bus
   AckI2C();				// Increment Register
   LOWX = ReadI2C();      		// Read the Bus
   AckI2C();				// Increment Register
   HIGHY = ReadI2C();      		// Read the Bus
   AckI2C();				//  Increment Register
   LOWY = ReadI2C();      		// Read the Bus
   AckI2C();				//  Increment Register
   HIGHZ = ReadI2C();      		// Read the Bus
   AckI2C();				// Increment Register
   LOWZ = ReadI2C();      		// Read the Bus
   AckI2C();
   HIGHT = ReadI2C();      		// Read the Bus
   AckI2C();				// Increment Register
   LOWT = ReadI2C();      		// Read the Bus
   AckI2C();
   HIGHGX = ReadI2C();      		// Read the Bus
   AckI2C();                            // Increment Register
   LOWGX = ReadI2C();                   // Read the Bus
   AckI2C();
   HIGHGY = ReadI2C();      		// Read the Bus
   AckI2C();                            // Increment Register
   LOWGY = ReadI2C();                   // Read the Bus
   AckI2C();
   HIGHGZ = ReadI2C();      		// Read the Bus
   AckI2C();                            // Increment Register
   LOWGZ = ReadI2C();                   // Read the Bus
   NotAckI2C();
   while( SSPCON2bits.ACKEN!=0);	//wait till ack sequence is complete
   StopI2C();
   CloseI2C();				// Turn off I2C bus
   PORTCbits.RC7 = 0;
}

void initializeUART(void)
{
    // p. 176 The TRIS control
    // bits corresponding to the RX/DT and TX/CK pins should
    // be set to 1. The EUSART control will automatically
    // reconfigure the pin from input to output, as needed.
    TRISBbits.RB5 = 1;
    TRISBbits.RB7 = 1;

    // The EUSART transmitter is enabled for asynchronous
    // operations by configuring the following three control
    // bits:
    // TXEN = 1
    // SYNC = 0
    // SPEN = 1
    TXSTAbits.TXEN = 1;
    TXSTAbits.SYNC = 0;
    RCSTAbits.SPEN = 1;

    //The EUSART receiver is enabled for asynchronous
    //operation by configuring the following three control bits:
    //    CREN = 1
    //    SYNC = 0
    //    SPEN = 1
    RCSTAbits.CREN = 1;            // Asynchronous mode: Enable receiver

    // defining buad rate of 19200
    // 16 MHz
    //BAUDCONbits.BRG16 = 1;
    BAUDCONbits.BRG16 = 1;
    TXSTAbits.BRGH1 = 1;        // high speed
    // if SPBRG = 25  then baud rate =  9600    page 190
    //    SPBRG = 23       baud rate = 10407
    //    SPBRG = 12       baud rate = 19200
    SPBRG = 207;
}

void write(unsigned char reg,  unsigned char value)
{
   OpenI2C(MASTER,SLEW_OFF);
   SSPADD=0x09;
   StartI2C();
   WriteI2C(ADDRESS_W);  	// Write slave with address
   WriteI2C(reg);               // This is the register to write to
   WriteI2C(value);             // This is the value to put in the register
   NotAckI2C();
   while( SSPCON2bits.ACKEN!=0);//wait till ack sequence is complete
   StopI2C();
   CloseI2C();
}
uint8_t read(unsigned char reg)
{
   OpenI2C(MASTER,SLEW_OFF);
   SSPADD=0x09;
   StartI2C();
   WriteI2C(ADDRESS_W);  		// Write slave with address
   WriteI2C(reg);       		//
   //uint8_t c =  readByte(MPU9150_ADDRESS, GYRO_CONFIG);
   RestartI2C();
   WriteI2C(ADDRESS_R); 		// Read slave with address
   uint8_t out = ReadI2C();      		// Read the Bus
   NotAckI2C();
   while( SSPCON2bits.ACKEN!=0);		//wait till ack sequence is complete
   StopI2C();
   CloseI2C();
   return out;
}
