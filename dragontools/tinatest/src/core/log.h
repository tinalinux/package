#ifndef __MLOG_H
#define __MLOG_H

#define BASE 1
#define DATA 2

#ifdef DEBUG_LEVEL
#ifdef COLOUR_LOG
#define DEBUG(level, fmt, arg...) \
do { \
    if (level <= DEBUG_LEVEL) { \
        if (level == BASE) { \
            printf("\033[32m" "[DEBUG]: (%s-%d): " fmt "\033[0m", \
                    __func__, __LINE__, ##arg); \
        } \
        else if (level == DATA) { \
            printf("\033[33m" "[DEBUG]: (%s-%d): " fmt "\033[0m", \
                    __func__, __LINE__, ##arg); \
        } \
    } \
}while(0)
#else
#define DEBUG(level, fmt, arg...) \
do { \
    if (DEBUG_LEVEL == level || DEBUG_LEVEL > DATA) \
        printf("[DEBUG]: (%s-%d): " fmt, __func__, __LINE__, ##arg); \
}while(0)
#endif
#else
#define DEBUG(level, fmt, arg...)
#endif

#ifdef COLOUR_LOG
#define ERROR(fmt, arg...) \
do { \
    fprintf(stderr, "\033[31m" "[ERROR]: (%s-%d): " fmt "\033[0m", \
            __func__, __LINE__, ##arg); \
}while(0)
#else
#define ERROR(fmt, arg...) \
do { \
    fprintf(stderr, "[ERROR]: (%s-%d): " fmt, __func__, __LINE__, ##arg); \
}while(0)
#endif

#endif
