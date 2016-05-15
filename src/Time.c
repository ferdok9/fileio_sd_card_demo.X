
#include "Time.h"

//Set up Timer, target 2Hz interrupts
void TimerInit(void)
{
//1ms
    PR1 = 2000;	 
    
    T1CONbits.TCS = 0;//Internal clock (FOSC/2)
    T1CONbits.TCKPS = 0x1; //1:8
    
    IPC0bits.T1IP = 5;	 //set interrupt priority    
    IFS0bits.T1IF = 0;	 //reset interrupt flag
    IEC0bits.T1IE = 1;	 //turn on  timer1 interrupt
    T1CONbits.TON = 1;   //turn on the timer1
}

//_T1Interrupt() is the T1 interrupt service routine (ISR).
void __attribute__((__interrupt__)) _T1Interrupt(void);
void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void)
{
//    PORTFbits.RF1 = ~PORTFbits.RF1;
    IFS0bits.T1IF = 0;
}