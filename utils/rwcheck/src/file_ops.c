#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "file_ops.h"
#include "rwcheck.h"
#include "log.h"

/*====================================================================
Function: int is_exited_and_readable(char *file)
Description:
    check if the file is existed and readable
Parameters:
    file: path of file
Return:
    0: existed and readable
    else: not existed and readable
====================================================================*/
int is_existed_and_readable(const char *file)
{
    return access(file, F_OK | R_OK);
}

/*====================================================================
Function: int is_exited_and_readable_writeable(char *file)
Description:
    check if the file is existed and readable and writeable
Parameters:
    file: path of file
Return:
    0: existed and writeable
    else: not existed and writeable
====================================================================*/
int is_existed_and_readable_writeable(const char *file)
{
    return access(file, F_OK | W_OK);
}

/*====================================================================
Function: int delete_boot_script(void);
Description:
    delete boot script :/etc/init.d/rwcheck
    unlink : /etc/rc.d/S100rwcheck
Parameters:
    none
Return:
    none
====================================================================*/
void delete_boot_script(void)
{
    remove(boot_script);
    remove(boot_script_link);
}

/*====================================================================
Function: int create_boot_script(struct docheck *check);
Description:
    create boot script :/etc/init.d/rwcheck
    link to : /etc/rc.d/S100rwcheck
Parameters:
    dir: path of dir
Return:
    0: succeffully
    -1: failed
====================================================================*/
int create_boot_script(struct docheck *check)
{
    int ret = -1;
    FILE *fp = NULL;
    char *check_dir = (char *)malloc((int)strlen(check->check_file_path) + 5);
    if (check_dir == NULL) {
        log_ferror("%s(%d): malloc failed\n", __func__, __LINE__);
        goto out;
    }
    strncpy(check_dir, check->check_file_path, strlen(check->check_file_path));

    fp = fopen(boot_script, "w");
    if (fp == NULL) {
        log_ferror("%s(%d): open %s failed\n", __func__, __LINE__, boot_script);
        goto out1;
    }
    fprintf(fp, "#!/bin/sh /etc/rc.common\n");
    fprintf(fp, "\n");
    fprintf(fp, "START=99\n");
    fprintf(fp, "\n");
    fprintf(fp, "start() {\n");
    fprintf(fp, "   rwcheck -l -d %s -o %s -b %lu -e %lu &\n",
            dirname(check_dir),
            check->output_file,
            check->begin_file_size.filesize_kb,
            check->end_file_size.filesize_kb);
    fprintf(fp, "}\n");
    fprintf(fp, "\n");
    fprintf(fp, "stop() {\n");
    fprintf(fp, "   return 0\n");
    fprintf(fp, "}\n");

    fsync(fileno(fp));
    fclose(fp);

    chmod(boot_script,
            S_IRUSR | S_IWUSR | S_IXUSR |
            S_IRGRP | S_IXGRP |
            S_IROTH | S_IXOTH);

    if (is_existed_and_readable(boot_script_link) == 0)
        remove(boot_script_link);
    if (symlink(boot_script, boot_script_link) != 0) {
        log_ferror("%s(%d): link %s to %s failed\n", __func__, __LINE__, boot_script, boot_script_link);
        remove(boot_script);
        goto out1;
    }

    ret = 0;

out1:
    free(check_dir);
out:
    sync();
    return ret;
}

/*====================================================================
Function: void mkdir_p(char *dir)
Description:
    Do as linux command: mkdir -p
    Recursive create dir
Parameters:
    dir: path of dir
Return:
    none
====================================================================*/
void mkdir_p(char *dir)
{
    char *l = strrchr(dir, '/');

    if (l) {
        *l = '\0';
        mkdir_p(dir);
        *l = '/';
        mkdir(dir, 0755);
    }
}

/*====================================================================
Function: touch(const char *filename)
Description:
    Do as linux command: touch
    create a file
Parameters:
    filename: path of file
Return:
    0: successful
    -1: failed
====================================================================*/
int touch(const char *filepath)
{
    FILE *fp;
    char *dir;
    char *buf = (char *)malloc(100);
    int len, ret = -1;

    len = strlen(filepath);
    strncpy(buf, filepath, len > 100 ? 100 : len);
    buf[len] = '\0';
    dir = dirname((char *)buf);

    //ensure dir is access
    mkdir_p(dir);
    //just touch a new file
    fp = fopen(filepath, "w");
    if (fp < 0) {
        log_ferror("%s(%d): create %s failed\n", __func__, __LINE__, filepath);
        goto out;
    }
    fsync(fileno(fp));
    fclose(fp);

    ret = 0;
out:
    free(buf);
    return ret;
}

/*==================================================
Function: int create_file(unsigned long filesize_kb)
Description:
    make file with specified size filled with random data
Parameters:
    filesize: file size(KB)
Retrun:
    0: successfull
    -1: something error
==================================================*/
int create_file(char *file, unsigned long filesize_kb)
{
    //malloc 4MB buf to save random data
    char *buf = (char *)malloc(SIZE_PER_READ_MB * MB * 1024);
    int fd_in, fd_out;
    int ret = -1;

    fd_out = open(file, O_WRONLY | O_SYNC | O_CREAT,
                S_IRUSR | S_IWUSR |
                S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH);
    if (fd_out < 0)
        goto out;
    fd_in = open("/dev/urandom", O_RDONLY);
    if (fd_out < 0)
        goto out1;

    //create file
    while(filesize_kb > 0) {
        //notice that the basic unit is KB
        unsigned once_size_kb = ((SIZE_PER_READ_MB * MB) < (filesize_kb)) ? (SIZE_PER_READ_MB * MB) : (filesize_kb);

        ret = read(fd_in, buf, 1024 * once_size_kb);
        if ((unsigned int)ret != once_size_kb * 1024) {
            log_ferror("read /dev/urandom failed\n");
            goto out2;
        }
        ret = write(fd_out, buf, 1024 * once_size_kb);
        if ((unsigned int)ret != once_size_kb * 1024) {
            log_ferror("write %s failed\n", INITIAL_FILE_NAME);
            goto out2;
        }
        sync();
        filesize_kb -= once_size_kb;
    }

    ret = 0;
out2:
    close(fd_in);
out1:
    close(fd_out);
out:
    free(buf);
    return ret;
}

/*====================================================================
Function:  unsigned long Get_Filesize(const char *file)
Description:
    get filesize
Parameters:
    file: the pathname of file
Return:
    file size (bytes)
====================================================================*/
unsigned long Get_Filesize(const char *file)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat(file, &statbuff) < 0)
        return filesize;
    else
        filesize = statbuff.st_size;
    return filesize;
}

/*====================================================================
Function:  unsigned long Set_DataLength_To_File_Bytes(const char *file, unsigned long off, unsigned long datalen)
Description:
    Set data numbytes from head of file exclude 2bytes of crc
Parameters:
    file: the pathname of file
    off: offset address of file to read data length
Return:
    data size
====================================================================*/
unsigned long Set_DataLength_To_File_Bytes(const char *file,
        unsigned long off, unsigned long nunBytes)
{
    //get filesize
    int fd = 0;
    unsigned long ret = -1;
    unsigned long filesize = Get_Filesize(file);
    if (filesize == -1)
        goto out;

    //make sure off <= filezise
    if (filesize < off)
        goto out;

    //set numBytes to head of file
    fd = open(file, O_RDWR | O_SYNC);
    if (fd < 0 )
        goto out;

    if (lseek(fd, off, SEEK_SET) < 0)
        goto out1;
    if (write(fd, &nunBytes, sizeof(unsigned long)) != sizeof(unsigned long))
        goto out1;

    sync();
    ret = nunBytes;
out1:
    close(fd);
out:
    return ret;
}

/*====================================================================
Function:  unsigned long Get_DataLength_From_File_Bytes(const char *file, unsigned long off)
Description:
    get data numbytes from head of file exclude 2bytes of crc
Parameters:
    file: the pathname of file
    off: offset address of file to read data length
Return:
    data size
====================================================================*/
unsigned long Get_DataLength_From_File_Bytes(const char *file, unsigned long off)
{
    //get filesize
    FILE *fp = NULL;
    unsigned long ret = -1;
    unsigned long filesize = Get_Filesize(file);
    if (filesize == -1)
        goto out;

    //make sure off <= filezise
    if (filesize < off)
        goto out;

    //get numBytes from head of file
    fp = fopen(file, "rb");
    if (fp == NULL)
        goto out;

    fseek(fp, off, SEEK_SET);
    if (fread(&ret, sizeof(char), sizeof(unsigned long), fp) != sizeof(unsigned long)) {
        ret = -1;
        goto out;
    }
    fclose(fp);

out:
    return ret;
}
