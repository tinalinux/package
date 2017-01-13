#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#include "log.h"
#include "crc16.h"
#include "rwcheck.h"
#include "file_ops.h"

enum isnew {
    oldline = 0,
    newline,
};

enum check_types {
    check_whole = 0,
    check_head,
    check_middle,
    check_tail,
};

enum check_status_num {
    status_num_checking= 0,
    status_num_error,
    status_num_y,
    status_num_n,
    status_max,
};

char *check_status_str[status_max];

struct obj {
    unsigned long off;
    unsigned long random_data_numbytes;
    enum check_status_num status;
    pthread_t func;
};

struct docheck *check;

void refresh_status(struct obj *obj, enum check_status_num status);

/*==================================================
Function: void help(void)
Description:
    this func is to print help
Parameters:
    none
Retrun:
    none
==================================================*/
void help(void)
{
    log_info("  Usage: rwcheck [-a] [-o out_filepath] [-d check_dir] [-t check_times]\n");
    log_info("               [-s check_filesize] [-b auto_check_begin_filesize]\n");
    log_info("               [-e auto_check_end_filesize]\n");
    log_info("\n");
    log_info("\t-l : loop mode, if not specified, set follow attribute:\n");
    log_info("\t          check begin file size  =  2K\n");
    log_info("\t          check end file size  =  512M\n");
    log_info("\t          output file = /mnt/USISK/rwcheck.result\n");
    log_info("\t          check diretory = /mnt/UDISK/\n");
    log_info("\t     In this mode, rwcheck will be call when boot, create and check forever until check failed\n");
    log_info("\t     No any logs will be print in screan but to output file\n");
    log_info("\t     No use to set option: -t and -a\n");
    log_info("\t-a : auto mode, if not specified, set follow attribute:\n");
    log_info("\t          check times  = 1\n");
    log_info("\t          check begin file size  =  2K\n");
    log_info("\t          check end file size  =  512M\n");
    log_info("\t-o # : output file\n");
    log_info("\t-d # : the diretory to check, we will create file here\n");
    log_info("\t       if not specified, we will set to currect diretory\n");
    log_info("\t-t # : check times\n");
    log_info("\t-b # : set begin file size for auto check mode ... KB\n");
    log_info("\t       or -b #k .. size in KB\n");
    log_info("\t       or -b #m .. size in MB\n");
    log_info("\t       or -b #g .. size in GB\n");
    log_info("\t-e # : set end file size for auto check mode ... KB\n");
    log_info("\t       or -e #k .. size in KB\n");
    log_info("\t       or -e #m .. size in MB\n");
    log_info("\t       or -e #g .. size in GB\n");
    log_info("\t-s # : set checking file size ... KB\n");
    log_info("\t       or -s #k .. size in KB\n");
    log_info("\t       or -s #m .. size in MB\n");
    log_info("\t       or -s #g .. size in GB\n");
}

/*==================================================
Function: void print_head(void)
Description:
    print the head of output log
    including checking diractory, check times and file size range
Parameters:
    none
Retrun:
    none
==================================================*/
void print_head(void)
{
    time_t now;
    time(&now);

    log_finfo("\trwcheck: to check read and write data\n\n");
    log_finfo("\t%s", ctime(&now));
    log_finfo("\tset checkfile as %s\n", check->check_file_path);
    if (check->mode != loop_mode)
        log_finfo("\tset check times to %u\n", check->check_times);
    log_finfo("\tset begin filesize to %lu%s\n",
            check->begin_file_size.size, check->begin_file_size.units);
    log_finfo("\tset end filesize to %lu%s\n",
            check->end_file_size.size, check->end_file_size.units);
    log_finfo("\tset filesize up rate to %d\n", (int )FILE_SIZE_UP_RATE);
    if (check->output_file != NULL)
        log_finfo("\tset output file as %s\n", check->output_file);
    if (check->mode == loop_mode) {
        log_finfo("\tset boot script as %s\n", boot_script);
        log_finfo("\tlink boot script as %s\n", boot_script_link);
    }
}

/*==================================================
Function: void *check_data(void *mode)
Description:
    do check in head/middle/tail of file
Parameters:
    mode: 0 -> check whole
          1 -> check head of file
          2 -> check middle of file
          3 -> check tail of file
Retrun:
    none
==================================================*/
void *check_data(void *mode)
{
    struct obj *obj= NULL;

    if (mode == (void *)check_head)   //check head of file
        obj = check->checking->head;
    else if (mode == (void *)check_middle) //check middle of file
        obj = check->checking->middle;
    else if (mode == (void *)check_tail) //check tail of file
        obj = check->checking->tail;
    else if (mode == (void *)check_whole) { //check tail of file
        obj = check->checking->whole;
    }
    else
        goto out;

    //record datalen to head of data area
    if (Set_DataLength_To_File_Bytes(check->check_file_path, obj->off, obj->random_data_numbytes + 4)
            != obj->random_data_numbytes + 4)
        goto out1;

    //append crc16 to the end of effective data
    if (Append_Crc16_To_File(check->check_file_path, obj->off) != 0)
        goto out1;

    //check crc16 and record result in status
    if (Check_Crc16_From_File(check->check_file_path, obj->off) == TRUE)
        refresh_status(obj, status_num_y);
    else
        refresh_status(obj, status_num_n);

    goto out;

out1:
    //change status to ERROR, meaning write crc failed
    refresh_status(obj, status_num_error);
out:
    pthread_exit(NULL);
}

/*==================================================
Function: void print_status(enum isnew isnew)
Description:
    print the status
Parameters:
    isnew: is a new filesize test?
Retrun:
    none
==================================================*/
void print_status(enum isnew isnew)
{
    struct once *now = check->checking;

    if (isnew == newline) {
        log_finfo("\r\t%6lu%2s",
            now->filesize.size, now->filesize.units);
        log_finfo("%12s", check_status_str[now->head->status]);
        log_finfo("%14s", check_status_str[now->middle->status]);
        log_finfo("%12s", check_status_str[now->tail->status]);
        log_finfo("%13s\n", check_status_str[now->whole->status]);
    }
    else {
        log_info("\r\t%6lu%2s",
            now->filesize.size, now->filesize.units);
        log_info("%12s", check_status_str[now->head->status]);
        log_info("%14s", check_status_str[now->middle->status]);
        log_info("%12s", check_status_str[now->tail->status]);
        log_info("%13s", check_status_str[now->whole->status]);
    }
}

/*==================================================
Function: void set_filesize(struct size_unit *unit, unsigned long size);
Description:
    set struct size_unit in size
    if filesize can not devide by 1024, drop it
Parameters:
    unit: the struction of size_unit
    size:  file size
Retrun:
    none
==================================================*/
void set_filesize(struct size_unit *unit, unsigned long size)
{
    unit->filesize_kb = size;
    size = size / 1024;
    if (size != 0) {
        if (size / 1024 != 0) {
            unit->size = size / 1024;
            sprintf(unit->units, "GB");
        }
        else {
            unit->size = size;
            sprintf(unit->units, "MB");
        }
    } else {
        unit->size = unit->filesize_kb;
        sprintf(unit->units, "KB");
    }
}

/*==================================================
Function: void refresh_status(struct obj *obj, enum check_status_num status)
Description:
    refresh object status
Parameters:
    obj: the object to refresh
    status: the status string
Retrun:
    none
==================================================*/
void refresh_status(struct obj *obj, enum check_status_num status)
{
    pthread_mutex_lock(&check->checking->lock_refresh_status);
    obj->status = status;
    print_status(oldline); //everytimes change status, refresh display
    pthread_mutex_unlock(&check->checking->lock_refresh_status);
}

/*==================================================
Function: int init_filesize(struct once *now, unsigned long filesize);
Description:
    initialize struct once and struct obj
    call it before do a new check(or after once and before do next)
    the file devide as:
    head--------------------------------------------end
       | 4B | 10B | head | middle | tail | 10B | 2B |
    note:
        4B : record file size (with 2B of crc16)
        10B : the interval
        2B : the crc16
        head|middle|tail : 3 modes checking space, it is devide as:
            head--------------------------------------------------------end
               | 4B | 2/3Bytes of data area | 2B | other space (no use) |
            4B: record the length of effective data (exclude opther space no use)
            notice: length of data area should not larger than 4M

    so, the length of head|middle|tail is :
        datalen = ((filesize - 26)/3-6)*2/3
Parameters:
    now: struction of once, now means do checking once now
    filesize: file size(KB)
Retrun:
    0: all done
    1: something error
==================================================*/
int init_filesize(struct once *now, unsigned long filesize)
{
    int ret = -1;

    //if the first times, just init before set
    if (now->head == NULL) {
        now->head = (struct obj *)malloc(sizeof(struct obj));
        if (now->head == NULL) {
            log_ferror("%s(%d): malloc memory failed!\n", __func__, __LINE__);
            ret = -1;
            goto out1;
        }
    }
    if (now->middle == NULL) {
        now->middle = (struct obj *)malloc(sizeof(struct obj));
        if (now->middle == NULL) {
            log_ferror("%s(%d): malloc memory failed!\n", __func__, __LINE__);
            ret = -1;
            goto out1;
        }
    }
    if (now->tail == NULL) {
        now->tail = (struct obj *)malloc(sizeof(struct obj));
        if (now->tail == NULL) {
            log_ferror("%s(%d): malloc memory failed!\n", __func__, __LINE__);
            ret = -1;
            goto out1;
        }
    }
    if (now->whole == NULL) {
        now->whole = (struct obj *)malloc(sizeof(struct obj));
        if (now->whole == NULL) {
            log_ferror("%s(%d): malloc memory failed!\n", __func__, __LINE__);
            ret = -1;
            goto out1;
        }
    }

    //set filesize
    set_filesize(&now->filesize, filesize);

    //refresh status to prapering
    now->head->status = now->middle->status = now->tail->status
        = now->whole->status = status_num_checking;

    //calculate object's offset address and random_data_len
    {
        //file devide in 3 part, in head, middle, tail
        unsigned long devide_parts_numbytes = (filesize * 1024 - 26 ) / 3;  //Bytes
        unsigned long random_data_numbytes = (devide_parts_numbytes - 6 ) * 2 / 3; //Bytes

        //make sure random data length not larger than CRC_RANDOM_DATA_MAX_SIZE(4M)
        random_data_numbytes = (random_data_numbytes > CRC_RANDOM_DATA_MAX_SIZE_BYTES) ? CRC_RANDOM_DATA_MAX_SIZE_BYTES : random_data_numbytes;
        //set random data numBytes
        now->head->random_data_numbytes = now->middle->random_data_numbytes
            = now->tail->random_data_numbytes = random_data_numbytes;
        now->whole->random_data_numbytes = filesize * 1024 - 6;

        //set offset address
        now->head->off = 14;
        now->middle->off = 14 + devide_parts_numbytes;
        now->tail->off = 14 + devide_parts_numbytes * 2;
        now->whole->off = 0;
    } while(0);

    ret = 0;
    goto out;

out1:
    if (now->head != NULL)
        free(now->head);
    if (now->middle != NULL)
        free(now->middle);
    if (now->tail != NULL)
        free(now->tail);
    if (now->whole != NULL)
        free(now->whole);
out:
    return ret;
}

/*==================================================
Function: int begin_loop(void)
Description:
    the main Function to do check for loop mode
Parameters:
    none
Retrun:
    0: all done
    -1: something error
==================================================*/
int begin_loop(void)
{
    int ret = -1;
    int file_cnt = 0;
    unsigned long filesize = 0;
    char *fpath = (char *)malloc(strlen(check->check_file_path) + 5);
    if (fpath == NULL)
        goto out;
    memset(fpath, 0 , strlen(check->check_file_path) + 5);

    //no exist /etc/init.d/rwcheck
    if (is_existed_and_readable(boot_script) != 0) {
        if ((ret = create_boot_script(check)) < 0) {
            log_ferror("create boot script failed\n");
            goto out;
        }
    }

    log_finfo("\n\n");
    print_head();
    log_finfo("\n\tBEGIN ...\n");

    while(1) {
        file_cnt = 0;

        //check file
        log_finfo("\t--- CHECK ---\n");
        for (filesize = (check->begin_file_size.filesize_kb);
                filesize <= check->end_file_size.filesize_kb;
                filesize *= FILE_SIZE_UP_RATE) {
            sprintf(fpath, "%s.%d", check->check_file_path, file_cnt);
            //0 means existed
            if (is_existed_and_readable(fpath) == 0) {
                if (Check_Crc16_From_File(fpath, 0) == TRUE) {
                    log_finfo("\t%s : check ... OK\n", fpath);
                    file_cnt++;
                    continue;
                }
                else {
                    //if check error and there is next num files, it means check failed
                    log_finfo("\t%s : check ... failed", fpath);
                    sprintf(fpath, "%s.%d", check->check_file_path, ++file_cnt);
                    if (is_existed_and_readable(fpath) == 0) {
                        log_ferror("!! EXIT\n");
                        goto out1;
                    }
                    else {
                        log_finfo(", but the last one, ignored\n");
                        break;
                    }
                }
            }
            break;
        }

        //remove files
        log_finfo("\t--- REMOVE ---\n");
        while (file_cnt > 0) {
            sprintf(fpath, "%s.%d", check->check_file_path, --file_cnt);
            if (remove((const char *)fpath) != 0) {
                log_ferror("\t%s remove failed !! EXIT\n", fpath);
                goto out1;
            }
            log_finfo("\t%s : remove ... OK\n", fpath);
            sync();
        }

        //create file
        log_finfo("\t--- CREATE ---\n");
        for (filesize = check->begin_file_size.filesize_kb;
                filesize <= check->end_file_size.filesize_kb;
                filesize *= FILE_SIZE_UP_RATE) {
            sprintf(fpath, "%s.%d", check->check_file_path, file_cnt);
            if (create_file(fpath, filesize) != 0) {
                log_ferror("\t%s create failed !! EXIT\n", fpath);
                goto out1;
            }
            if (Set_DataLength_To_File_Bytes(fpath, 0, filesize - 2)
                    != filesize - 2) {
                log_ferror("\t%s record data length failed !! EXIT\n", fpath);
                goto out1;
            }
            if (Append_Crc16_To_File(fpath, 0) != 0) {
                log_ferror("\t%s append crc16 failed !! EXIT\n", fpath);
                goto out1;
            }
            log_finfo("\t%s : create ... OK\n", fpath);
            file_cnt++;
        }

        log_finfo("\n");
    }

out1:
    delete_boot_script();
    free(fpath);
out:
    return ret;
}

/*==================================================
Function: int begin(void)
Description:
    the main Function to do check
Parameters:
    none
Retrun:
    0: all done
    1: something error
==================================================*/
int begin(void)
{
    int ret = 0;
    //alloc memory
    struct once *now = (struct once *)malloc(sizeof(struct once));
    if (now == NULL) {
        log_ferror("%s(%d): malloc memory failed!\n", __func__, __LINE__);
        ret = -1;
        goto out;
    }
    memset(now, 0 ,sizeof(struct once));
    //record "now" to "check"
    check->checking = now;

    //initialize "now"
    ret = pthread_mutex_init(&check->checking->lock_refresh_status, NULL);
    if (ret < 0) {
        log_ferror("%s(%d): initialize mutex lock failed!\n", __func__, __LINE__);
        ret = -1;
        goto out1;
    }

    print_head();
    while (check->check_times > 0) {
        //do once check
        log_finfo("\n\tfilesize        head        middle        tail        whole\n");
        ret = init_filesize(now, check->begin_file_size.filesize_kb);
        if (ret != 0)
            goto out2;

        while (now->filesize.filesize_kb <= check->end_file_size.filesize_kb) {
            print_status(oldline);
            ret = create_file(check->check_file_path, now->filesize.filesize_kb);
            if (ret != 0) {
                log_ferror("%s(%d): create %s failed!\n", __func__, __LINE__,
                        check->check_file_path);
                ret = -1;
                goto out2;
            }

            //create pthread to do check
            ret = pthread_create(&now->head->func, NULL,
                    check_data, (void *)check_head);
            if (ret != 0)
                refresh_status(now->head, status_num_error);

            ret = pthread_create(&now->middle->func, NULL,
                    check_data, (void *)check_middle);
            if (ret != 0)
                refresh_status(now->middle, status_num_error);

            ret = pthread_create(&now->tail->func, NULL,
                    check_data, (void *)check_tail);
            if (ret != 0)
                refresh_status(now->tail, status_num_error);

            //wait for finishing check head, middle, tail
            pthread_join(now->head->func, NULL);
            pthread_join(now->middle->func, NULL);
            pthread_join(now->tail->func, NULL);

            //after checking head, middle, tail, to check whole
            ret = pthread_create(&now->whole->func, NULL,
                   check_data , (void *)check_whole);
            if (ret != 0)
                refresh_status(now->whole, status_num_error);

            pthread_join(now->whole->func, NULL);

            //do next filesize
            print_status(newline);
            init_filesize(now, now->filesize.filesize_kb * FILE_SIZE_UP_RATE);
            remove(check->check_file_path);
        }
        check->check_times--;
    }

out2:
    pthread_mutex_destroy(&check->checking->lock_refresh_status);
    if (now->head != NULL) {
        free(now->head);
    }
    if (now->middle != NULL) {
        free(now->middle);
    }
    if (now->tail != NULL) {
        free(now->tail);
    }
    if (now->whole != NULL) {
        free(now->whole);
    }
out1:
    free(now);
out:
    return ret;
}

/*==================================================
Function: int main(int argc, char **argv)
Description:
    to analyse options and call begin()
Parameters:
    ...
Retrun:
    0: all done
    1: something error
==================================================*/
int main(int argc, char **argv)
{
    int opts = 0, ret = 0;
    if (argc == 1) {
        help();
        exit(0);
    }

    check_status_str[status_num_checking] = (char *)"CHECKING";
    check_status_str[status_num_error] = (char *)"ERROR";
    check_status_str[status_num_y] = (char *)"Y";
    check_status_str[status_num_n] = (char *)"N";

    check = (struct docheck *)malloc(sizeof(struct docheck));
    if (check == NULL) {
        log_ferror("%s(%d): malloc memory failed\n", __func__, __LINE__);
        ret = 1;
        goto out;
    }
    memset(check, 0, sizeof(struct docheck));

    while ((opts = getopt(argc, argv, ":hlao:d:t:s:b:e:")) != EOF) {
        switch (opts) {
            case 'a':
                if (check->mode != loop_mode)
                    check->mode = auto_mode;
                break;
            case 'l':
                check->mode = loop_mode;
                break;
            case 'o':
                {
                    check->output_file = optarg;
                    break;
                }
            case 'd':
                {
                    int len = strlen(optarg);
                    check->check_file_path =
                        (char *)malloc(len + (int)sizeof(INITIAL_FILE_NAME) + 5);
                    if (check->check_file_path == NULL) {
                        log_ferror("%s(%d): malloc memory failed!", __func__, __LINE__);
                        goto out;
                    }

                    strncpy(check->check_file_path, optarg, len);

                    //ensure the diractory without '/' at the end
                    if (check->check_file_path[len-1] == '/')
                        check->check_file_path[len-1] = '\0';
                    check->check_file_path[len] = '\0';
                    mkdir_p(check->check_file_path);
                    break;
                }
            case 't':
                check->check_times = (unsigned int)atoi(optarg);
                break;
            case 's':
                {
                    unsigned long size = 0;
                    sscanf(optarg, "%lu", &size);
                    if(optarg[strlen(optarg)-1] == 'm' || optarg[strlen(optarg)-1] == 'M')
                        set_filesize(&check->begin_file_size, size * MB);
                    else if (optarg[strlen(optarg)-1] == 'g' || optarg[strlen(optarg)-1] == 'G')
                        set_filesize(&check->begin_file_size, size * GB);
                    else
                        set_filesize(&check->begin_file_size, size * KB);
                    memcpy(&check->end_file_size, &check->begin_file_size, sizeof(struct size_unit));
                    break;
                }
            case 'b':
                {
                    unsigned long size = 0;
                    sscanf(optarg, "%lu", &size);
                    if(optarg[strlen(optarg)-1] == 'm' || optarg[strlen(optarg)-1] == 'M')
                        set_filesize(&check->begin_file_size, size * MB);
                    else if (optarg[strlen(optarg)-1] == 'g' || optarg[strlen(optarg)-1] == 'G')
                        set_filesize(&check->begin_file_size, size * GB);
                    else
                        set_filesize(&check->begin_file_size, size * KB);
                    break;
                }
            case 'e':
                {
                    unsigned long size = 0;
                    sscanf(optarg, "%lu", &size);
                    if(optarg[strlen(optarg)-1] == 'm' || optarg[strlen(optarg)-1] == 'M')
                        set_filesize(&check->end_file_size, size * MB);
                    else if (optarg[strlen(optarg)-1] == 'g' || optarg[strlen(optarg)-1] == 'G')
                        set_filesize(&check->end_file_size, size * GB);
                    else
                        set_filesize(&check->end_file_size, size * KB);
                    break;
                }
            case '?':
                log_ferror("invalid option\n");
                exit(1);
            case ':':
                log_ferror("option requires an argument\n");
                exit(1);
            case 'h':
                help();
                exit(0);
        }
    }

    //set to check automatically
    if (check->mode == auto_mode) {
        if (check->check_times == 0)
            check->check_times = 1;
        if (check->begin_file_size.filesize_kb == 0)
            set_filesize(&check->begin_file_size, 2 * KB);
        if (check->end_file_size.filesize_kb == 0)
            set_filesize(&check->end_file_size, 512 * MB);
    }

    //set to loop mode
    if (check->mode == loop_mode) {
        if (check->begin_file_size.filesize_kb == 0)
            set_filesize(&check->begin_file_size, 2 * KB);
        if (check->end_file_size.filesize_kb == 0)
            set_filesize(&check->end_file_size, 512 * MB);
        if (check->output_file == NULL)
            check->output_file = (char *)LOOP_MODE_DEFAULT_OUTPUT_FILE;
        if (check->check_file_path == NULL) {
            check->check_file_path =
                (char *)malloc((int)sizeof(LOOP_MODE_DEFAULT_CHECK_PATH) + (int)sizeof(INITIAL_FILE_NAME) + 5);
            if (check->check_file_path == NULL) {
                log_error("%s(%d): malloc memory failed!", __func__, __LINE__);
                goto out;
            }
            strncpy(check->check_file_path, (char *)LOOP_MODE_DEFAULT_CHECK_PATH, (int)sizeof(LOOP_MODE_DEFAULT_CHECK_PATH));
        }
        check->check_times = 0;
    }

    //if not specified, set check diretory to currect diretory
    if (check->check_file_path == NULL) {
        check->check_file_path =
            (char *)malloc(1 + (int)sizeof(INITIAL_FILE_NAME) + 5);
        if (check->check_file_path == NULL) {
            log_error("%s(%d): malloc memory failed!", __func__, __LINE__);
            goto out;
        }
        sprintf(check->check_file_path, ".");
    }
    sprintf(check->check_file_path, "%s/%s", check->check_file_path, INITIAL_FILE_NAME);

    //if specified, set outfile
    if (check->output_file != NULL) {
        Set_Outfile(check->output_file, check->output_file);
    }

    //check if it can start
    if (check->begin_file_size.filesize_kb == 0) {
        log_error("please set begin file size (-a or -b or -s)\n");
        goto out1;
    }
    if (check->end_file_size.filesize_kb == 0) {
        log_error("please set end file size (-a or -e or -s)\n");
        goto out1;
    }
    if (check->end_file_size.filesize_kb < check->begin_file_size.filesize_kb) {
        log_error("end file size %lu can't be less than begin file size %lu\n",
                check->end_file_size.filesize_kb, check->begin_file_size.filesize_kb);
        goto out1;
    }
    if (check->mode != loop_mode && check->check_times == 0) {
        log_ferror("please set check times (-a or -t)\n");
        goto out1;
    }

    //finish analysing options, begin to check accroding mode
    if (check->mode == loop_mode)
        ret = begin_loop();
    else
        ret = begin();

out1:
    free(check->check_file_path);
out:
    free(check);
    return ret;
}
