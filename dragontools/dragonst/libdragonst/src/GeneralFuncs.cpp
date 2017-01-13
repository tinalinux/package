#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "GeneralFuncs.h"

GenBool GENFUNCreateThread(THREADHANDLE* pThreadHandle, PFUNCTHREADROUTINE pfnThreadRoutine, void* pParam)
{
    if (pThreadHandle == NULL)
    {
        return GEN_FALSE;
    }

	int iRet = 0;

#ifdef WIN32
	*pThreadHandle = CreateThread(NULL, 0, pfnThreadRoutine, pParam, 0, NULL);

	if (*pThreadHandle == NULL)
	{
		iRet = -1;
	}
#else
    iRet = pthread_create(pThreadHandle, NULL, pfnThreadRoutine, pParam);
#endif

	return (iRet == 0) ? GEN_TRUE : GEN_FALSE;
}


void GENFUNReleaseThread(THREADHANDLE* pThreadHandle)
{
    if (pThreadHandle != NULL)
    {
#ifdef WIN32
        WaitForSingleObject(*pThreadHandle, INFINITE);
        CloseHandle(*pThreadHandle);
#else
        pthread_join(*pThreadHandle, NULL);
#endif

        *pThreadHandle = THREADHANDLENULL;
    }
}


void GENFUNSleep(unsigned int uiMilliseconds)
{
#ifdef WIN32
    Sleep(uiMilliseconds);
#else
    struct timeval timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = uiMilliseconds * 1000;
    select(0, NULL, NULL, NULL, &timeout);
#endif
}


GenBool GENFUNGetCurrentPath(char* szBuffer, int iBufferSize)
{
    if (iBufferSize < MAX_PATH)
    {
        return GEN_FALSE;
    }

#ifdef WIN32
    int iLen = 0;
    int iPos = 0;

    GetModuleFileNameA(NULL, szBuffer, iBufferSize);
    iLen = strlen(szBuffer);

    for (iPos = iLen - 1; iPos > 0; iPos--)
    {
        if (szBuffer[iPos] == '\\')
        {
            break;
        }
    }

    if (iPos > 0)
    {
        memset(szBuffer + iPos, 0, iLen - iPos - 1);
    }
    else
    {
        memset(szBuffer, 0, iBufferSize);
    }
#else
    getcwd(szBuffer, iBufferSize);
#endif

    return GEN_TRUE;
}


GenBool GENFUNSemaphoreP(void* pSemaphore)
{
#ifdef WIN32
#else
   struct sembuf sem_b;
   sem_b.sem_num = 0;
   sem_b.sem_op = -1; // P
   sem_b.sem_flg = SEM_UNDO;

   if(semop(*((int*)pSemaphore), &sem_b, 1) == -1)
   {
       return GEN_FALSE;
   }
#endif

    return GEN_TRUE;
}


GenBool GENFUNSemaphoreV(void* pSemaphore)
{
#ifdef WIN32
#else
   struct sembuf sem_b;
   sem_b.sem_num = 0;
   sem_b.sem_op = 1; // V
   sem_b.sem_flg = SEM_UNDO;

   if(semop(*((int*)pSemaphore), &sem_b, 1) == -1)
   {
       return GEN_FALSE;
   }
#endif

    return GEN_TRUE;
}
