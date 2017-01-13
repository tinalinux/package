#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include "log.h"
#include "file_ops.h"

const char *g_stdout_file;
const char *g_stderr_file;

/*====================================================================
Function: int create_outfile(const char *filepath)
Description:
    create a file(logfile)
Parameters:
    filename: path of file
Return:
    0: successful
    -1: failed
====================================================================*/
static int create_outfile(const char *filepath)
{
    if (is_existed_and_readable(filepath) != 0) {
        if (errno == ENOENT) {
            touch(filepath);
            if (is_existed_and_readable(filepath) == 0)
                return 0;
        }
        return -1;
    } else
        return 0;
}

/*====================================================================
Function: int Set_Outfile(const char *logfile, const char *errlogfile)
Description:
    To set logfile and errlogfile
    if set, the log will alse save to file
Parameters:
    logfile: save stdout log file
    errlogfile: save stderr log file
Return:
    0: successful
    -1: failed
====================================================================*/
void Set_Outfile(const char *fstdout, const char *fstderr)
{
    if (fstdout != NULL) {
        g_stdout_file = fstdout;
        if (create_outfile(g_stdout_file) < 0) {
            g_stdout_file = NULL;
            fprintf(stderr, "Error: %s(%d) create outfile failed!", __func__, __LINE__);
        }
    }
    if (fstderr != NULL) {
        g_stderr_file = fstderr;
        if (create_outfile(g_stderr_file) < 0) {
            g_stderr_file = NULL;
            fprintf(stderr, "Error: %s(%d) create outfile failed!", __func__, __LINE__);
        }
    }
}

/*====================================================================
Function: void mainlog(logtype type, const char *fmt, ...)
Description:
    the main func to print log
    if point out logfile, will also print to logfile,
    see also Function: Set_Logfile
Parameters:
    type: LOG_INFO;LOG_ERROR;LOG_OUTFILE_ERROR;LOG_OUTFILE_INFO;LOG_OUTFILE
    fmt & ...: the format log contest
Return:
    none
====================================================================*/
void mainlog(logtype type, const char *fmt, ...)
{
    va_list ap;
    FILE *fp = NULL;

    va_start(ap, fmt);
    switch (type) {
        case LOG_OUTFILE_ERROR:
            if (g_stderr_file != NULL) {
                fp = fopen(g_stderr_file, "a");
                if (fp != NULL) {
                    vfprintf(fp, fmt, ap);
                    fclose(fp);
                }
            }
        case LOG_ERROR:
            vfprintf(stderr, fmt, ap);
            fflush(stderr);
            break;
        case LOG_OUTFILE_INFO:
            if (g_stdout_file != NULL) {
                fp = fopen(g_stdout_file, "a");
                if (fp != NULL) {
                    vfprintf(fp, fmt, ap);
                    fclose(fp);
                    fsync(fileno(fp));
                }
            }
        case LOG_INFO:
            vfprintf(stdout, fmt, ap);
            fflush(stdout);
            break;
        case LOG_OUTFILE:
            if (g_stderr_file != NULL) {
                fp = fopen(g_stderr_file, "a");
                if (fp != NULL) {
                    vfprintf(fp, fmt, ap);
                    fclose(fp);
                    fsync(fileno(fp));
                }
            }
            if (g_stdout_file != NULL) {
                fp = fopen(g_stdout_file, "a");
                if (fp != NULL) {
                    vfprintf(fp, fmt, ap);
                    fclose(fp);
                    fsync(fileno(fp));
                }
            }
            break;
    }

    va_end(ap);
}
