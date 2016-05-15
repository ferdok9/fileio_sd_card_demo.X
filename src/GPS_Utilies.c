
//#ifndef MAKE_PC_SOFT
// #include "io430.h"
// #include "..\FW\Fat16.H"
// //#include "NTFS.H"
//#endif
#include <string.h>
#include "GPS_Utilies.h"


//===========================================================================//
// ----------------------- Data Conversion routines ------------------------ //
//===========================================================================//
unsigned char get_ascii_hex(char* p) {
   unsigned char tmp;

   tmp = *p;
   if ((tmp >= '0') && (tmp <= '9')) return tmp - '0';
   if ((tmp >= 'A') && (tmp <= 'F')) return tmp - 'A' + 10;
   return 0xFF;
}

//---------------------------------------------------------------------------//
unsigned char get_ascii(char* p) {

   return ((*p) ^ '0');
}

//---------------------------------------------------------------------------//
unsigned char hex_to_ascii(unsigned char ch) {

   if (ch < 10) return '0' + ch;
   if (ch < 16) return 'A' - 10 + ch;
   return '?';
}

//---------------------------------------------------------------------------//
unsigned char get_2ascii(char* p) {
  unsigned char tmp, tmpl;

  tmp = get_ascii(p);
  if (tmp > 9) return 0xFF;
  p++;
  tmpl = get_ascii(p);
  if (tmpl > 9) return 0xFF;
  return tmpl + tmp*10;
}

//---------------------------------------------------------------------------//
unsigned int get_3ascii(char* p) {
  unsigned int tmp, tmp1;

  tmp = get_2ascii(p);
  if (tmp == 0xFF) return 0xFFFF;
  p += 2;
  tmp1 = get_ascii(p);
  if (tmp1 > 9) return 0xFFFF;
  return tmp1 + tmp*10;
}

//---------------------------------------------------------------------------//
unsigned int get_2ascii_byte(char* p) {
  unsigned char tmp, tmpl;

  tmp = get_ascii_hex(p);
  if (tmp > 0x0F) return 0xFFFF;
  p++;
  tmpl = get_ascii_hex(p);
  if (tmpl > 0x0F) return 0xFFFF;
  return tmpl + tmp*16;
}

//---------------------------------------------------------------------------//
void byte_to_2dec_ascii(char* buff, unsigned char byte) {
  *buff = '0' | (byte / 10);
  buff++;
  *buff = '0' | (byte % 10);
}

#ifdef CheckSume
//---------------------------------------------------------------------------//
#ifdef __cplusplus
   extern "C"
#endif
void dump(unsigned char* in_buff, char* out_buff, unsigned int len) {
  while(len--) {
    *(out_buff + len + len + 1) = hex_to_ascii(*(in_buff + len) & 0x0F);
    *(out_buff + len + len) = hex_to_ascii(*(in_buff + len) >> 4);
  }
}
// compres HEX data. Returns 0 in success, !0 if any error
unsigned char undump(char* in_buff, char* out_buff, unsigned short len) {
   unsigned short b;
   while(len--) {
      b = get_2ascii_byte(in_buff);
      if (b > 0xFF)
         return 0xFF;
      *out_buff++ = b;
      in_buff += 2;
   }
   return 0;
}

//---------------------------------------------------------------------------//
char strchkz(const char* p, int len) {
  while (len--) {
    if (*p) return 1;
    p++;
  }
  return 0;
}

//---------------------------------------------------------------------------//
unsigned int  string_zterminate(unsigned int max_len, char* p) {
  unsigned int i = max_len;
  p += max_len - 1;

  while ((i) && (*p <= 0x20)) {
    *p = 0;
    p--;
    i--;
  }
  return max_len - i;
}

//---------------------------------------------------------------------------//
// Read unsigned int 0 .. 65535 from text buffer until non-digit character was
//   found
unsigned int read_uint(char const *p) {
   unsigned int tmp, ch;
   char         i;

   tmp = 0;
   for (i=0; i<5; i++ ) {
      ch = get_ascii((char*)p); p++;
      if (ch < 10) {
         tmp = tmp*10 + ch;
      } else {
         return tmp;
      }
   }
   return tmp;
}


//===========================================================================//
//           Calculate an XOR type checksum over a memory area               //
//===========================================================================//
unsigned char CalcChkSum(unsigned char* str, unsigned int len) {
  unsigned char chk = 0;

  while (len--)
    chk ^= *(str++);
  return chk;
}
unsigned char CalcChkSum55(unsigned char* str, unsigned int len) {
  return (0x55 ^ CalcChkSum( str, len ) );
}

//===========================================================================//
static const unsigned char CRC_TABLE[256] = { //reversed, 8-bit, poly=0x07
0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75, 0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69, 0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D, 0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51, 0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,
0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05, 0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19, 0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D, 0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21, 0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,
0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95, 0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89, 0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD, 0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1, 0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,
0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5, 0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9, 0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD, 0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1, 0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};


unsigned char CalcFCS( const unsigned char *buf, int len, unsigned char iv)
{
  unsigned char FCS = iv;              // default value for iv is 0xFF

  while (len--) {
    FCS=CRC_TABLE[FCS ^ *buf++];
  }
  return (0xFF-FCS);
}


void AddFCS( unsigned char* fcs, unsigned char ch ) {
  *fcs = CRC_TABLE[(*fcs) ^ ch];
}


int CheckFCS( unsigned char *buf, int len) {
  unsigned char FCS=0xFF;

  while (len--) {
    FCS = CRC_TABLE[FCS^*buf++];
  }
  /*0xCF is the reversed order of 11110011.*/
  return (FCS==0xCF);
}
#endif

#ifdef MAKE_PC_SOFT

//===========================================================================//
// Text utilies                                                              //
//===========================================================================//

// Read a line from text file
// Input  : fhandle - input file handle. File must be open and file pointer must
//          point to the start of the line
//          buff - output buffer, min 128 bytes
// Return : 0 if reading failed or text line size if read OK
//          file pointer point to first byte of the next line
//   Note : This function looks for CR+LF sequence.
//          CR+LF are included in size returned by this function.
unsigned char ReadTextLine(unsigned int fhandle, char* buff) {
  unsigned char len = 0;
  char ch;

  while (( read_file(fhandle, 1, &ch) == 0) && (len <127)) {
    // is it EOL
    *(buff+len) = ch;
    len++;
    if (ch == 0x0A) {
      if ( (len > 1) && (*(buff+len-2) == 0x0D) )
        return len;
      else
        return 0;
    }
  }
  return 0;
}


unsigned char WriteTextLine(unsigned int fhandle, char* buff) {
  unsigned int len = strlen((char const*)buff);

  write_file( fhandle, len, (char*)buff );
  return write_file( fhandle, 2, (char*)CRLF );
}



void PrintDateTime(struct rtc_time* real_time) {
  PrintDate(real_time);
  PrintTime(real_time);
}
void PrintDate(struct rtc_time* real_time) {
  byte_to_2dec_ascii(&gp_buff[gp_buff_ptr], real_time->mdate);
  gp_buff_ptr += 2;
  gp_buff[gp_buff_ptr++] = '-';
  byte_to_2dec_ascii(&gp_buff[gp_buff_ptr], real_time->month);
  gp_buff_ptr += 2;
  gp_buff[gp_buff_ptr++] = '-';
  byte_to_2dec_ascii(&gp_buff[gp_buff_ptr], real_time->year);
  gp_buff_ptr += 2;
  gp_buff[gp_buff_ptr++] = ' ';
}
void PrintTime(struct rtc_time* real_time) {
  byte_to_2dec_ascii(&gp_buff[gp_buff_ptr], real_time->hours);
  gp_buff_ptr += 2;
  gp_buff[gp_buff_ptr++] = ':';
  byte_to_2dec_ascii(&gp_buff[gp_buff_ptr], real_time->minutes);
  gp_buff_ptr += 2;
  gp_buff[gp_buff_ptr++] = ':';
  byte_to_2dec_ascii(&gp_buff[gp_buff_ptr], real_time->seconds);
  gp_buff_ptr += 2;
  gp_buff[gp_buff_ptr++] = ' ';
}
#endif



#ifdef Print_Util
//---------------------------------------------------------------------------//
// Извежда в текст цяло число със знак в буфер. Приема като параметри :
//   - числото
//   - брой знаци на буфера
//   - наличие на водещи нули
// Връща 0 при успех, 1 при къс буфер или друга грешка
char print_long(long value, char chars, char leadzero, char *buffer) 
{
   char *p;
   char negative;
   char tmp;
   char sign_pos;
   int  i;

   p = buffer;
   p += chars - 1;
   // leadzero = 0 > празните полета пред числото ще са ' '
   // leadzero = 1 > празните полета отпред ще се '0'

   // нормализиране на числото - да е позитивно
   if (value < 0) 
   {
      value = -value;
      negative = 1;
   } 
   else 
   {
      negative = 0;
   }

   // писане на числото - отзад напред
   for (i=chars; i>0; i--) 
   {
       tmp = value % 10;
       value = value / 10;
       if (tmp) sign_pos = i-2;
       *p = tmp + '0';
       p--;
   }

   // маха водещите нули, ако е указано
   if (leadzero == 0) 
   {
      p = buffer;
      for (i=0; i<chars-1; i++) 
      {
         tmp = *p;
         if (tmp != '0') goto print_long_xlz;
         tmp = ' ';
         *p = tmp;
         p++;
      }
   }
  print_long_xlz:

   // поставя знака на числото ( ако го има )
   if ((negative) && (sign_pos < 0x10)) 
   {
      if (leadzero) p = buffer;
      else          p = buffer + sign_pos;
      *p = '-';
   }
   return 0;
}

//---------------------------------------------------------------------------//
// Извежда число от 0 до 99 в два знака, с водеща нула
void print_00_99(unsigned char value, char *buffer) 
{
   char *p;

   p = buffer;
   if (value > 99) 
   {
      *p = '?';
      p++;
      *p = '?';
   } 
   else 
   {
      *p = '0' + value / 10;
      p++;
      *p = '0' + value % 10;
   }
}

//---------------------------------------------------------------------------//
// Извежда число от 0 до 255 в три знака, с водещи нули и интервал отзад
void print_000_255(unsigned char value, char *buffer)
{
   *buffer = '0' + value / 100;
   buffer++;
   value = value % 100;
   *buffer = '0' + value / 10;
   buffer++;
   *buffer = '0' + value % 10;
   buffer++;
   *buffer = ' ';
}

//---------------------------------------------------------------------------//
// Извежда число от 0 до 255 в един до три знака, връща броя на знаците
unsigned char print_0_255(unsigned char value, char *buffer)
{
   char number = 0;

   if (value > 99)
   {
      *buffer = '0' + value / 100;
      buffer++;
      value = value % 100;
      number++;
   }
   if ((value > 9) || (number))
   {
      *buffer = '0' + value / 10;
      buffer++;
      value = value % 10;
      number++;
   }
   *buffer = '0' + value;
   number++;
   return number;
}
#endif //Print_Util
// --- end ---
