
#ifndef GPS_PARSER_H
#define	GPS_PARSER_H

#include <string.h>
#include "DataTypes.h"
#include "UART.h"
#include "GPS_Utilies.h"

// parser I/O variables
#define GPSMsgLengh 128

#define GPSM_MESSAGE 1
#define GPSM_LOCATION 2
#define GPSM_DISCONTINUE 4
#define GPSM_TIME 8

extern char     GPS_Msg_Buff[GPSMsgLengh];
extern uint8    GPS_Msg_Buff_Ptr;         // = 0 - waiting for '$'
//extern uint8    GPS_Rd_Ptr;

extern struct TLocation last_gps_location;

struct rtc_time 
{
    uint8    hours;
    uint8    minutes;
    uint8    seconds;
    uint8    mdate;
    uint8    month;
    uint8    year;
};

struct TLocation
{
    signed long x;
    signed long y;
    signed long z;
    unsigned int vel;
};
extern void GPS_ReceiveChar(char GPSNextChar, char * GPS_Rx_Buff);

#endif	/* GPS_PARSER_H */

