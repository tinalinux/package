#ifndef _C_TINAPLAYER_H
#define _C_TINAPLAYER_H

#ifdef CPP
extern "C"
{
#endif

#define UP_VOLUME 2
#define KEEP_VOLUME 1
#define DOWN_VOLUME 0
#define MAX_VOLUME 61
#define MIN_VOLUME 1

typedef enum{
    STATUS_STOPPED = 0,
    STATUS_PREPARING,
    STATUS_PREPARED,
    STATUS_PLAYING,
    STATUS_PAUSED,
    STATUS_COMPLETED,
    STATUS_SEEKING = 6,
}PlayerStatus;

PlayerStatus Tinaplayer_Get_Status(void);
int Tinaplayer_Set_Status(PlayerStatus status);
int Tinaplayer_Play(const char* path);
int Tinaplayer_Stop(void);
int Tinaplayer_Pause(void);
int Tinaplayer_IsPlaying(void);
int Tinaplayer_Init(void);
int Tinaplayer_Continue(void);
int Tinaplayer_SetVolume(int mode);
void Tinaplayer_Start(void);
void say(const char *fname);

#ifdef CPP
}
#endif

#endif
