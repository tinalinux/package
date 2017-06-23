#ifndef __LOCALMUSIC_PLAYER
#define __LOCALMUSIC_PLAYER

void LocalMusic_State(void);
int LocalMusic_Init(void);
int LocalMusic_Play(const long num);
int LocalMusic_Stop(void);
int LocalMusic_Pause(void);
int LocalMusic_Next(void);
int LocalMusic_Previous(void);
int LocalMusic_Scan(void);
int LocalMusic_GetPath(long row);

#endif
