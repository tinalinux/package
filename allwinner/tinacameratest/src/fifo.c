#include "fifo.h"

P_FIFO_T FIFO_Creat(int nNodeMax, int nNodeSize)
{
	P_FIFO_T pstNewFifo = (P_FIFO_T)malloc(sizeof(FIFO_T));
	if (pstNewFifo == NULL)
		return NULL;

	pstNewFifo->nNodeMax	= nNodeMax;
	pstNewFifo->nNodeSize	= nNodeSize;
	pstNewFifo->nNodeCount	= 0;
	pstNewFifo->pvDataBuff	= (char *)malloc(nNodeMax * nNodeSize);
	pstNewFifo->pvRead		= pstNewFifo->pvDataBuff;
	pstNewFifo->pvWrite		= pstNewFifo->pvDataBuff;
	pstNewFifo->nReadTimes	= 0;
	pstNewFifo->nWriteTimes	= 0;

	return pstNewFifo;
}

int FIFO_Push(P_FIFO_T hFifo, void *pvBuff)
{
	P_FIFO_T pstFifo = hFifo;
	char *pcNewBuff = (char *)pvBuff;
	char *pcBuffEnd = (char *)pstFifo->pvDataBuff + pstFifo->nNodeSize * pstFifo->nNodeMax;

	//写到了读的位置 需要选择覆盖或者丢弃
	if ((pstFifo->pvWrite == pstFifo->pvRead) &&
			(pstFifo->nWriteTimes > pstFifo->nReadTimes)) {
		printf("pop is too slower than push!so drop or cover the node!\n");
		return -1;
	} else {
		memcpy(pstFifo->pvWrite, pcNewBuff, pstFifo->nNodeSize);
		pstFifo->pvWrite = (char *)pstFifo->pvWrite + pstFifo->nNodeSize;
		if (pstFifo->pvWrite == pcBuffEnd)
			pstFifo->pvWrite= pstFifo->pvDataBuff;
		pstFifo->nNodeCount++;
		pstFifo->nWriteTimes++;
		printf("PUSH FINISH NodeCount is %u", pstFifo->nNodeCount);
		return 1;
	}
	return 0;
}

int FIFO_Pop(P_FIFO_T hFifo, void *pvBuff)
{
	if ( hFifo->nNodeCount == 0)
	{
		//printf("NO buffer pop!!!\n");
		return -1;
	}
	else if( hFifo->nNodeCount > 0)
	{
		P_FIFO_T pstFifo = hFifo;
		char *pcReadBuff = (char *)pvBuff;
		memset(pcReadBuff, 0x00,pstFifo->nNodeSize);
		char *pcBuffEnd = (char *)pstFifo->pvDataBuff  + pstFifo->nNodeSize * pstFifo->nNodeMax;
		if (pstFifo->pvRead == pcBuffEnd)
			pstFifo->pvRead = pstFifo->pvDataBuff;
		memcpy(pcReadBuff, pstFifo->pvRead, pstFifo->nNodeSize);
		pstFifo->pvRead =  (char *)pstFifo->pvRead + pstFifo->nNodeSize;
		pstFifo->nReadTimes++;
		pstFifo->nNodeCount--;
		printf("POP FINISH NodeCount is %u", pstFifo->nNodeCount);
		return 1;
	}
	return 0;
}
