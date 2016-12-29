#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/ctrl/edit.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "awencoder.h"

#include "videodev.h"
#include "dragonview.h"

#define ID_CAMERA ID_CAM1
#define video_Width 640
#define video_Height 480
#define dev_id 0

static bool flag_camera;
static int screen_Width;
static int screen_Height;
static int np =1 ;
static int np_show =1;
static struct buffer *buffers = NULL;
enum v4l2_buf_type type;
static int videofh = 0;
static long long timestamp_now = 0;
sem_t sem_id1;
pthread_mutex_t mutex;
struct v4l2_buffer buf_pop;
static HWND hwnd_camera;
pthread_t id_1,id_2;

struct buffer
{
	unsigned char* start;
	int length;
	unsigned char* rgb_addr;
};

struct screen_coordinate
{
	int lx;
	int ty;
	int rx;
	int by;
} video;

long long get_timestamp(long long secs,long long usecs)
{
		long long timestamp;
		timestamp = usecs/1000 + secs* 1000;
		return timestamp;
}

int YU12toRGB24(unsigned char* pYUV,unsigned char* pBGR24,int width,int height)
{
    if (width < 1 || height < 1 || pYUV == NULL || pBGR24 == NULL)
        return 0;
    const long len = width * height;
    unsigned char* yData = pYUV;
    unsigned char* vData = &yData[len];
    unsigned char* uData = &vData[len >> 2];

    int bgr[3];
    int yIdx,uIdx,vIdx,idx;
    for (int i = 0;i < height;i++){
        for (int j = 0;j < width;j++){
            yIdx = i * width + j;
            vIdx = (i/2) * (width/2) + (j/2);
            uIdx = vIdx;

            bgr[0] = (int)(yData[yIdx] + 1.732446 * (uData[vIdx] - 128));                                    // b分量
            bgr[1] = (int)(yData[yIdx] - 0.698001 * (uData[uIdx] - 128) - 0.703125 * (vData[vIdx] - 128));    // g分量
            bgr[2] = (int)(yData[yIdx] + 1.370705 * (vData[uIdx] - 128));                                    // r分量

            for (int k = 0;k < 3;k++){
                idx = (i * width + j) * 3 + k;
                if(bgr[k] >= 0 && bgr[k] <= 255)
                    pBGR24[idx] = bgr[k];
                else
                    pBGR24[idx] = (bgr[k] < 0)?0:255;
            }
        }
    }
    return 1;
}

int init_capture()
{
	int n_buffers = 0;
	struct v4l2_input inp;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers req;
	int ret;
	/*get videofh*/
	if((videofh = open("/dev/video0", O_RDWR, 0)) < 0) {
		printf("can not open video0 and videofh is %d\n!!",videofh);
		return -1;
	}
	/* set VIDEO input index */
	inp.index = dev_id;
	inp.type = V4L2_INPUT_TYPE_CAMERA;
	if (ioctl(videofh, VIDIOC_S_INPUT, &inp) == -1) {
		db_error("VIDIOC_S_INPUT failed! s_input: %d\n",inp.index);
		return -1;
	}
	/* set capture parms */
	struct v4l2_streamparm parms;
	parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parms.parm.capture.timeperframe.numerator = 1;
	parms.parm.capture.timeperframe.denominator = 30;
	if (ioctl(videofh, VIDIOC_S_PARM, &parms) == -1) {
		db_error("VIDIOC_S_PARM failed!\n");
		return -1;
	}
	/* set image format */
	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = video_Width;
	fmt.fmt.pix.height = video_Height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	fmt.fmt.pix.field		= V4L2_FIELD_NONE;
	if (ioctl(videofh, VIDIOC_S_FMT, &fmt) < 0) {
		db_error("VIDIOC_S_FMT failed!\n");
		return -1;
	}
	/* reqBuffers */
	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count  = 10;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if(ioctl(videofh, VIDIOC_REQBUFS, &req) < 0) {
		db_error("VIDIOC_REQBUFS failed\n");
		return -1;
	}
	/* QUERYBUF */
	struct v4l2_buffer buf;
	buffers = (struct buffer*)calloc(req.count, sizeof(struct buffer));
	for (n_buffers= 0; n_buffers < req.count; ++n_buffers) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= n_buffers;
		if (ioctl(videofh, VIDIOC_QUERYBUF, &buf) == -1) {
			db_error("VIDIOC_QUERYBUF error\n");
			return -1;
		}
		buffers[n_buffers].length= buf.length;
		buffers[n_buffers].start = (unsigned char*)mmap(	NULL /* start anywhere */,
															buf.length,
															PROT_READ | PROT_WRITE /* required */,
															MAP_SHARED /* recommended */,
															videofh, buf.m.offset);

		//buffers[n_buffers].rgb_addr = (unsigned char *)malloc(video_Width*video_Height*3);
		db_debug("map buffer index: %d, mem: 0x%x, len: %x, offset: %x\n",
				n_buffers,(int)buffers[n_buffers].start,buf.length,buf.m.offset);
	}
	/* QBUF */
	for(n_buffers = 0; n_buffers < req.count; n_buffers++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= n_buffers;
		if (ioctl(videofh, VIDIOC_QBUF, &buf) == -1) {
			db_error("VIDIOC_QBUF error\n");
			return -1;
		}
	}
	return 0;
}

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

void FIFO_Push(P_FIFO_T hFifo, void *pvBuff)
{
	P_FIFO_T pstFifo = hFifo;
	char *pcNewBuff = (char *)pvBuff;
	char *pcBuffEnd = (char *)pstFifo->pvDataBuff + pstFifo->nNodeSize * pstFifo->nNodeMax;

	//写到了读的位置 需要选择覆盖或者丢弃
	if ((pstFifo->pvWrite == pstFifo->pvRead) &&
			(pstFifo->nWriteTimes > pstFifo->nReadTimes))
	{	//printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$pop is too slower than push!so drop or cover the node!\n");
		//pstFifo->nWriteTimes++;
	}
	else
	{	if (pstFifo->pvWrite == pcBuffEnd)
		{	pstFifo->pvWrite = pstFifo->pvDataBuff;
			memcpy(pstFifo->pvWrite, pcNewBuff, pstFifo->nNodeSize);
			pstFifo->pvWrite = (char *)pstFifo->pvWrite + pstFifo->nNodeSize;
			pstFifo->nNodeCount++;
			pstFifo->nWriteTimes++;
			sem_post(&sem_id1);
		}
		else
		{	memcpy(pstFifo->pvWrite, pcNewBuff, pstFifo->nNodeSize);
			pstFifo->pvWrite = (char *)pstFifo->pvWrite + pstFifo->nNodeSize;
			pstFifo->nNodeCount++;
			pstFifo->nWriteTimes++;
			sem_post(&sem_id1);
		}
	}
}

void FIFO_Pop(P_FIFO_T hFifo, void *pvBuff)
{
	if ( hFifo->nNodeCount == 0)
	{
		//printf("NO buffer pop!!!\n");
	}
	else if( hFifo->nNodeCount > 0)
	{
	P_FIFO_T pstFifo = hFifo;
	char *pcReadBuff = (char *)pvBuff;
	memset(pcReadBuff, 0x00,pstFifo->nNodeSize);
	char *pcBuffEnd = (char *)pstFifo->pvDataBuff  + pstFifo->nNodeSize * pstFifo->nNodeMax;
		if ( pstFifo->pvRead == pcBuffEnd)
			pstFifo->pvRead = pstFifo->pvDataBuff;
	memcpy(pcReadBuff, pstFifo->pvRead, pstFifo->nNodeSize);
	pstFifo->pvRead =  (char *)pstFifo->pvRead + pstFifo->nNodeSize;
	pstFifo->nReadTimes++;
	pstFifo->nNodeCount--;
	}
}

P_FIFO_T fifo;
void* capture_thread(void*)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(videofh, &fds);
	struct v4l2_buffer buf1;
	int ret;
	struct timeval tv;
	int nwrite;
	VideoInputBuffer co_buf;
	//init the co_buf
	memset(&co_buf, 0x00, sizeof(VideoInputBuffer));
	//init the buf1
	memset(&buf1, 0, sizeof(struct v4l2_buffer));
	buf1.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf1.memory = V4L2_MEMORY_MMAP;
	//streamon
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(videofh, VIDIOC_STREAMON, &type) == -1)
		db_error("VIDIOC_STREAMON error! %s\n",strerror(errno));
	//int num =10000;
	int buf_length = video_Width*video_Height*3/2;
	while(flag_camera)
	{
		//wait for sensor capture data
		tv.tv_sec  = 1;
		tv.tv_usec = 0;
		ret = select(videofh + 1, &fds, NULL, NULL, &tv);
		if (ret == -1)
			db_error("select error\n");
		else if (ret == 0)
			db_error("select timeout\n");
		/* DQBUF */
		ret = ioctl(videofh, VIDIOC_DQBUF, &buf1);
		if (ret == 0)
			printf("\n************DQBUF[%d] FINISH**************\n\n ",buf1.index);
		/* PUSH BUF */
		if(np %3 == 1)
		{
			pthread_mutex_lock(&mutex);
			FIFO_Push(fifo, &buf1);
			pthread_mutex_unlock(&mutex);
		}
		/* co_buf.nPts = get_timestamp((long long)(buf1.timestamp.tv_sec),(long long)(buf1.timestamp.tv_usec));
		printf("before decoder the co_buf.nPts is = %lld ms\n",co_buf.nPts);
		printf("the interval of two frames is %lld ms\n",co_buf.nPts - timestamp_now );
		timestamp_now = co_buf.nPts; */
		/* QBUF */
		if(ioctl(videofh, VIDIOC_QBUF, &buf1) == 0)
			printf("\n************QBUF[%d] FINISH**************\n\n ",buf1.index);
		np++;
	}
	// VIDOIC_STREAMOFF and ummap buf
	/* ioctl(videofh, VIDIOC_STREAMOFF, &type);
	db_msg("capture quit!\n");
	for (int i = 0; i < 10; i++)
	{
		db_debug("ummap index: %d, mem: %x, len: %x\n",
				i,(unsigned int)buffers[i].start,(unsigned int)buffers[i].length);
		munmap(buffers[i].start, buffers[i].length);
	} */
	return (void*)0;
}

void init_mybitmap(MYBITMAP *a_my_bmp)
{
	a_my_bmp->flags = MYBMP_TYPE_RGB | MYBMP_FLOW_DOWN | MYBMP_RGBSIZE_3;
	a_my_bmp->frames = 1;
	a_my_bmp->depth = 24;
	a_my_bmp->alpha = 0;
	a_my_bmp->transparent = 0;
	a_my_bmp->w = video_Width;
	a_my_bmp->h = video_Height;
	a_my_bmp->pitch = video_Width * 3;
	a_my_bmp->size = video_Width * video_Height * 3;
}

void* show_thread(void* param)
{
	HDC hdc;
	MYBITMAP a_my_bmp;
	BITMAP Bitmap1;
	init_mybitmap(&a_my_bmp);
	int index ;
	struct screen_coordinate *p = (struct screen_coordinate *)param;
	while(flag_camera)
	{
			sem_wait(&sem_id1);
			pthread_mutex_lock(&mutex);
			FIFO_Pop(fifo, &buf_pop);
			pthread_mutex_unlock(&mutex);
			printf("buf_pop.index is [%d]\n",buf_pop.index);
			index = buf_pop.index;
			/* transfer YU12 to RGB24 */
			buffers[index].rgb_addr = (unsigned char *)malloc(video_Width*video_Height*3);
			YU12toRGB24( buffers[index].start, buffers[index].rgb_addr , video_Width, video_Height);
			a_my_bmp.bits = buffers[index].rgb_addr;
			/* fill box with bitmap */
			hdc = GetClientDC(hwnd_camera);
			ExpandMyBitmap (hdc, &Bitmap1, &a_my_bmp, NULL, 0);
			FillBoxWithBitmap( hdc, 0, 0, p->rx, p->by, &Bitmap1);
			/* set caption */
			/* sprintf(caption,"%s - %s","Picture Viewer","cody");
			SetWindowCaption (hWnd,caption); */
			/* end paint and free */
			EndPaint(hwnd_camera, hdc);
			free(Bitmap1.bmBits);
			free(buffers[index].rgb_addr);
			printf("frame[%d] has show !!!\n",np_show);
			np_show++;
			usleep(30*1000);
	}
/* 	sem_destroy(&sem_id1); */
}

void Camera_exit()
{
	printf("Camera exit and release something!!!!!\n");
	flag_camera = 0;
	pthread_join(id_1,NULL);
	pthread_join(id_2,NULL);
	/*streamoff*/
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(videofh, VIDIOC_STREAMOFF, &type);
	for (int i = 0; i < 10; i++)
	{
		db_debug("ummap index: %d, mem: %x, len: %x\n",
				i,(unsigned int)buffers[i].start,(unsigned int)buffers[i].length);
		munmap(buffers[i].start, buffers[i].length);
	}
	free(buffers);
	close(videofh);
	sem_destroy(&sem_id1);
}

/* 二级camera窗口代码 */
static int CameraWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwnd_camera;
	HDC hdc;
	char caption[300];
	MYBITMAP a_my_bmp;
	BITMAP Bitmap1;
	init_mybitmap(&a_my_bmp);
	screen_Width  = video.rx - video.lx;
	screen_Height = video.by - video.ty;
	int index ;
	//static RECT rcphoto ={ lcdWidth - screen_Width, 0, lcdWidth, screen_Height};
	static RECT rcphoto ={ 0, 0, screen_Width, screen_Height};
	switch (message) {
			case MSG_CREATE:
			//建立camera控件
			hwnd_camera = CreateWindow(CTRL_STATIC, "camera",
							WS_CHILD | WS_VISIBLE, ID_CAMERA,
							0, 0, screen_Width, screen_Height,
							hWnd, 0);
			/*初始化相关参数*/
			init_capture();//TODO 设置采集视频时间
			sem_init(&sem_id1, 0, 0);
			fifo =  FIFO_Creat(10, sizeof(struct v4l2_buffer));
			/*建立采集线程*/
			pthread_t id_1;
			pthread_create(&id_1, NULL, capture_thread,NULL);
			/*建立个定时器用来刷新每帧照片*/
			SetTimer (hwnd_camera, ID_CAMERA , 30);
            break;

		case MSG_TIMER:
			hdc = GetClientDC(hwnd_camera);
			InvalidateRect (hdc, &rcphoto, TRUE);
			break;

		case MSG_PAINT:
			/* wait and pop buf */
			while(1)
			{sem_wait(&sem_id1);
			pthread_mutex_lock(&mutex);
			FIFO_Pop(fifo, &buf_pop);
			pthread_mutex_unlock(&mutex);
			printf("buf_pop.index is [%d]\n",buf_pop.index);
			index = buf_pop.index;
			/* transfer YU12 to RGB24 */
			buffers[index].rgb_addr = (unsigned char *)malloc(video_Width*video_Height*3);
			YU12toRGB24( buffers[index].start, buffers[index].rgb_addr , video_Width, video_Height);
			a_my_bmp.bits = buffers[index].rgb_addr;
			/* fill box with bitmap */
			hdc = GetClientDC(hwnd_camera);
			ExpandMyBitmap (hdc, &Bitmap1, &a_my_bmp, NULL, 0);
			FillBoxWithBitmap( hdc, 0, 0, screen_Width, screen_Height, &Bitmap1);
			/* set caption */
			/* sprintf(caption,"%s - %s","Picture Viewer","cody");
			SetWindowCaption (hWnd,caption); */
			/* end paint and free */
			EndPaint(hWnd, hdc);
			free(Bitmap1.bmBits);
			free(buffers[index].rgb_addr);
			printf("frame[%d] has show !!!\n",np_show);
			np_show++;}
			return 0;

        case MSG_CLOSE:
            KillTimer(hwnd_camera, ID_CAMERA);
            DestroyAllControls(hWnd);
            DestroyMainWindow(hWnd);
            PostQuitMessage(hWnd);
			sem_destroy(&sem_id1);
            return 0;
    }
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

static void InitCameraWin ( PMAINWINCREATE pCreateInfo, HWND hWnd)
{
   pCreateInfo->dwStyle = WS_CHILD | WS_VISIBLE;
   pCreateInfo->dwExStyle = WS_EX_NONE;
   pCreateInfo->spCaption = "camera";
   pCreateInfo->hMenu = 0;
   pCreateInfo->hCursor = GetSystemCursor(0);
   pCreateInfo->hIcon = 0;
   pCreateInfo->MainWindowProc = CameraWinProc; //camera窗体回调函数
   pCreateInfo->lx = video.lx;
   pCreateInfo->ty = video.ty;
   pCreateInfo->rx = video.rx;
   pCreateInfo->by = video.by;
   printf("pcreateinfo lx is %d\n",pCreateInfo->lx);
   pCreateInfo->iBkColor = COLOR_lightwhite;
   pCreateInfo->dwAddData = 0;
   pCreateInfo->hHosting = hWnd;
}


void CameraWin (HWND hwnd, int x_start, int y_start, int x_end, int y_end)
{
    atexit(Camera_exit);
	/*video.lx = x_start;
	video.ty = y_start;
	video.rx = x_end;
	video.by = y_end;
	MAINWINCREATE CreateCameraWin; //新建一个窗口

	InitCameraWin (&CreateCameraWin, hwnd);
	HWND camera_hWnd;
	camera_hWnd = CreateMainWindow (&CreateCameraWin);//建立窗口

	if (camera_hWnd != HWND_INVALID)
	{
		printf("camera win create success !!\n");
		ShowWindow (camera_hWnd, SW_SHOWNORMAL); //显示窗口
	}
	screen_Width  = video.rx - video.lx;
	screen_Height = video.by - video.ty;*/

	hwnd_camera = CreateWindow(CTRL_STATIC, "camera",
							WS_CHILD | WS_VISIBLE, ID_CAMERA,
							x_start, y_start, x_end, y_end,
							hwnd, 0);
	/*初始化相关参数*/
	if (init_capture() < 0) {
		printf("return to out !!");
		HDC hdc;
		hdc = BeginPaint (hwnd_camera);
		TextOut (hdc, (x_end-x_start)/2, (y_end-y_start)/2, "No Camera!");
		EndPaint (hwnd_camera, hdc);
		return (void)0;
	}
	flag_camera = 1;
	sem_init(&sem_id1, 0, 0);
	fifo =  FIFO_Creat(10, sizeof(struct v4l2_buffer));
	/*建立采集线程*/
	pthread_create(&id_1, NULL, capture_thread, NULL);
	pthread_create(&id_2, NULL, show_thread, &video);
}

