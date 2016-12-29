#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "np_utils.h"

extern char str_scene[NATIVEPOWER_SCENE_MAX][128];
int np_scene_change(const char *scene_name);

#endif
