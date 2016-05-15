
#ifndef UART1_H
#define	UART1_H

#include "DataTypes.h"

#define     MsgClockLengh 128

#define     SetSnoozeDelayFlagMask      0x0001    //0b00000001
#define     ReceiveGPSFlagMask          0x0002    //0b00000010
#define     ReceivedMsgFlagMask          0x0004    //0b00000100

extern volatile char cReceiveGPS;
extern volatile uint16 u16ByteFlags;
extern volatile uint8 ExecTransmit;

//Initiation
extern void UART1Init(int BAUDRATEREG1);

//UART transmit function
extern void UART1PutChar(char Ch);
extern void UART2PutChar(char Ch);
extern void UART3PutChar(char Ch);
extern void UART4PutChar(char Ch);

//UART receive function
extern char UART1GetChar();
extern char UART2GetChar();
extern char UART3GetChar();
extern char UART4GetChar();

void UARTsInit(void);

void RepeaterProcessEvents();


#endif	/* UART1_H */

