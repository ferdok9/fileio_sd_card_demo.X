
#include "GPIOx.h"

void LedInit(void)
{
    TRISGbits.TRISG1 = 0;
    TRISFbits.TRISF1 = 0;
    
    PORTGbits.RG1 = 0;
    PORTFbits.RF1 = 0;
}
