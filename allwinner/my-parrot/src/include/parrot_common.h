#ifndef __COMMON_H__
#define __COMMON_H__

#define IO_BUFF_SIZE (20*1000)  //20K once
/*----------------------------------------------------------------------
|   log
+---------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#ifdef __TINA_LOG__
#define plogd(fmt, arg...) TLOGD("[%d %lu] "fmt, getpid(),syscall(SYS_gettid),##arg)
#define ploge(fmt, arg...) TLOGE("[%d %lu] "fmt, getpid(),syscall(SYS_gettid),##arg)
#define plogw(fmt, arg...) TLOGW("[%d %lu] "fmt, getpid(),syscall(SYS_gettid),##arg)
#define plogi(fmt, arg...) TLOGI("[%d %lu] "fmt, getpid(),syscall(SYS_gettid),##arg)
#define plogv(fmt, arg...) TLOGV("[%d %lu] "fmt, getpid(),syscall(SYS_gettid),##arg)
#else
#define plog(fmt, arg...) do { \
    struct timeval tv; \
    struct timezone tz; \
    struct tm *p; \
    gettimeofday(&tv, &tz); \
    p = localtime(&tv.tv_sec); \
    printf("[%d %lu %02d-%02d %02d:%02d:%02d.%06ld] "fmt"\n", getpid(),syscall(SYS_gettid),1+p->tm_mon, p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec,##arg); \
    free(p); \
} while(0)

#define plogd(fmt, arg...) plog(fmt, ##arg)
#define ploge(fmt, arg...) plog(fmt, ##arg)
#define plogw(fmt, arg...) plog(fmt, ##arg)
#define plogi(fmt, arg...) plog(fmt, ##arg)
#define plogv(fmt, arg...) plog(fmt, ##arg)
#endif
/*----------------------------------------------------------------------
|   result codes
+---------------------------------------------------------------------*/
/** Result indicating that the operation or call succeeded */
#define SUCCESS                     0

/** Result indicating an unspecififed failure condition */
#define FAILURE                     (-1)

#include <assert.h>
#define ASSERT(x) assert(x)

/*----------------------------------------------------------------------
|   error code
+---------------------------------------------------------------------*/
#if !defined(ERROR_BASE)
#define ERROR_BASE -20000
#endif

// general errors
#define INVALID_PARAMETERS  (ERROR_BASE - 0)
#define PERMISSION_DENIED   (ERROR_BASE - 1)
#define OUT_OF_MEMORY       (ERROR_BASE - 2)
#define NO_SUCH_NAME        (ERROR_BASE - 3)
#define NO_SUCH_PROPERTY    (ERROR_BASE - 4)
#define NO_SUCH_ITEM        (ERROR_BASE - 5)
#define NO_SUCH_CLASS       (ERROR_BASE - 6)
//#define OVERFLOW            (ERROR_BASE - 7)
#define INTERNAL            (ERROR_BASE - 8)
#define INVALID_STATE       (ERROR_BASE - 9)
#define INVALID_FORMAT      (ERROR_BASE - 10)
#define INVALID_SYNTAX      (ERROR_BASE - 11)
#define NOT_IMPLEMENTED     (ERROR_BASE - 12)
#define NOT_SUPPORTED       (ERROR_BASE - 13)
//#define TIMEOUT             (ERROR_BASE - 14)
#define WOULD_BLOCK         (ERROR_BASE - 15)
#define TERMINATED          (ERROR_BASE - 16)
#define OUT_OF_RANGE        (ERROR_BASE - 17)
#define OUT_OF_RESOURCES    (ERROR_BASE - 18)
#define NOT_ENOUGH_SPACE    (ERROR_BASE - 19)
#define INTERRUPTED         (ERROR_BASE - 20)
#define CANCELLED           (ERROR_BASE - 21)

#define CHECK(_x)                   \
do {                                \
    int _result = (_x);             \
    if (_result != SUCCESS) {       \
        ploge("%s(%d): @@@ CHECK failed, result=%d\n", __FILE__, __LINE__, _result); \
        return _result;             \
    }                               \
} while(0)

#define CHECK_POINTER(_p)                     \
do {                                          \
    if ((_p) == NULL) {                       \
        ploge("%s(%d): @@@ NULL pointer parameter\n", __FILE__, __LINE__); \
        return INVALID_PARAMETERS;            \
    }                                         \
} while(0)

#define DELETE_POINTER(_p)          \
do{                                 \
    if((_p) != NULL){               \
        ploge("delete: %p",_p);     \
        delete _p;                  \
        _p = NULL;                  \
    }                               \
}while(0)

#endif /*__COMMON_H__*/
