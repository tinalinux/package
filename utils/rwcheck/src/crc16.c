#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "crc16.h"
#include "file_ops.h"
#include "log.h"

#define BytesPerRead (16 * 1024) //16kB

/* Needed for CRC generation/checking */
static const unsigned short crc16_table[] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/*====================================================================
Function:  static unsigned short crc16_common(unsigned short oldcrc, char *buf, long numBytes)
Description:
    this function is extracting common part of crc16 for get/check crc16 from buf/file
Parameters:
    oldcrc: old crc
    buf: the data to be checked
    numBytes: the count bytes of the buf to calculate crc16
Return:
    result
====================================================================*/
static unsigned short crc16_common(unsigned short oldcrc, char *buf, unsigned long numBytes)
{
	unsigned long index = 0;

	for (; index < numBytes; index++) {
		oldcrc = (oldcrc >> 8) ^ crc16_table[(oldcrc ^ buf[index]) & 0xff];
	}
    return oldcrc;
}

/*====================================================================
Function: unsigned short Get_Crc16(char *buf, unsigned long numBytes)
Description:
    get crc16 from buf
    notice: numBytes just the data length to calculate crc16
Parameters:
    crc: save the result
    file: the pathname of file
    numBytes: the count bytes of the buf
    off: the offset of file
Return:
    -1: something error
    0: sucessfully
====================================================================*/
unsigned short Get_Crc16(char *buf, unsigned long numBytes)
{
	unsigned short crc = 0xffff;

    crc = crc16_common(crc, buf, numBytes);
	crc ^= 0xffff;
    return crc;
}

/*====================================================================
Function:  int Get_Crc16_From_File(unsigned short *crc, const char *file, unsigned long numBytes, int off)
Description:
    get crc16 from file
    notice: the length of data is recorded in fisrt 4 bytes on offset address, if not specificted, read from file
Parameters:
    crc: save the result
    file: the pathname of file
    nunBytes: effiective data len of file, if not specified(numBytes==0), read it from file
    off: the offset of file
Return:
    -1: something error
    0: sucessfully
====================================================================*/
int Get_Crc16_From_File(unsigned short *crc, const char *file, unsigned long numBytes, unsigned long off)
{
    int ret = -1;
    unsigned short crc_tmp = 0xffff;
    unsigned long filesize = 0;
    char *pData = NULL;
    FILE *fp = NULL;

    filesize = Get_Filesize(file);
    if (filesize < off)
        goto out;

    if (numBytes == 0) {
        numBytes = Get_DataLength_From_File_Bytes(file, off);
        if (numBytes == -1 || numBytes + 2 > filesize)
            goto out;
    }

    pData = (char *)malloc(sizeof(char) * BytesPerRead);
    if (pData == NULL)
		goto out1;

    fp = fopen(file, "rb");
    if (!fp)
		goto out2;

    //go to offset address, calculate crc16 here
	fseek(fp, off, SEEK_SET);
    while (numBytes > 0) {
        //read data from file
        unsigned long cnt = fread(pData, 1, numBytes < BytesPerRead ? numBytes : BytesPerRead , fp);
        if (cnt != (numBytes < BytesPerRead ? numBytes : BytesPerRead)) {
            log_ferror("%s(%d): read %s failed!\n", __func__, __LINE__, file);
            goto out2;
        }
        //get checksum
        crc_tmp = crc16_common(crc_tmp, pData, cnt);
        numBytes -= cnt;
    }
	crc_tmp ^= 0xffff;
    *crc = crc_tmp;

    ret = 0;

out2:
    fclose(fp);
out1:
    free(pData);
out:
    return ret;
}

/*====================================================================
Function:   void Append_Crc16(char *buf, int numBytes)
Description:
    Appends 16bit CRC at the end of numBytes long buffer.
    It will call Get_Crc16 to get checksum first.
    Make sure buf, extends at least 2 bytes beyond.
Parameters:
    buf: the data to be checked
    numBytes: the count bytes of the buf
Return:
    none
====================================================================*/
void Append_Crc16(char *buf, unsigned long numBytes)
{
    unsigned short crc16 = Get_Crc16(buf, numBytes);

	buf[numBytes++] = crc16;
	buf[numBytes] = crc16 >> 8;
}

/*====================================================================
Function:  int Append_Crc16_To_File(const char *file, unsigned long numBytes)
Description:
    Appends 16bit CRC at the end of numBytes long buffer.
    Make sure buf, extends at least 2 bytes beyond.
Parameters:
    file: the pathname of file
    numBytes: the count bytes of the buf
    off: the offset of file
Return:
    0: successfully
    -1: something error
====================================================================*/
int Append_Crc16_To_File(const char *file, unsigned long off)
{
    int ret = -1;
    unsigned short crc = 0;
    unsigned long filesize = 0;
    unsigned long numBytes = -1;
    int fd = 0;

    filesize = Get_Filesize(file);
    if (filesize < off)
        goto out;

    numBytes = Get_DataLength_From_File_Bytes(file, off);
    if (numBytes == -1 || (off + numBytes + 2) > filesize) {
        goto out;
    }

    ret = Get_Crc16_From_File(&crc, file, numBytes, off);
    if (ret == -1)
        goto out;

    //append crc16 to file
    fd = open(file, O_RDWR | O_SYNC);
    if (fd < 0)
        goto out;
    if (lseek(fd, off + numBytes, SEEK_SET) < 0)
        goto out1;
    if (write(fd, &crc, 2) != 2)
        goto out1;

    sync();
    ret = 0;
out1:
    close(fd);
out:
    return ret;
}

/*====================================================================
Function: bool Check_Crc16(char *buf, unsigned long numBytes);
Description:
    Check crc16 from buf
    At the end of data, it records the checksum of crc16 (2 Bytes)
    so the numBytes is include two bytes of crc16
Parameters:
    filename: the file name
    off: the effective data deviate from the begining of file
Return:
    TRUE: check crc16 OK
    FALSE: check crc16 Failed
====================================================================*/
bool Check_Crc16(char *buf, unsigned long numBytes)
{
	if ((Get_Crc16(buf, numBytes) & 0xffff) != 0xf0b8) {
		return FALSE;
	} else {
        return TRUE;
    }
}

/*====================================================================
Function: bool Check_Crc16_From_File(const char *filename, int off)
Description:
    Check crc16 from file
    At the head of data, it records the length of data exclude 2 bytes of crc16
    At the end of dtat, it records the checksum of crc16 (2 Bytes)
Parameters:
    filename: the file name
    off: the effective data deviate from the begining of file
Return:
    TRUE: check crc16 OK
    FALSE: check crc16 Failed
====================================================================*/
bool Check_Crc16_From_File(const char *file, unsigned long off)
{
    unsigned short crc = 0;
    unsigned long numBytes = 0;
    unsigned long filesize = 0;
    bool ret = FALSE;

    filesize = Get_Filesize(file);
    if (filesize < off)
        goto out;

    numBytes = Get_DataLength_From_File_Bytes(file, off);
    if (numBytes == -1 || numBytes + 2 > filesize)
        goto out;

    //add 2 bytes of crc
    numBytes += 2;

    if (Get_Crc16_From_File(&crc, file, numBytes, off) == 0) {
        if ((unsigned short)(~crc) == 0xf0b8)
            ret = TRUE;
    }
out:
    return ret;
}
