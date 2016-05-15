

#include <xc.h>
//#include <p24FJ256GB108.h>

#include "CONFIG.h"
#include "DataTypes.h"

#include <stdio.h>
#include <uart.h>
#include "UART.h"
#include "GPIOx.h"
#include "GPS_Parser.h"
#include "GPS_Utilies.h"
#include "timer.h"

//volatile char cReceiveGPS = 0;

unsigned int state = 0;
unsigned char temp1;

void TimerInit(void);

int main(void)
{
    uint8 u8ForCountL = 0;
    char PrintBuf[16];
    //Disable Watch Dog Timer
    RCONbits.SWDTEN	 = 0;

    //Set up I/O Port
    AD1PCFGL	 =	0xFFFF;             //set to all digital I/O
    TRISB        =	0xF3FF;             //configure all PortB as input

    LedInit();
    
//    TimerInit();
    UARTsInit();



    while(1)
    {
        if( ReceiveGPSFlagMask == ( u16ByteFlags & ReceiveGPSFlagMask ) )
        {
            GPS_ReceiveChar(cReceiveGPS, GPS_Msg_Buff);
            u16ByteFlags &= ~ReceiveGPSFlagMask;
        }
        
//        if( ReceivedMsgFlagMask == ( u16ByteFlags & ReceivedMsgFlagMask ) )
//        {
//            for(u8ForCountL = 0; ((GPS_Msg_Buff_Ptr - 1) >= u8ForCountL); u8ForCountL++)
//            {
//                UART1PutChar( GPS_Msg_Buff[u8ForCountL] );
//            }
//
//            u16ByteFlags &= ~ReceivedMsgFlagMask;
//        }
        
        if( ReceivedMsgFlagMask == ( u16ByteFlags & ReceivedMsgFlagMask ) )
        {

            print_long(last_gps_location.x,9,0,PrintBuf);
            
            UART1PutChar(13); UART1PutChar(10);
            
            for(u8ForCountL = 0; (8 >= u8ForCountL); u8ForCountL++)
            {
                UART1PutChar( PrintBuf[u8ForCountL] );
            }
            u16ByteFlags &= ~ReceivedMsgFlagMask;
        }
    }

    return(0);
}

