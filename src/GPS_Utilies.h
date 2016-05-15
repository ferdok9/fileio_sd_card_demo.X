
#ifndef GPS_UTILIES_H
#define	GPS_UTILIES_H

#define	Print_Util

unsigned char get_ascii_hex(char* p);
unsigned char get_ascii(char* p);
unsigned char hex_to_ascii(unsigned char ch);
unsigned char get_2ascii(char* p);
unsigned int get_3ascii(char* p);
unsigned int get_2ascii_byte(char* p);
void byte_to_2dec_ascii(char* buff, unsigned char byte);

#ifdef Print_Util
extern char print_long(long value, char chars, char leadzero, char *buffer);
extern void print_00_99(unsigned char value, char *buffer);
extern void print_000_255(unsigned char value, char *buffer);
extern unsigned char print_0_255(unsigned char value, char *buffer);
#endif //Print_Util

#endif	/* GPS_UTILIES_H */

