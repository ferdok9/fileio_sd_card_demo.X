
#include "io430.h"
#include "String.H"

#include "Global_FW.H"
#include "IO.H"
#include "Archive.H"
#include "Utilies.H"
#include "Communication.H"
#include "GPS.H"
#include "FAT16.H"
#include "Config.H"
//#include "CMUX.H"

// This file contains procedures necessary to process NMEA 3.xx messages
// from any standard GPS receiver
//
// Programer   : Ventsislav Karadzhov
//
//
// Entry point : ServiceGPSParcer, must be called frequently
// Result      : gps_masks (returned by Parser), last_ok_location, gps_rtc
//               last_ok_location_hdop;

// output data from module
TLocation last_ok_location;
TLocation last_arh_location;
unsigned int last_ok_location_hdop;
unsigned char last_ok_location_satelites;
//unsigned char last_ok_location_dir;

// parser I/O variables
static char          gps_msg_buffer[90];
static unsigned char gps_msg_buffer_ptr;         // = 0 - waiting for '$'
static unsigned char gps_rd_ptr;

// last read message variables
static struct rtc_time gps_rtc;
static TLocation last_gps_location;
static unsigned char coord_error;
//static unsigned char gps_tmp_fix_indicator;
       unsigned char gps_tmp_valide_mark;
       unsigned char gps_tmp_number_of_satelites;
static unsigned int gps_hdop;
unsigned int gps_tmp_velocity;
static unsigned char gps_tmp_direction;

// static functions from this file
static void GPSParseMsg(void);
static unsigned char read_gps_time(void);
static unsigned char read_gps_date(void);
static unsigned char read_latitude(void);
static unsigned char read_longitude(void);
static unsigned char read_fix_indicator(void);
static unsigned char read_number_of_satelites(void);
static unsigned char read_hdop(void);
static unsigned char read_altitude(void);
static unsigned char read_valide_mark(void);
static unsigned char read_velocity(void);
static unsigned char read_direction(void);
static unsigned long read_coord(void);
static long read_float_as_long(unsigned char expo, char *p);
static unsigned char goto_next_comma(void);
static void gps_debug_msg(void);

// static return variables
static unsigned char gps_parser_result;


//===========================================================================//
// Entry point for GPS message parser.                                       //
//===========================================================================//
unsigned char GPS_Parser(void) {
  char                 got_char;
  static unsigned char prev_gps_seconds;
  static unsigned long gps_rd_file_pos = 0;
  unsigned int         fh;

  gps_parser_result = 0;

    if ( conf.gen.masks & GMSK_GPS_FROM_FILE ) 
    {
        // Read GPS data from file
        if ( RTC.seconds != prev_gps_seconds ) 
        {
            fh = open_file("EMULATORGPS") & 0x7FFF;
            if ( fh < 10 ) 
            {
                if (get_file_size(fh) > gps_rd_file_pos) 
                {
                    set_file_pos(fh, gps_rd_file_pos);
                    gps_msg_buffer_ptr = ReadTextLine(fh, gps_msg_buffer);
                    gps_rd_file_pos += (unsigned long)gps_msg_buffer_ptr;
                    gps_msg_buffer[gps_msg_buffer_ptr++] = 0x00;
                    GPSParseMsg();
                    gps_msg_buffer_ptr = ReadTextLine(fh, gps_msg_buffer);
                    gps_rd_file_pos += (unsigned long)gps_msg_buffer_ptr;
                    gps_msg_buffer[gps_msg_buffer_ptr++] = 0x00;
                    GPSParseMsg();
                    set_file_pos(fh, gps_rd_file_pos);
                } 
                else 
                {
                    gps_rd_file_pos = 0;
                }
                close_file(fh);
            } 
            else 
            {
                gps_rd_file_pos = 0;
            }
            prev_gps_seconds = RTC.seconds;
        }
    } 
    else 
    {
        // Read GPS data from GPS module
        while ( ReceiveChar(&got_char, &rsGPS_rx_buff) == 0 ) 
        {
            if (got_char == '$') 
            {
                // a start of message is received
                gps_msg_buffer_ptr = 0;
                gps_msg_buffer[gps_msg_buffer_ptr++] = got_char;
            } 
            else if (got_char == 0x0D) 
            {
                // an end of message is received
                gps_msg_buffer[gps_msg_buffer_ptr++] = 0x0D;
                gps_msg_buffer[gps_msg_buffer_ptr++] = 0x0A;
                gps_msg_buffer[gps_msg_buffer_ptr++] = 0x00;
                GPSParseMsg();
                break;                           // read max one text line at a time
            } 
            else if (gps_msg_buffer_ptr < sizeof(gps_msg_buffer)-4) 
            {
                // a message body char is received
                gps_msg_buffer[gps_msg_buffer_ptr++] = got_char;
            } 
            else 
            {
                // overflow condition
                gps_msg_buffer_ptr = 0;
            }
        }   // while data is available

    }      // if ? GPS source

    // return result
    return gps_parser_result;
}     // ServiceGPSParser()


//===========================================================================//
static void GPSParseMsg(void) 
{
    unsigned char  chksum = 0;
    unsigned char  tmp1, tmp2;

    // calculate and compare message checksum
    gps_rd_ptr = 1;
    while (gps_msg_buffer[gps_rd_ptr] != '*') 
    {
        chksum ^= gps_msg_buffer[gps_rd_ptr];
        if ( (gps_rd_ptr >= gps_msg_buffer_ptr) || (gps_msg_buffer[gps_rd_ptr] < ' ') ) 
        {
            // end of text reached without '*', return
            return;
        }
        gps_rd_ptr++;
    }
    tmp1 = get_ascii_hex(&gps_msg_buffer[gps_rd_ptr+1]);
    if (tmp1 == 0xFF) return;

    tmp2 = get_ascii_hex(&gps_msg_buffer[gps_rd_ptr+2]);
    if (tmp2 == 0xFF) return;    

    if (chksum != (tmp1*16 + tmp2)) return;

    if (gps_msg_buffer[gps_rd_ptr+3] != 0x0D) return;

    // try to identify the message type
    if (memcmp(&gps_msg_buffer[0], "$GP", 3) != 0)
    {
        return;
    }
    gps_rd_ptr = 6;
    if (memcmp(&gps_msg_buffer[3], "GGA", 3) == 0) 
    {
        // $GPGGA message
        if (read_gps_time()) return;
        if (read_latitude()) return;
        if (read_longitude()) return;
        if (read_fix_indicator()) return;
        if (read_number_of_satelites()) return;
        if (read_hdop()) return;
        if (read_altitude()) return;
        last_gps_location.timestamp = TimeStamp;
        gps_parser_result |= GPSM_MESSAGE;
        // DEBUG message
        gps_debug_msg();

    } 
    else if (memcmp(&gps_msg_buffer[3], "RMC", 3) == 0) 
    {
        // $GPRMC message
        if (read_gps_time()) return;
        if (read_valide_mark()) return;
        if (read_latitude()) return;
        if (read_longitude()) return;
        if (read_velocity()) return;
        if (read_direction()) return;
        if (read_gps_date()) return;
        last_gps_location.timestamp = TimeStamp;
        gps_parser_result |= GPSM_MESSAGE;
        // check for valide coordinates
        if (last_gps_location.x && gps_tmp_valide_mark) 
        {
        gps_parser_result |= GPSM_LOCATION;
        last_gps_location.type = 0xF0;
        last_ok_location = last_gps_location;
        last_ok_location_satelites = gps_tmp_number_of_satelites;
        last_ok_location.direction = gps_tmp_direction;
        last_ok_location_hdop = gps_hdop;
        } 
        else 
        {
        gps_parser_result |= GPSM_DISCONTINUE;
        }
        // check if valide date is got from satelite ( or localy generated )
        if (gps_rtc.year >= 0x08)
        {
        gps_parser_result |= GPSM_TIME;
        }
        AdjustRTC( &gps_rtc, gps_tmp_valide_mark);
        // DEBUG message
        gps_debug_msg();

    }
    // the processing of the other message types is skipped
}


//---------------------------------------------------------------------------//
// Try to extract GPS time from NMEA message. Expects gps_rd_ptr to point the
//   comma just before the hours, leaves gps_rd_ptr to point the comma
//   immediately after the seconds.
// In success sets gps time related variables, in error gps_hours = 0xFF;
static unsigned char read_gps_time(void) {
  unsigned int   i;
  unsigned char  tmp;

  // check if gps_rd_ptr points to a comma ( "," )
  if (gps_msg_buffer[gps_rd_ptr] != ',') return 1;
  gps_rd_ptr++;
  gps_rtc.hours = get_2ascii(&gps_msg_buffer[gps_rd_ptr]);
  gps_rtc.minutes = get_2ascii(&gps_msg_buffer[gps_rd_ptr+2]);
  gps_rtc.seconds = get_2ascii(&gps_msg_buffer[gps_rd_ptr+4]);
  if ((gps_rtc.hours > 23 ) || (gps_rtc.minutes > 59) || (gps_rtc.seconds > 59))
    return 1;
  gps_rd_ptr += 6;
  // after the 6 digit need a comma or the fractional part of seconds
  if (gps_msg_buffer[gps_rd_ptr] == ',') return 0;
  // after the 6 digit need a decimal point
  if (gps_msg_buffer[gps_rd_ptr] != '.') return 1;
  // after the dec point need 1 - 5 digits of fractional part of seconds
  for (i=0; i<5; i++) {
    gps_rd_ptr++;
    if (gps_msg_buffer[gps_rd_ptr] == ',') return 0;
    tmp = get_ascii(&gps_msg_buffer[gps_rd_ptr]);
    if (tmp > 9) return 1;
  }
  return 1;
}


//---------------------------------------------------------------------------//
// Tryes to extract GPS time form NMEA message and does elementary checks for
//   data validity
static unsigned char read_gps_date(void) {
  gps_rd_ptr++;
  gps_rtc.mdate = get_2ascii(&gps_msg_buffer[gps_rd_ptr]);
  gps_rtc.month = get_2ascii(&gps_msg_buffer[gps_rd_ptr+2]);
  gps_rtc.year = get_2ascii(&gps_msg_buffer[gps_rd_ptr+4]);
  gps_rd_ptr += 6;
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
unsigned char read_latitude(void) {
  signed long coord;

  coord = read_coord();
  if (coord_error) return 0xFF;
  gps_rd_ptr +=2;
  if (gps_msg_buffer[gps_rd_ptr-1] == 'N') {
    last_gps_location.y = coord;
    return 0;
  }
  if (gps_msg_buffer[gps_rd_ptr-1] == 'S') {
    last_gps_location.y = -coord;
    return 0;
  }
  if ((gps_msg_buffer[gps_rd_ptr-1] == ',') && (coord == 0))  {
    gps_rd_ptr--;
    last_gps_location.y = 0;
    return 0;
  }
  return 0xFF;
}


//---------------------------------------------------------------------------//
// Tryes to extract location longitude from NMEA message
unsigned char read_longitude(void) {
  signed long coord;

  coord = read_coord();
  if (coord_error) return 0xFF;
  gps_rd_ptr +=2;
  if (gps_msg_buffer[gps_rd_ptr-1] == 'E') {
    last_gps_location.x = coord;
    return 0;
  }
  if (gps_msg_buffer[gps_rd_ptr-1] == 'W') {
    last_gps_location.x = -coord;
    return 0;
  }
  if ((gps_msg_buffer[gps_rd_ptr-1] == ',') && (coord == 0))  {
    gps_rd_ptr--;
    last_gps_location.x = 0;
    return 0;
  }
  return 0xFF;
}


//---------------------------------------------------------------------------//
// Tryes to extract location fix indicator from NMEA message
unsigned char read_fix_indicator(void) {
  unsigned char tmp;

  gps_rd_ptr++;
  tmp = get_ascii(&gps_msg_buffer[gps_rd_ptr]);
  if (tmp > 9) return 0xFF;
  gps_rd_ptr++;
  if (gps_msg_buffer[gps_rd_ptr] != ',') return 0xFF;
  //gps_tmp_fix_indicator = tmp;
  return 0;
}


//---------------------------------------------------------------------------//
// Tryes to extract number of tracked satelites from NMEA message
unsigned char read_number_of_satelites(void) {
  unsigned char tmp;

  gps_rd_ptr++;
  tmp = get_2ascii(&gps_msg_buffer[gps_rd_ptr]);
  if (tmp > 99) {
    tmp = get_ascii(&gps_msg_buffer[gps_rd_ptr]);
    if (tmp > 9) return 0xFF;
    gps_rd_ptr--;
  }
  gps_rd_ptr += 2;
  if (gps_msg_buffer[gps_rd_ptr] != ',') return 0xFF;
  if (tmp > 9) tmp = 9;
  gps_tmp_number_of_satelites = tmp;
  return 0;
}


//---------------------------------------------------------------------------//
// Tryes to extract HDOP from NMEA message
// The result stored in "gps_hdop" is actual HDOP in [meter] mult by 10
unsigned char read_hdop(void) {
  gps_rd_ptr++;
  gps_hdop = (int)(read_float_as_long(1, &gps_msg_buffer[gps_rd_ptr]));
  return goto_next_comma();
}

//---------------------------------------------------------------------------//
// Tryes to extract location altitude from NMEA message
// The result stored in last_gps_location.z is actual altitude in [meters] mult by 10
unsigned char read_altitude(void) {
  gps_rd_ptr++;
  last_gps_location.z = read_float_as_long(1, &gps_msg_buffer[gps_rd_ptr]) + 1000;
  return goto_next_comma();
}

//---------------------------------------------------------------------------//
// Reads validity mark from NMEA message, it have to be 'A' = active,
//   or 'V' = void, the result is stored in 'gps_tmp_valide_mark'
// This function returns 0 if mark is valide and !0 if mark is not found
unsigned char read_valide_mark(void) {

   gps_rd_ptr += 2;
   gps_tmp_valide_mark = 0;
   if (gps_msg_buffer[gps_rd_ptr-2] != ',') return 0xFF;
   if (gps_msg_buffer[gps_rd_ptr-1] == 'V') {
      return 0;
   }
   if (gps_msg_buffer[gps_rd_ptr-1] == 'A') {
      gps_tmp_valide_mark = 1;
      return 0;
   }
   return 0xFF;
}

//---------------------------------------------------------------------------//
// Read transportation velocity reported by GPS.
// The result is stored in 'gps_tmp_velocity' in [km/h * 10], e.g 30 means 3 km/h
// This function is intended to return !0 if any error, but still always returns 0
unsigned char read_velocity(void) {
  gps_rd_ptr++;
  gps_tmp_velocity = read_float_as_long(1, &gps_msg_buffer[gps_rd_ptr]);
  goto_next_comma();
                                                  // 1 knot = 1.852 km/h
  last_gps_location.vel = (gps_tmp_velocity * 13 + 3) / 7;   // knots  to  km/h*10
  return 0;
}

//---------------------------------------------------------------------------//
// Read transportation direction reported by GPS.
// Returns 0 if data is OK, !0 on error
unsigned char read_direction(void) {
  gps_rd_ptr++;
  gps_tmp_direction = read_float_as_long(1, &gps_msg_buffer[gps_rd_ptr]) / 20;
  return goto_next_comma();
}


//---------------------------------------------------------------------------//
// Reads from 'gps_msg_buffer' a number with max 4 digits fraction part and
//   makes a conversion. Conversion is necessary because input format is
//   degrees.minutes dispite of integer.fraction part of a composite number.
// Output format is a unsigned integer, containing minutes * 1E4.
// It is expected that 'gps_rd_prt' point to the comma just before the number.
// Returns 0 if any error, otherwise returns the result.
//   hmm, is it the result not a zero ...
unsigned long read_coord(void) {
  unsigned char tmp;
  int           i;
  unsigned long result = 0;

  coord_error = 1;
  // check if 'gps_rd_ptr' points to ','
  if (gps_msg_buffer[gps_rd_ptr++] != ',') {
     coord_error = 1;
     return 0;
  }

  // empty string, e.g. GPS receiver has no idea where we are ...
  if (gps_msg_buffer[gps_rd_ptr] == ',') {
    coord_error = 0;
    return 0;
  }

  // first pass : look for a decimal point
  //   somewhere it MUST end with a decimal point
  for (i=0; i<6; i++) {
      tmp = get_ascii(&gps_msg_buffer[gps_rd_ptr++]);
    if (tmp == ('.' ^ '0')) goto get_coord_fl;             // goto, you like it?
    if (tmp > 9) {
      return 0;  // with 'error'
    }
    result = ( result * 10 ) + (unsigned long)( tmp );
  }
  return 0;  // with 'error'

  // result correction 100 / 60, FIXME need to make it w/o division
 get_coord_fl:
  result = (result / 100) * 60 + result % 100;

  // add the fraction part, MUST end with ','
  for (i=0; i<4; i++) {
    tmp = get_ascii(&gps_msg_buffer[gps_rd_ptr]);
    if (tmp < 10)
      result = (result*10) + (unsigned long)(tmp);
    if ((tmp == (',' ^ '0')) || (tmp == ('*' ^ '0')))
      break;
    gps_rd_ptr++;
  }

  while ( i < 4 ) {
    result *= 10;
    i++;
  }
  goto_next_comma();
  coord_error = 0;
  return result;
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
unsigned char goto_next_comma(void) {
  while ((gps_msg_buffer[gps_rd_ptr] != ',') &&
    (gps_rd_ptr < sizeof(gps_msg_buffer)-4)) {
    gps_rd_ptr++;
  }
  return 0;
}

//---------------------------------------------------------------------------//
void gps_debug_msg(void) {
  unsigned short fh;

  if (conf.gen.masks & GMSK_GPS_TO_RS232) {
    PutRS232Data( (char*)CRLF, 2 );
    PutRS232Data( gps_msg_buffer, gps_msg_buffer_ptr-1 );
  }

#ifdef MAKE_RSUSB
  if (conf.gen.masks & GMSK_GPS_TO_USB) {
    PutUSBData( (char*)CRLF, 2 );
    PutUSBData( gps_msg_buffer, gps_msg_buffer_ptr-1 );
  }
#endif

  if (conf.gen.masks & GMSK_GPS_TO_FILE) {
    fh = open_append_filename("GPS");
    write_file(fh, gps_msg_buffer_ptr-1, gps_msg_buffer);
    close_file(fh);
  }

}


