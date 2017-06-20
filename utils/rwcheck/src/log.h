#ifndef __MYLOG_H
#define __MYLOG_H

#include <stdarg.h>
#include <stdio.h>

#define log_error(fmt, ...) \
    do { \
            mainlog(LOG_ERROR, "Error: " fmt,  ## __VA_ARGS__); \
    } while (0)

#define log_info(fmt, ...) \
    do { \
            mainlog(LOG_INFO, fmt,  ## __VA_ARGS__); \
    } while (0)

#define log_file(fmt, ...) \
    do { \
            mainlog(LOG_OUTFILE, fmt,  ## __VA_ARGS__); \
    } while (0)

#define log_finfo(fmt, ...) \
    do { \
            mainlog(LOG_OUTFILE_INFO, fmt,  ## __VA_ARGS__); \
    } while (0)

#define log_ferror(fmt, ...) \
    do { \
            mainlog(LOG_OUTFILE_ERROR, fmt,  ## __VA_ARGS__); \
    } while (0)

typedef enum {
    LOG_INFO = 0,       // just print to stdout
    LOG_ERROR,          // just print to stderr
    LOG_OUTFILE,        // just print to output file(stdout, stderr)
    LOG_OUTFILE_INFO,   // print to stdout and output file
    LOG_OUTFILE_ERROR,  // print to stderr and output file
} logtype;

void Set_Outfile(const char *fstdout, const char *fstderr);

void mainlog(logtype type, const char *fmt, ...);

#endif
