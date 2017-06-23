#define LOG_TAG "TinaPlayer"
#include "tinaplayer.h"
#include <string.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include "xplayer.h"
#include "tinasoundcontrol.h"

#define SAVE_PCM_DATA 0
#define SAVE_YUV_DATA 0

#if SAVE_PCM_DATA
	FILE* savaPcmFd = NULL;
#endif

#if SAVE_YUV_DATA
	FILE* savaYuvFd = NULL;
#endif


namespace aw{

	typedef struct PlayerPriData{
		XPlayer*		mXPlayer;
		void*				mUserData;
		int                 mVolume;
		int                 mLoop;
		int					mVideoFrameNum;
		int				mAudioFrameNum;
		NotifyCallback		mNotifier;
		SoundCtrl*          mSoundCtrl;
	}PlayerPriData;

	//* a callback for awplayer.
	int CallbackForXPlayer(void* pUserData, int msg, int param0, void* param1)
	{
		int ret = 0;
		int app_msg = 0;
		TinaPlayer* p = NULL;
		p = (TinaPlayer*)pUserData;
	    switch(msg)
	    {
		/*
	        case NOTIFY_NOT_SEEKABLE:
	        {
				logd(" ***NOTIFY_NOT_SEEKABLE***");
				app_msg = TINA_NOTIFY_NOT_SEEKABLE;
	            break;
	        }
			*/
	        case AWPLAYER_MEDIA_ERROR:
	        {
				logd(" ****NOTIFY_ERROR***");
				app_msg = TINA_NOTIFY_ERROR;
	            break;
	        }

	        case AWPLAYER_MEDIA_PREPARED:
	        {
				logd(" ***NOTIFY_PREPARED***");
				app_msg = TINA_NOTIFY_PREPARED;
	            break;
	        }

	        case AWPLAYER_MEDIA_BUFFERING_UPDATE:
	        {
	            //int nBufferedFilePos;
	            //int nBufferFullness;

	            //nBufferedFilePos = param0 & 0x0000ffff;
	            //nBufferFullness  = (param0>>16) & 0x0000ffff;
	            //TLOGD("info: buffer %d percent of the media file, buffer fullness = %d percent.\n",
	            //    nBufferedFilePos, nBufferFullness);
	            //TLOGD(" NOTIFY_BUFFERRING_UPDATE\n");
				//app_msg = TINA_NOTIFY_BUFFERRING_UPDATE;
	            break;
	        }

	        case AWPLAYER_MEDIA_PLAYBACK_COMPLETE:
	        {
	            logd(" ****NOTIFY_PLAYBACK_COMPLETE****");
				if(p){
					if(p->mPriData->mLoop == 0){
						app_msg = TINA_NOTIFY_PLAYBACK_COMPLETE;
					}
				}
	            break;
	        }
			/*
	        case NOTIFY_RENDERING_START:
	        {
	            logd(" NOTIFY_RENDERING_START");
				app_msg = TINA_NOTIFY_RENDERING_START;
	            break;
	        }
			*/
	        case AWPLAYER_MEDIA_SEEK_COMPLETE:
	        {
	            logd(" NOTIFY_SEEK_COMPLETE****");
				app_msg = TINA_NOTIFY_SEEK_COMPLETE;
				if(param1 != NULL){
					logd(" seek to result = %d",((int*)param1)[0]);
					logd(" seek to %d ms",((int*)param1)[1]);
				}
	            break;
	        }

			/*

			case NOTIFY_BUFFER_START:
	        {
	            logd("NOTIFY_BUFFER_START:no enough data to play");
				app_msg = TINA_NOTIFY_BUFFER_START;
	            break;
	        }

			case NOTIFY_BUFFER_END:
	        {
	            logd(" NOTIFY_BUFFER_END:has enough data to play again");
				app_msg = TINA_NOTIFY_BUFFER_END;
	            break;
	        }

	        case NOTIFY_VIDEO_FRAME:
	        {
				#if SAVE_YUV_DATA
					VideoPicData* videodata = (VideoPicData*)param1;
					if(videodata){
						//logd("*****NOTIFY_VIDEO_FRAME****,videodata->nPts = %lld ms",videodata->nPts/1000);
						if(savaYuvFd!=NULL){
							if(p->mVideoFrameNum == 200){
								logd(" *****NOTIFY_VIDEO_FRAME****,videodata->ePixelFormat = %d,videodata->nWidth = %d,videodata->nHeight=%d",videodata->ePixelFormat,videodata->nWidth,videodata->nHeight);
								int write_ret0 = fwrite(videodata->pData0, 1, videodata->nWidth*videodata->nHeight, savaYuvFd);
								if(write_ret0 <= 0){
									loge("yuv write0 error,err str: %s",strerror(errno));
								}
								int write_ret1 = fwrite(videodata->pData1, 1, videodata->nWidth*videodata->nHeight/2, savaYuvFd);
								if(write_ret1 <= 0){
									loge("yuv write1 error,err str: %s",strerror(errno));
								}
								logd("only save 1 video frame\n");
								fclose(savaYuvFd);
								savaYuvFd = NULL;
							}
							p->mVideoFrameNum++;
							//if(p->mVideoFrameNum >= 100){
							//	logd("only save 100 video frame\n");
							//	fclose(savaYuvFd);
							//	savaYuvFd = NULL;
							//}
						}
					}
				#endif
				app_msg = TINA_NOTIFY_VIDEO_FRAME;
				break;
	        }

	        case NOTIFY_AUDIO_FRAME:
	        {
				#if SAVE_PCM_DATA
					AudioPcmData* pcmData = (AudioPcmData*)param1;
					if(pcmData){
						if(savaPcmFd!=NULL){
							//logd(" *****NOTIFY_AUDIO_FRAME#####,*pcmData->pData = %p,pcmData->nSize = %d",*(pcmData->pData),pcmData->nSize);
							int write_ret = fwrite(pcmData->pData, 1, pcmData->nSize, savaPcmFd);
							if(write_ret <= 0){
								loge("pcm write error,err str: %s",strerror(errno));
							}
							p->mAudioFrameNum++;
							if(p->mAudioFrameNum >= 500){
								logd("only save 500 audio frame\n");
								fclose(savaPcmFd);
								savaPcmFd = NULL;
							}
						}
					}
				#endif
				app_msg = TINA_NOTIFY_AUDIO_FRAME;
				break;
	        }

	        case NOTIFY_VIDEO_PACKET:
	        {
			//DemuxData* videoData = (DemuxData*)param1;
				//logd("videoData pts: %lld", videoData->nPts);
				//static int frame = 0;
				//if(frame == 0)
				//{
				//	FILE* outFp = fopen("/mnt/UDISK/video.jpg", "wb");
		        //	if(videoData->nSize0)
		        //	{
		        //		fwrite(videoData->pData0, 1, videoData->nSize0, outFp);
		        //	}
		        //	if(videoData->nSize1)
		        //	{
		        //		fwrite(videoData->pData1, 1, videoData->nSize1, outFp);
		        //	}
		        //	fclose(outFp);
		        //	frame ++;
			//}
			//TLOGD(" NOTIFY_VIDEO_PACKET\n");
			//app_msg = TINA_NOTIFY_VIDEO_PACKET;
			break;
	        }

	        case NOTIFY_AUDIO_PACKET:
	        {
			//DemuxData* audioData = (DemuxData*)param1;
				//logd("audio pts: %lld", audioData->nPts);
				//static int audioframe = 0;
				//if(audioframe == 0)
				//{
				//	FILE* outFp = fopen("/mnt/UDISK/audio.pcm", "wb");
		        //	if(audioData->nSize0)
		        //	{
		        //		fwrite(audioData->pData0, 1, audioData->nSize0, outFp);
		        //	}
		        //	if(audioData->nSize1)
		        //	{
		        //		fwrite(audioData->pData1, 1, audioData->nSize1, outFp);
		        //	}
		        //	fclose(outFp);
		        //	audioframe ++;
			//}
			//TLOGD(" NOTIFY_AUDIO_PACKET\n");
			//app_msg = TINA_NOTIFY_AUDIO_PACKET;
			break;

	        }
			*/
	        default:
	        {
	            logd(" warning: unknown callback from AwPlayer");
	            break;
	        }
	    }
		if(app_msg != 0){
			if(p){
				p->callbackToApp(app_msg, param0, param1);
			}
		}
		return ret;
	}
	TinaPlayer::TinaPlayer()
	{
		logd(" TinaPlayer() contructor begin");
		mPriData = (PlayerPriData*)malloc(sizeof(PlayerPriData));
	memset(mPriData,0x00,sizeof(PlayerPriData));
		mPriData->mLoop = 0;
		mPriData->mNotifier = NULL;
		mPriData->mUserData = NULL;
		mPriData->mVideoFrameNum = 0;
		mPriData->mAudioFrameNum = 0;
		mPriData->mVolume = 20;
		mPriData->mXPlayer = XPlayerCreate();

		mPriData->mSoundCtrl = TinaSoundDeviceInit();
		if(mPriData->mSoundCtrl == NULL){
			loge(" _SoundDeviceInit(),ERR:soundCtrl == NULL");
		}
		if(mPriData->mXPlayer != NULL){

			XPlayerSetAudioSink(mPriData->mXPlayer,mPriData->mSoundCtrl);
		}
		logd(" TinaPlayer() contructor finish");
	}

	TinaPlayer::~TinaPlayer()
	{
		logd(" ~TinaPlayer() contructor begin");
		if(mPriData->mXPlayer != NULL)
	        XPlayerDestroy(mPriData->mXPlayer);
	    free(mPriData);
	    mPriData = NULL;
		logd(" ~TinaPlayer() contructor finish");
	}

	int TinaPlayer::initCheck()
	{
		int ret = -1;
		if(mPriData->mXPlayer == NULL)
	    {
	        loge("initCheck() fail, XPlayer::mplayer = %p", mPriData->mXPlayer);
	    }else{
			ret = XPlayerInitCheck(mPriData->mXPlayer);
		}
	return ret;
	}

	int TinaPlayer::setNotifyCallback(NotifyCallback notifier, void* pUserData)
	{
		mPriData->mNotifier = notifier;
		mPriData->mUserData = pUserData;
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("setNotifyCallback fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerSetNotifyCallback(mPriData->mXPlayer, CallbackForXPlayer, (void*)this);
		}
		return ret;
	}

	int TinaPlayer::setDataSource(const char* pUrl, const map<string, string>* pHeaders)
	{
		logd("llh>>>TinaPlayer::setDataSource begin");
		if(mPriData->mXPlayer == NULL){
			loge("setDataSource fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
			return -1;
		}else{
			logd("llh>>>TinaPlayer::mPriData->mXPlayer != NULL,XPlayerSetDataSourceUrl begin");
			return XPlayerSetDataSourceUrl(mPriData->mXPlayer, pUrl, NULL, NULL);
		}
		logd("llh>>>TinaPlayer::setDataSource finish");
	}


	int TinaPlayer::prepareAsync()
	{
		pid_t tid = syscall(SYS_gettid);
		logd("TinaPlayer::prepareAsync() begin,tid = %d",tid);
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("prepareAsync fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerPrepareAsync(mPriData->mXPlayer);
		}
		logd("TinaPlayer::prepareAsync() finish,tid = %d",tid);
		return ret;
	}

	int TinaPlayer::prepare()
	{
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("prepare fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerPrepare(mPriData->mXPlayer);
		}
		return ret;
	}

	int TinaPlayer::start()
	{
		#if SAVE_PCM_DATA
			savaPcmFd = fopen("/tmp/save.pcm", "wb");
			if(savaPcmFd==NULL){
				loge("fopen save.pcm fail****");
				loge("err str: %s",strerror(errno));
			}else{
				fseek(savaPcmFd,0,SEEK_SET);
			}
		#endif
		#if SAVE_YUV_DATA
			savaYuvFd = fopen("/tmp/save.yuv", "wb");
			if(savaYuvFd==NULL){
				loge("fopen save.yuv fail****");
				loge("err str: %s",strerror(errno));
			}else{
				fseek(savaYuvFd,0,SEEK_SET);
			}
		#endif
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("start fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerStart(mPriData->mXPlayer);
		}
		return ret;
	}

	int TinaPlayer::stop()
	{
		#if SAVE_PCM_DATA
			if(savaPcmFd!=NULL){
				fclose(savaPcmFd);
				savaPcmFd = NULL;
			}
		#endif
		#if SAVE_YUV_DATA
			if(savaYuvFd!=NULL){
				fclose(savaYuvFd);
				savaYuvFd = NULL;
			}
		#endif
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("stop fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerStop(mPriData->mXPlayer);
		}
		return ret;
	}

	int TinaPlayer::pause()
	{
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("pause fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerPause(mPriData->mXPlayer);
		}
		return ret;
	}

	int TinaPlayer::seekTo(int msec)
	{
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("seekTo fail,XPlayer::mplayer = %p,seek to %d ms",mPriData->mXPlayer,msec);
		}else{
			ret = XPlayerSeekTo(mPriData->mXPlayer,msec);
		}
		return ret;
	}

	int TinaPlayer::reset()
	{
		#if SAVE_PCM_DATA
			if(savaPcmFd!=NULL){
				fclose(savaPcmFd);
				savaPcmFd = NULL;
			}
		#endif
		#if SAVE_YUV_DATA
			if(savaYuvFd!=NULL){
				fclose(savaYuvFd);
				savaYuvFd = NULL;
			}
		#endif
		pid_t tid = syscall(SYS_gettid);
		logd("TinaPlayer::reset() begin,tid = %d",tid);
		struct timeval time1, time2, time3;
		memset(&time3, 0, sizeof(struct timeval));
		gettimeofday(&time1, NULL);
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("reset fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerReset(mPriData->mXPlayer);
		}
		gettimeofday(&time2, NULL);
		time3.tv_sec += (time2.tv_sec-time1.tv_sec);
		time3.tv_usec += (time2.tv_usec-time1.tv_usec);
		logd("TinaPlayer::reset() >>> time elapsed: %ld seconds  %ld useconds\n", time3.tv_sec, time3.tv_usec);
		logd("TinaPlayer::reset() finish,tid = %d",tid);
		return ret;
	}


	int TinaPlayer::isPlaying()
	{
		int ret = 0;
		if(mPriData->mXPlayer == NULL){
			loge("isPlaying fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerIsPlaying(mPriData->mXPlayer);
		}
		return ret;
	}


	int TinaPlayer::getCurrentPosition(int* msec)
	{
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("getCurrentPosition fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerGetCurrentPosition(mPriData->mXPlayer,msec);
		}
		return ret;
	}


	int TinaPlayer::getDuration(int *msec)
	{
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("getDuration fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerGetDuration(mPriData->mXPlayer,msec);
		}
		return ret;
	}


	int TinaPlayer::setLooping(int loop)
	{
		mPriData->mLoop = loop;
		int ret = -1;
		if(mPriData->mXPlayer == NULL){
			loge("setLooping fail,XPlayer::mplayer = %p",mPriData->mXPlayer);
		}else{
			ret = XPlayerSetLooping(mPriData->mXPlayer,loop);
		}
		return ret;
	}

	/*now,this function do not do */
	int TinaPlayer::setVolume(int volume)
	{
		if(volume > 40){
			loge("the volume(%d) is larger than the largest volume(40),set it to 40",volume);
			volume = 40;
		}else if(volume < 0){
			loge("the volume(%d) is smaller than 0,set it to 0",volume);
			volume =0;
		}
		mPriData->mVolume = volume;
		//volume -= 20;
		return -1;
		//return ((AwPlayer*)mPlayer)->setVolume(volume);
	}

	int TinaPlayer::getVolume()
	{
		return mPriData->mVolume;
	}

	void TinaPlayer::callbackToApp(int msg, int param0, void* param1){
		if(mPriData->mNotifier){
			mPriData->mNotifier(mPriData->mUserData,msg,param0,param1);
		}else{
			loge(" mNotifier is null ");
		}
	}

	//int TinaPlayer::setVideoOutputScaleRatio(int horizonScaleDownRatio,int verticalScaleDownRatio){
	//	return ((AwPlayer*)mPlayer)->setVideoOutputScaleRatio(horizonScaleDownRatio,verticalScaleDownRatio);
	//}

}
