/*
 * Copyright (C) 2016 Allwinnertech
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __NP_SCENE_UTILS_H__
#define __NP_SCENE_UTILS_H__

typedef struct
{
    int (*SetBootLock)(const char *boot_lock);
    int (*SetRoomAge)(const char *room_age);
    int (*SetCpuFreq)(const char *cpu_freq);
    int (*SetCpuGov)(const char *cpu_gov);
    int (*SetCpuHot)(const char *cpu_hot);

    int (*GetCpuFreq)(char *cpu_freq, int len);
    int (*GetCpuOnline)(char *cpu_online, int len);
} CPU_SCENE_OPS;

typedef struct
{
    int (*SetGpuFreq)(const char *gpu_freq);

    int (*GetGpuFreq)(char *gpu_freq, int len);
} GPU_SCENE_OPS;

typedef struct
{
    int (*SetDramScen)(const char *dram_scen);

    int (*GetDramFreq)(char *dram_freq, int len);
} DRAM_SCENE_OPS;

typedef struct
{
    char bootlock[4];
    char cpu_freq[16];
    char roomage[64];
    char cpu_gov[16];
    char cpu_hot[16];
} CPU_SCENE;

typedef struct
{
    char gpu_freq[16];
} GPU_SCENE;

typedef struct
{
    char dram_freq[16];
    char dram_scene[8];
} DRAM_SCENE;

typedef struct
{
    CPU_SCENE cpu;
    GPU_SCENE gpu;
    DRAM_SCENE dram;
} NP_SCENE;

int np_get_scene_conifg(NP_SCENE *scene, const char *scene_name);

int np_set_scene(NP_SCENE *scene);

#endif
