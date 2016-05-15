

#include <xc.h>
#include <uart.h>
#include "UART.h"

volatile char cReceiveGPS;
volatile uint8 ExecTransmit = 0;
volatile uint16 u16ByteFlags;

/////////////////////////////////////////////////////////////////////////
//UART transmit function, parameter Ch is the character to send
void UART1PutChar(char Ch)
{
   //transmit ONLY if TX buffer is empty
   while(U1STAbits.UTXBF == 1);
   U1TXREG = Ch;
}
/////////////////////////////////////////////////////////////////////////
//UART receive function, returns the value received.
char UART1GetChar()
{
   char Temp;
   //wait for buffer to fill up, wait for interrupt
   while(IFS0bits.U1RXIF == 0);
   Temp = U1RXREG;
   //reset interrupt
   IFS0bits.U1RXIF = 0;
   //return my received byte
   return Temp;
}
/////////////////////////////////////////////////////////////////////////
//UART transmit function, parameter Ch is the character to send
void UART2PutChar(char Ch)
{
   //transmit ONLY if TX buffer is empty
   while(U2STAbits.UTXBF == 1);
   U2TXREG = Ch;
}
/////////////////////////////////////////////////////////////////////////
//UART receive function, returns the value received.
char UART2GetChar()
{
   char Temp;
   //wait for buffer to fill up, wait for interrupt
   while(IFS1bits.U2RXIF == 0);
   Temp = U2RXREG;
   //reset interrupt
   IFS1bits.U2RXIF = 0;
   //return my received byte
   return Temp;
}
/////////////////////////////////////////////////////////////////////////
//UART transmit function, parameter Ch is the character to send
void UART3PutChar(char Ch)
{
   //transmit ONLY if TX buffer is empty
   while(U3STAbits.UTXBF == 1);
   U3TXREG = Ch;
}
/////////////////////////////////////////////////////////////////////////
//UART receive function, returns the value received.
char UART3GetChar()
{
   char Temp;
   //wait for buffer to fill up, wait for interrupt
   while(IFS5bits.U3RXIF == 0);
   Temp = U3RXREG;
   //reset interrupt
   IFS5bits.U3RXIF = 0;
   //return my received byte
   return Temp;
}
/////////////////////////////////////////////////////////////////////////
//UART transmit function, parameter Ch is the character to send
void UART4PutChar(char Ch)
{
   //transmit ONLY if TX buffer is empty
   while(U4STAbits.UTXBF == 1);
   U4TXREG = Ch;
}
/////////////////////////////////////////////////////////////////////////
//UART receive function, returns the value received.
char UART4GetChar()
{
   char Temp;
   //wait for buffer to fill up, wait for interrupt
   while(IFS5bits.U4RXIF == 0);
   Temp = U4RXREG;
   //reset interrupt
   IFS5bits.U4RXIF = 0;
   //return my received byte
   return Temp;
}
/////////////////////////////////////////////////////////////////////////
void __attribute__((__interrupt__)) _U1RXInterrupt(void);
void __attribute__((__interrupt__, auto_psv)) _U1RXInterrupt(void)
{
    unsigned int ucDataL = 0;
    ucDataL = getcUART1();
//    putcUART1(ucDataL);
    putcUART2(ucDataL);
    U1RX_Clear_Intr_Status_Bit;
}
/////////////////////////////////////////////////////////////////////////

void __attribute__((__interrupt__)) _U2RXInterrupt(void);
void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void)
{
//    char cReceiveGPS = 0;
    cReceiveGPS = getcUART2();
    if( 1 == ExecTransmit ){ putcUART1(cReceiveGPS); }
    
    u16ByteFlags |= ReceiveGPSFlagMask;

    U2RX_Clear_Intr_Status_Bit;
}
///////////////////////////////////////////////////////////////////////

void __attribute__((__interrupt__)) _U3RXInterrupt(void);
void __attribute__((__interrupt__, auto_psv)) _U3RXInterrupt(void)
{
    unsigned int ucDataL = 0;
    ucDataL = getcUART3();
    putcUART1(ucDataL);
    
    U3RX_Clear_Intr_Status_Bit;
}
/////////////////////////////////////////////////////////////////////////

void __attribute__((__interrupt__)) _U4RXInterrupt(void);
void __attribute__((__interrupt__, auto_psv)) _U4RXInterrupt(void)
{
    unsigned int ucDataL = 0;
    ucDataL = getcUART4();
    putcUART1(ucDataL);
    
    U4RX_Clear_Intr_Status_Bit;
}
/////////////////////////////////////////////////////////////////////////
void UARTsInit(void)
{
//Per
    RPINR18bits.U1RXR   =	15;     //UART0 receive set to R 15
    RPOR15bits.RP30R    =	3;      //UART0 transmit set to R 30
//LCD
    RPINR19bits.U2RXR   =	38;     //UART0 receive set to R 38
    RPOR1bits.RP2R      =	5;      //UART0 transmit set to R 2
//Exc
    RPINR17bits.U3RXR   =	3;      //UART0 receive set to R 3
    RPOR2bits.RP4R      =	28;      //UART0 transmit set to R 4
//GSM
    RPINR27bits.U4RXR   =	20;     //UART0 receive set to R 20
    RPOR12bits.RP25R    =	30;      //UART0 transmit set to R 25

//9600 U2BRG = 416;
//11520 U1BRG = 34;

//250000 U1BRG = 15;
    
   //Set up registers
   U1BRG = 15;
   
   U1MODEbits.BRGH      = 1;    //4x baud clock
   U1MODEbits.UARTEN    = 1;    //turn on module
   
   U1STAbits.UTXISEL1   = 1;    //Interrupt when 
   U1STAbits.UTXISEL0   = 0;    //the transmit buffer becomes empty
   U1STAbits.UTXEN      = 1;

   //Set up registers
   U2BRG = 416;
   
   U2MODEbits.BRGH      = 1;    //4x baud clock
   U2MODEbits.UARTEN    = 1;    //turn on module
   
   U2STAbits.UTXISEL1   = 1;    //Interrupt when 
   U2STAbits.UTXISEL0   = 0;    //the transmit buffer becomes empty
   U2STAbits.UTXEN      = 1;

   //Set up registers
   U3BRG = 15;
   
   U3MODEbits.BRGH      = 1;    //4x baud clock
   U3MODEbits.UARTEN    = 1;    //turn on module
   
   U3STAbits.UTXISEL1   = 1;    //Interrupt when 
   U3STAbits.UTXISEL0   = 0;    //the transmit buffer becomes empty
   U3STAbits.UTXEN      = 1;

//   //Set up registers
//   U4BRG = 15;
//   
//   U4MODEbits.BRGH      = 1;    //4x baud clock
//   U4MODEbits.UARTEN    = 1;    //turn on module
//   
//   U4STAbits.UTXISEL1   = 1;    //Interrupt when 
//   U4STAbits.UTXISEL0   = 0;    //the transmit buffer becomes empty
//   U4STAbits.UTXEN      = 1;
   
    EnableIntU1RX;
    EnableIntU2RX;
//    EnableIntU3RX;
//    EnableIntU4RX;
   
//reset RX interrupt flag
    U1RX_Clear_Intr_Status_Bit;
    U2RX_Clear_Intr_Status_Bit;
    U3RX_Clear_Intr_Status_Bit;
    U4RX_Clear_Intr_Status_Bit;
}
/////////////////////////////////////////////////////////////////////////
//Repeater main function
void RepeaterProcessEvents()
   {
    unsigned int ucDataL = 0;

   //wait for data to be received
   ucDataL = UART1GetChar();

   //send data back on UART TX line
   UART1PutChar(ucDataL);
}
