#ifndef __FILE_OPS_H
#define __FILE_OPS_H

#include "rwcheck.h"

int touch(const char *filepath);
void mkdir_p(char *dir);
int create_file(char *file, unsigned long filesize_kb);
int create_boot_script(struct docheck *check);
void delete_boot_script(void);
unsigned long Get_Filesize(const char *file);
unsigned long Set_DataLength_To_File_Bytes(const char *file,
        unsigned long off, unsigned long nunBytes);
unsigned long Get_DataLength_From_File_Bytes(const char *file, unsigned long off);
int is_existed_and_readable_writeable(const char *file);
int is_existed_and_readable(const char *file);

#endif
