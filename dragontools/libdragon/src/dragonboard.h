/*
 * \file        dragonboard.h
 * \brief
 *
 * \version     1.0.0
 * \date        2012年05月31日
 * \author      James Deng <csjamesdeng@allwinnertech.com>
 *
 * Copyright (c) 2012 Allwinner Technology. All Rights Reserved.
 *
 */

#ifndef __DRAGONBOARD_H__
#define __DRAGONBOARD_H__
#include <tina_log.h>

#define DRAGONBOARD_VERSION             0x160827
#define DRAGONBOARD_COPYRIGHT                       "DragonBoard V4.0 Copyright (C) 2016 Allwinner"

#define DRAGONBOARD

#define CMD_PIPE_NAME                   "/tmp/cmd_pipe"
#define VOLUME_PIPE_NAME                "/tmp/volume_pipe"
#define CAMERA_PIPE_NAME                "/tmp/camera_pipe"
#define MIC_PIPE_NAME                   "/tmp/mic_pipe"
#define WIFI_PIPE_NAME                  "/tmp/wifi_pipe"
#define TEST_COMPLETION                 "done"

#undef LOG_TAG
#define LOG_TAG "dragonboard"
//#undef	CONFIG_TLOG_LEVEL
//#define CONFIG_TLOG_LEVEL OPTION_TLOG_LEVEL_DETAIL

#define db_error(fmt, arg...) 	TLOGE(fmt, ##arg)
#define db_warn(fmt, arg...) 	TLOGW(fmt, ##arg)
#define db_msg(fmt, arg...) 	TLOGI(fmt, ##arg)
#define db_debug(fmt, arg...) 	TLOGD(fmt, ##arg)
#define db_dump(fmt, arg...) 	TLOGI(fmt, ##arg)

#endif /* __DRAGONBOARD_VERSION__ */
