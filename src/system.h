/******************************************************************************
*
*                 File I/O SD Card Demo System Header File
*
******************************************************************************
* FileName:           system_config.h
* Processor:          PIC24FJ256GB110
* Compiler:           XC16
* Company:            Microchip Technology, Inc.
*
* Software License Agreement
*
* The software supplied herewith by Microchip Technology Incorporated
* (the "Company") for its PICmicro(R) Microcontroller is intended and
* supplied to you, the Company's customer, for use solely and
* exclusively on Microchip PICmicro Microcontroller products. The
* software is owned by the Company and/or its supplier, and is
* protected under applicable copyright laws. All rights are reserved.
* Any use in violation of the foregoing restrictions may subject the
* user to criminal sanctions under applicable laws, as well as to
* civil liability for the breach of the terms and conditions of this
* license.
*
* THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*
********************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "drv_spi.h"

// Definition for system clock
#define SYS_CLK_FrequencySystemGet()        32000000
// Definition for peripheral clock
#define SYS_CLK_FrequencyPeripheralGet()    SYS_CLK_FrequencySystemGet()
// Definition for instruction clock
#define SYS_CLK_FrequencyInstructionGet()   (SYS_CLK_FrequencySystemGet() / 2)


// System initialization function
void SYSTEM_Initialize (void);

// User-defined function to set the chip select for our example drive
void USER_SdSpiSetCs_1 (uint8_t a);
// User-defined function to get the card detection status for our example drive
bool USER_SdSpiGetCd_1 (void);
// User-defined function to get the write-protect status for our example drive
bool USER_SdSpiGetWp_1 (void);
// User-defined function to initialize tristate bits for CS, CD, and WP
void USER_SdSpiConfigurePins_1 (void);

