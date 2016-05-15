
#include "GPS_Parser.h"

char     GPS_Msg_Buff[GPSMsgLengh];
uint8    GPS_Msg_Buff_Ptr;         // = 0 - waiting for '$'
static uint8    GPS_Rd_Ptr;

// last read message variables
static struct rtc_time gps_rtc;
        struct TLocation last_gps_location;
static unsigned char coord_error;

static unsigned char gps_tmp_valide_mark;
static unsigned char gps_tmp_number_of_satelites;
static unsigned int gps_hdop;
static unsigned int gps_tmp_velocity;
static unsigned char gps_tmp_direction;

void GPS_ReceiveChar(char GPSNextChar, char * GPS_Rx_Buff);

// static functions from this file
static void GPSParseMsg(char * GPS_Rx_Buff);
static uint8 GPS_CheckSumeTest(char * GPS_Rx_Buff);
static uint8 AsciiNumber_To_HEX( char InputChar );

static unsigned char read_gps_time(char * GPS_Rx_Buff);
static unsigned char read_gps_date(char * GPS_Rx_Buff);
static unsigned char read_latitude(char * GPS_Rx_Buff);
static unsigned char read_longitude(char * GPS_Rx_Buff);
static unsigned char read_fix_indicator(char * GPS_Rx_Buffs);
static unsigned char read_number_of_satelites(char * GPS_Rx_Buff);
static unsigned char read_hdop(char * GPS_Rx_Buff);
static unsigned char read_altitude(char * GPS_Rx_Buff);
static unsigned char read_valide_mark(char * GPS_Rx_Buff);
static unsigned char read_velocity(char * GPS_Rx_Buff);
static unsigned char read_direction(char * GPS_Rx_Buff);
static unsigned long read_coord(char * GPS_Rx_Buff);
static long read_float_as_long(unsigned char expo, char *p);
static unsigned char goto_next_comma(char * GPS_Rx_Buff);
//static void gps_debug_msg(void);

// static return variables
static unsigned char gps_parser_result;

void GPS_ReceiveChar(char GPSNextChar, char * GPS_Rx_Buff)
{
    if (GPSNextChar == '$') 
    {
        // a start of message is received
        GPS_Msg_Buff_Ptr = 0;
        GPS_Rx_Buff[GPS_Msg_Buff_Ptr++] = GPSNextChar;
    } 
    else if (GPSNextChar == 0x0D) 
    {
        // an end of message is received
        GPS_Rx_Buff[GPS_Msg_Buff_Ptr++] = 0x0D;
        GPS_Rx_Buff[GPS_Msg_Buff_Ptr++] = 0x0A;

        GPSParseMsg(GPS_Rx_Buff);
        return;                           // read max one text line at a time
    } 
    else if ( (sizeof(GPS_Rx_Buff) - 4) > GPS_Msg_Buff_Ptr ) 
    {
        // a message body char is received
        GPS_Rx_Buff[GPS_Msg_Buff_Ptr++] = GPSNextChar;
    } 
    else 
    {
        // overflow condition
        GPS_Msg_Buff_Ptr = 0;
    }
}
//
//////////////////////////////////////////////////////////////////////////////////
uint8 GPS_CheckSumeTest(char * GPS_Rx_Buff)
{
    unsigned char  cCheckL = 0;
    uint8 cTmp1L;
    uint8 cTmp2L;
    uint8 cTmp3L;
    uint8 Msg_Rd_Ptr = 0;
    
    // calculate and compare message checksum
    Msg_Rd_Ptr = 1;
    while (GPS_Rx_Buff[Msg_Rd_Ptr] != '*') 
    {
        cCheckL ^= GPS_Rx_Buff[Msg_Rd_Ptr];
        if ( (Msg_Rd_Ptr >= GPS_Msg_Buff_Ptr) ) 
        {
            // end of text reached without '*', return
            return 0xF1;
        }
        if (  (GPS_Rx_Buff[Msg_Rd_Ptr] < ' ') ) 
        {
            // end of text reached without '*', return
            return 0xF2;
        }
        Msg_Rd_Ptr++;
    }
    cTmp1L = AsciiNumber_To_HEX(GPS_Rx_Buff[Msg_Rd_Ptr+1]);
    if (cTmp1L == 0xFF) return 0xFE;

    cTmp2L = AsciiNumber_To_HEX(GPS_Rx_Buff[Msg_Rd_Ptr+2]);
    if (cTmp2L == 0xFF) return 0xFD;    

    cTmp3L = ((cTmp1L << 4) | cTmp2L);
    
    if (cCheckL != cTmp3L) 
    {
        return 1;
    }
    
    if ( GPS_Rx_Buff[Msg_Rd_Ptr + 3] != 0x0D ) return 2;
    
    // try to identify the message type
    if (memcmp( GPS_Rx_Buff, "$GP", 3) != 0)
    {
        return 3;
    }
    
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////
void GPSParseMsg(char * GPS_Rx_Buff) 
{
    if( 0 == GPS_CheckSumeTest( GPS_Rx_Buff ) )
    {
        u16ByteFlags |= ReceivedMsgFlagMask;
//        return;
    }

    GPS_Rd_Ptr = 6;
    if (memcmp(&GPS_Rx_Buff[3], "GGA", 3) == 0) 
    {

        // $GPGGA message
        if (read_gps_time(GPS_Rx_Buff)) return;
        if (read_latitude(GPS_Rx_Buff)) return;
        if (read_longitude(GPS_Rx_Buff)) return;
        if (read_fix_indicator(GPS_Rx_Buff)) return;
        if (read_number_of_satelites(GPS_Rx_Buff)) return;
        if (read_hdop(GPS_Rx_Buff)) return;
        if (read_altitude(GPS_Rx_Buff)) return;
//        last_gps_location.timestamp = TimeStamp;
        gps_parser_result |= GPSM_MESSAGE;
//        // DEBUG message
//        gps_debug_msg();

    } 
    else if (memcmp(&GPS_Rx_Buff[3], "RMC", 3) == 0) 
    {
        // $GPRMC message
        if (read_gps_time(GPS_Rx_Buff)) return;
        if (read_valide_mark(GPS_Rx_Buff)) return;
        if (read_latitude(GPS_Rx_Buff)) return;
        if (read_longitude(GPS_Rx_Buff)) return;
        if (read_velocity(GPS_Rx_Buff)) return;
        if (read_direction(GPS_Rx_Buff)) return;
        if (read_gps_date(GPS_Rx_Buff)) return;
//        last_gps_location.timestamp = TimeStamp;
        gps_parser_result |= GPSM_MESSAGE;
//        // check for valide coordinates
//        if (last_gps_location.x && gps_tmp_valide_mark) 
//        {
//        gps_parser_result |= GPSM_LOCATION;
//        last_gps_location.type = 0xF0;
//        last_ok_location = last_gps_location;
//        last_ok_location_satelites = gps_tmp_number_of_satelites;
//        last_ok_location.direction = gps_tmp_direction;
//        last_ok_location_hdop = gps_hdop;
//        } 
//        else 
//        {
//        gps_parser_result |= GPSM_DISCONTINUE;
//        }
//        // check if valide date is got from satelite ( or localy generated )
//        if (gps_rtc.year >= 0x08)
//        {
//        gps_parser_result |= GPSM_TIME;
//        }
//        AdjustRTC( &gps_rtc, gps_tmp_valide_mark);
//        // DEBUG message
//        gps_debug_msg();

    }
}
//////////////////////////////////////////////////////////////////////////////////
uint8 AsciiNumber_To_HEX( char InputChar )
{
    uint8 u8RetValL = 0xFF;
    
    if((47 < InputChar) && (58 > InputChar))
    {
        u8RetValL = ( InputChar - 48);
    }
    else if((64 < InputChar) && (71 > InputChar))
    {
        u8RetValL = ( (InputChar - 65) + 10 );
    } 
    else if((96 < InputChar) && (103 > InputChar))
    {
        u8RetValL = ( (InputChar - 97) + 10 );
    }
    return u8RetValL;
}
//////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------//
// Try to extract GPS time from NMEA message. Expects gps_rd_ptr to point the
//   comma just before the hours, leaves gps_rd_ptr to point the comma
//   immediately after the seconds.
// In success sets gps time related variables, in error gps_hours = 0xFF;
unsigned char read_gps_time(char * GPS_Rx_Buff) 
{
    unsigned int   i;
    unsigned char  tmp;

    // check if gps_rd_ptr points to a comma ( "," )
    if (GPS_Rx_Buff[GPS_Rd_Ptr] != ',') return 1;

    GPS_Rd_Ptr++;//7
    gps_rtc.hours = get_2ascii(&GPS_Rx_Buff[GPS_Rd_Ptr]);
    gps_rtc.minutes = get_2ascii(&GPS_Rx_Buff[GPS_Rd_Ptr+2]);
    gps_rtc.seconds = get_2ascii(&GPS_Rx_Buff[GPS_Rd_Ptr+4]);

    if ((gps_rtc.hours > 23 ) || (gps_rtc.minutes > 59) || (gps_rtc.seconds > 59))
    return 1;
    GPS_Rd_Ptr += 6;//13
    // after the 6 digit need a comma or the fractional part of seconds
    if (GPS_Rx_Buff[GPS_Rd_Ptr] == ',') return 0;
    // after the 6 digit need a decimal point
    if (GPS_Rx_Buff[GPS_Rd_Ptr] != '.') return 1;
    // after the dec point need 1 - 5 digits of fractional part of seconds
    for (i=0; i<5; i++) 
    {
        GPS_Rd_Ptr++;
        if (GPS_Rx_Buff[GPS_Rd_Ptr] == ',') 
        {
            return 0;
        }
        tmp = get_ascii(&GPS_Rx_Buff[GPS_Rd_Ptr]);
        if (tmp > 9) return 1;
    }
    return 1;
}
//---------------------------------------------------------------------------//
// Tryes to extract GPS time form NMEA message and does elementary checks for
//   data validity
static unsigned char read_gps_date(char * GPS_Rx_Buff) {
  GPS_Rd_Ptr++;
  gps_rtc.mdate = get_2ascii(&GPS_Rx_Buff[GPS_Rd_Ptr]);
  gps_rtc.month = get_2ascii(&GPS_Rx_Buff[GPS_Rd_Ptr+2]);
  gps_rtc.year = get_2ascii(&GPS_Rx_Buff[GPS_Rd_Ptr+4]);
  GPS_Rd_Ptr += 6;
  // check for valide data read
  if ((gps_rtc.mdate) && (gps_rtc.mdate < 32) &&
      (gps_rtc.month) && (gps_rtc.month < 13) && (gps_rtc.year < 99)) {
    return 0;
  } else {
    return 1;
  }
}

//---------------------------------------------------------------------------//
// Tryes to extract location latitude from NMEA message
unsigned char read_latitude(char * GPS_Rx_Buff) 
{
  signed long coord;

  coord = read_coord(GPS_Rx_Buff);
  if (coord_error) return 0xFF;
  GPS_Rd_Ptr +=2;
  if (GPS_Rx_Buff[GPS_Rd_Ptr-1] == 'N') 
  {
    last_gps_location.y = coord;
    return 0;
  }
  if (GPS_Rx_Buff[GPS_Rd_Ptr-1] == 'S') 
  {
    last_gps_location.y = -coord;
    return 0;
  }
  if ((GPS_Rx_Buff[GPS_Rd_Ptr-1] == ',') && (coord == 0))  
  {
    GPS_Rd_Ptr--;
    last_gps_location.y = 0;
    return 0;
  }
  return 0xFF;
}

//---------------------------------------------------------------------------//
// Tryes to extract location longitude from NMEA message
unsigned char read_longitude(char * GPS_Rx_Buff) 
{
  signed long coord;

  coord = read_coord(GPS_Rx_Buff);
  
  if (coord_error) return 0xFF;
  
  GPS_Rd_Ptr +=2;
  if (GPS_Rx_Buff[GPS_Rd_Ptr-1] == 'E') 
  {
    last_gps_location.x = coord;
    return 0;
  }
  if (GPS_Rx_Buff[GPS_Rd_Ptr-1] == 'W') 
  {
    last_gps_location.x = -coord;
    return 0;
  }
  if ((GPS_Rx_Buff[GPS_Rd_Ptr-1] == ',') && (coord == 0))  
  {
    GPS_Rd_Ptr--;
    last_gps_location.x = 0;
    return 0;
  }
  return 0xFF;
}

//---------------------------------------------------------------------------//
// Reads from 'GPS_Rx_Buff' a number with max 4 digits fraction part and
//   makes a conversion. Conversion is necessary because input format is
//   degrees.minutes dispite of integer.fraction part of a composite number.
// Output format is a unsigned integer, containing minutes * 1E4.
// It is expected that 'GPS_Rd_Ptr' point to the comma just before the number.
// Returns 0 if any error, otherwise returns the result.
//   hmm, is it the result not a zero ...
unsigned long read_coord(char * GPS_Rx_Buff) {
    unsigned char tmp;
    int           i;
    unsigned long result = 0;

    coord_error = 1;
    // check if 'GPS_Rd_Ptr' points to ','
    if (GPS_Rx_Buff[GPS_Rd_Ptr] != ',') 
    {
        coord_error = 1;
        return 0;
    }

    // empty string, e.g. GPS receiver has no idea where we are ...
    GPS_Rd_Ptr++;
    if (GPS_Rx_Buff[GPS_Rd_Ptr] == ',')
    {
        coord_error = 0;
        return 0;
    }

    // first pass : look for a decimal point
    // somewhere it MUST end with a decimal point
    for (i=0; i<6; i++) 
    {
        tmp = get_ascii(&GPS_Rx_Buff[GPS_Rd_Ptr++]);
        if (tmp == ('.' ^ '0')) goto get_coord_fl;  // goto, you like it?
        if (tmp > 9) 
        {
            return 0;  // with 'error'
        }
        result = ( result * 10 ) + (unsigned long)( tmp );
    }
    return 0;  // with 'error'

    // result correction 100 / 60, FIXME need to make it w/o division
    get_coord_fl:
    result = (result / 100) * 60 + result % 100;

    // add the fraction part, MUST end with ','
    for (i=0; i<4; i++) 
    {
        tmp = get_ascii(&GPS_Rx_Buff[GPS_Rd_Ptr]);
        
        if (tmp < 10)
        result = (result*10) + (unsigned long)(tmp);
        
        if ((tmp == (',' ^ '0')) || (tmp == ('*' ^ '0')))
        break;
        GPS_Rd_Ptr++;
    }

    while ( i < 4 ) 
    {
        result *= 10;
        i++;
    }
    goto_next_comma(GPS_Rx_Buff);
    coord_error = 0;
    return result;
}
//---------------------------------------------------------------------------//
unsigned char goto_next_comma(char * GPS_Rx_Buff) 
{
  while ((GPS_Rx_Buff[GPS_Rd_Ptr] != ',') &&
         (GPS_Rd_Ptr < sizeof(GPS_Rx_Buff)-4)) 
  {
    GPS_Rd_Ptr++;
  }
  return 0;
}
//---------------------------------------------------------------------------//
// Tryes to extract location altitude from NMEA message
// The result stored in last_gps_location.z is actual altitude in [meters] mult by 10
unsigned char read_altitude(char * GPS_Rx_Buff) 
{
  GPS_Rd_Ptr++;
  last_gps_location.z = read_float_as_long(1, &GPS_Rx_Buff[GPS_Rd_Ptr]) + 1000;
  return goto_next_comma(GPS_Rx_Buff);
}
//---------------------------------------------------------------------------//
// Reads a float.
// Returns long = float * 1E'expo', alse returns 0 if any error
// Math library is not used in case of inefficiency.
long read_float_as_long(unsigned char expo, char *p) {
  unsigned long temp = 0;
  unsigned char negative = 0;
  unsigned char is_digit = 0;
  unsigned char tmp, tmp1;
  int  i;
  // first pass - look for a decimal point
  for (i=0; i<12; i++) {
    tmp = get_ascii(p); p++;
    if (tmp < 10) {
      temp = (temp*10) + (unsigned long)(tmp);
      is_digit = 1;
    }
    tmp1 = tmp ^ '0';
    if ((tmp1 == '.') || (tmp1 == ',') || (tmp1 == 0x0D))
      break;
    if (tmp1 == '-') {
      if ((negative) || (is_digit)) return 0;
      negative = 1;
    } else {
      if (tmp > 9) return 0;
    }
  }

  // second pass - read fraction part
  is_digit = 1;
  if (expo) {
    for (i=0; i<expo; i++) {
       tmp = get_ascii(p); p++;
       if (tmp > 9) is_digit = 0;
       temp = (temp*10) + (unsigned long)(tmp * is_digit);
     }
  }  // if (expo)

  // set sign
  if (negative ) return -temp;
  else           return  temp;
}
//---------------------------------------------------------------------------//
// Tryes to extract location fix indicator from NMEA message
unsigned char read_fix_indicator(char * GPS_Rx_Buff) 
{
  unsigned char tmp;

  GPS_Rd_Ptr++;
  tmp = get_ascii(&GPS_Rx_Buff[GPS_Rd_Ptr]);
  if (tmp > 9) return 0xFF;
  GPS_Rd_Ptr++;
  if (GPS_Rx_Buff[GPS_Rd_Ptr] != ',') return 0xFF;
  //gps_tmp_fix_indicator = tmp;
  return 0;
}

//---------------------------------------------------------------------------//
// Tryes to extract number of tracked satelites from NMEA message
unsigned char read_number_of_satelites(char * GPS_Rx_Buff) 
{
  unsigned char tmp;

  GPS_Rd_Ptr++;
  tmp = get_2ascii(&GPS_Rx_Buff[GPS_Rd_Ptr]);
  if (tmp > 99) {
    tmp = get_ascii(&GPS_Rx_Buff[GPS_Rd_Ptr]);
    if (tmp > 9) return 0xFF;
    GPS_Rd_Ptr--;
  }
  GPS_Rd_Ptr += 2;
  if (GPS_Rx_Buff[GPS_Rd_Ptr] != ',') return 0xFF;
  if (tmp > 9) tmp = 9;
  gps_tmp_number_of_satelites = tmp;
  return 0;
}
//---------------------------------------------------------------------------//
// Tryes to extract HDOP from NMEA message
// The result stored in "gps_hdop" is actual HDOP in [meter] mult by 10
unsigned char read_hdop(char * GPS_Rx_Buff) 
{
  GPS_Rd_Ptr++;
  gps_hdop = (int)(read_float_as_long(1, &GPS_Rx_Buff[GPS_Rd_Ptr]));
  
  return goto_next_comma(GPS_Rx_Buff);
}
//---------------------------------------------------------------------------//
// Reads validity mark from NMEA message, it have to be 'A' = active,
//   or 'V' = void, the result is stored in 'gps_tmp_valide_mark'
// This function returns 0 if mark is valide and !0 if mark is not found
unsigned char read_valide_mark(char * GPS_Rx_Buff) {

   GPS_Rd_Ptr += 2;
   gps_tmp_valide_mark = 0;
   if (GPS_Rx_Buff[GPS_Rd_Ptr-2] != ',') return 0xFF;
   if (GPS_Rx_Buff[GPS_Rd_Ptr-1] == 'V') {
      return 0;
   }
   if (GPS_Rx_Buff[GPS_Rd_Ptr-1] == 'A') {
      gps_tmp_valide_mark = 1;
      return 0;
   }
   return 0xFF;
}
//---------------------------------------------------------------------------//
// Read transportation velocity reported by GPS.
// The result is stored in 'gps_tmp_velocity' in [km/h * 10], e.g 30 means 3 km/h
// This function is intended to return !0 if any error, but still always returns 0
unsigned char read_velocity(char * GPS_Rx_Buff) 
{
  GPS_Rd_Ptr++;
  gps_tmp_velocity = read_float_as_long(1, &GPS_Rx_Buff[GPS_Rd_Ptr]);
  goto_next_comma(GPS_Rx_Buff);
                                                  // 1 knot = 1.852 km/h
  last_gps_location.vel = (gps_tmp_velocity * 13 + 3) / 7;   // knots  to  km/h*10
  return 0;
}

//---------------------------------------------------------------------------//
// Read transportation direction reported by GPS.
// Returns 0 if data is OK, !0 on error
unsigned char read_direction(char * GPS_Rx_Buff) 
{
  GPS_Rd_Ptr++;
  gps_tmp_direction = read_float_as_long(1, &GPS_Rx_Buff[GPS_Rd_Ptr]) / 20;
  return goto_next_comma(GPS_Rx_Buff);
}
