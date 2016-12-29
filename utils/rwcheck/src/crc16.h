#ifndef __CRC16_H
#define __CRC16_H

typedef enum {
    FALSE = 0,
    TRUE = 1,
}bool;

//thest two functions will append two bytes crc16 to the end of buf/file
//Make sure buf, extends at least 2 bytes beyond.
//===>    length of buf >= numBytes + 2    <===
void Append_Crc16(char *buf, unsigned long numBytes);
int Append_Crc16_To_File(const char *file, unsigned long off);

unsigned short Get_Crc16(char *buf, unsigned long numBytes);
int Get_Crc16_From_File(unsigned short *crc, const char *file, unsigned long numBytes, unsigned long off);

//ensure the checknum crc16 is saved in the last two bytes of buf/offset address,
//and the numBytes includes two bytes of crc16
bool Check_Crc16(char *buf, unsigned long numBytes);
bool Check_Crc16_From_File(const char *file, unsigned long off);

#endif
