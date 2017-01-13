#ifndef GENERAL_FUNCS_H
#define GENERAL_FUNCS_H

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#include <limits.h>
#include <sys/sem.h>
#endif

#define GEN_TRUE 1
#define GEN_FALSE 0

typedef unsigned char GenBool;

#ifdef WIN32
	#define THREADROUTINE WINAPI
    #define THREADHANDLENULL NULL
    #define GEN_PATH_CHAR '\\'
#else
	#define THREADROUTINE
    #define THREADHANDLENULL 0
    #define MAX_PATH PATH_MAX
    #define GEN_PATH_CHAR '/'
#endif

#ifdef WIN32
	typedef HANDLE THREADHANDLE;
	typedef LPTHREAD_START_ROUTINE PFUNCTHREADROUTINE;
#else
	typedef pthread_t THREADHANDLE;
    typedef void* (*PFUNCTHREADROUTINE)(void*);

   typedef union semun
   {
       int val;
       struct semid_ds *buf;
       unsigned short int *array;
       struct seminfo* __buf;
   }UnSemun;
#endif

typedef enum
{
    EM_SHM_STATE_NONE = 0,
    EM_SHM_STATE_READ,
    EM_SHM_STATE_WRITE,
}EmShareMemeryState;

typedef struct
{
    EmShareMemeryState state;
    char buffer[1020];
}StShared;

GenBool GENFUNCreateThread(THREADHANDLE* pThreadHandle, PFUNCTHREADROUTINE pfnThreadRoutine, void* pParam);
void GENFUNReleaseThread(THREADHANDLE* pThreadHandle);
void GENFUNSleep(unsigned int uiMilliseconds);

// szBuffer的大小必须大于等于MAX_PATH（260B）
GenBool GENFUNGetCurrentPath(char* szBuffer, int iBufferSize);

GenBool GENFUNSemaphoreP(void* pSemaphore);
GenBool GENFUNSemaphoreV(void* pSemaphore);

#endif
