
/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : adapter.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/


#ifndef ADAPTER_H
#define ADAPTER_H

#include "ve.h"
//#include "memoryAdapter.h"
//#include "secureMemoryAdapter.h"
#include <sc_interface.h>

int   AdapterInitialize(struct ScMemOpsS *memops);

void  AdpaterRelease(struct ScMemOpsS *memops);

int   AdapterLockVideoEngine(int flag);

void  AdapterUnLockVideoEngine(int flag);

void  AdapterVeReset(void);

int   AdapterVeWaitInterrupt(void);

void* AdapterVeGetBaseAddress(void);

int   AdapterMemGetDramType(void);

#endif
