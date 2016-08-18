#ifndef __C_POWERMANAGER_H
#define __C_POWERMANAGER_H

#ifdef CPP
extern "C"
{
#endif

int Power_Init(void);
int Power_Lock_TimeCnt(void);
int Power_Lock(void);
int Power_Unlock(void);
int Power_Get_LockCnt(void);


#ifdef CPP
}
#endif

#endif
