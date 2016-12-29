
#ifndef _FIFO_H_
#define _FIFO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tagFIFO_T
{
	unsigned int nNodeSize;
	unsigned int nNodeMax;
	void *pvDataBuff;
	void *pvRead;
	void *pvWrite;
	unsigned int nNodeCount;
	unsigned int nReadTimes;
	unsigned int nWriteTimes;

}FIFO_T, *P_FIFO_T;

///////////////////////////////////////////////////////////////
//	函 数 名 : FIFO_Creat
//	函数功能 : FIFO的创建
//	处理过程 :
//
//	参数说明 : 无
//	返 回 值 : 0 表示成功，非0表示失败。
///////////////////////////////////////////////////////////////
P_FIFO_T FIFO_Creat(int nNodeMax, int nNodeSize);


///////////////////////////////////////////////////////////////
//	函 数 名 : FIFO_Push
//	函数功能 : 往FIFO写入数据
//	处理过程 :
//
//	参数说明 :	hFifo : FIFO的名称.
//
//	返 回 值 :
///////////////////////////////////////////////////////////////
int FIFO_Push(P_FIFO_T hFifo, void *pvBuff);



///////////////////////////////////////////////////////////////
//	函 数 名 : FIFO_Pop
//	函数功能 : 从FIFO读出数据
//	处理过程 :
//
//	参数说明 :	hFifo : FIFO的名称.
//				pvBuff: FIFO节点的指针
//	返 回 值 :
///////////////////////////////////////////////////////////////
int FIFO_Pop(P_FIFO_T hFifo, void *pvBuff);

#endif