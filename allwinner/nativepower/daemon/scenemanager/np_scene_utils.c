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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "np_uci_config.h"
#include "np_scene_utils.h"
#include "SceneConfig.h"

int np_get_scene_conifg(NP_SCENE *scene, const char *scene_name)
{
    NP_UCI *uci = np_uci_open(NATIVE_POWER_CONFIG_PATH);
    if (uci == NULL)
        return -1;

    np_uci_read_config(uci, scene_name, "bootlock", scene->cpu.bootlock);
    np_uci_read_config(uci, scene_name, "cpu_freq", scene->cpu.cpu_freq);
    np_uci_read_config(uci, scene_name, "cpu_gov", scene->cpu.cpu_gov);
    np_uci_read_config(uci, scene_name, "cpu_hot", scene->cpu.cpu_hot);
    np_uci_read_config(uci, scene_name, "roomage", scene->cpu.roomage);

    np_uci_read_config(uci, scene_name, "gpu_freq", scene->gpu.gpu_freq);

    np_uci_read_config(uci, scene_name, "dram_freq", scene->dram.dram_freq);
    np_uci_read_config(uci, scene_name, "dram_scene", scene->dram.dram_scene);

    np_uci_close(uci);
    return 0;
}

static int setConfig(const char *path, const int flag, const char *value)
{
    int fd = -1;
    fd = open(path, flag);

    if (fd < 0)
    {
        return -1;
    }

    write(fd, value, strlen(value));

    return 0;
}

static int SetBootLock(const char * bootlock)
{
    return setConfig(CPU0LOCK, O_WRONLY, bootlock);
}

static int SetCpuGov(const char * cpu_gov)
{
    return setConfig(CPU0GOV, O_WRONLY, cpu_gov);
}

static int SetCpuHot(const char * cpu_hot)
{
    return setConfig(CPUHOT, O_WRONLY, cpu_hot);
}

static int SetDramScen(const char * dram_scen)
{
    return setConfig(DRAMPAUSE, O_WRONLY, dram_scen);
}

CPU_SCENE_OPS cpu_ops = {
       .SetBootLock = SetBootLock,
       .SetRoomAge = NULL,
       .SetCpuFreq = NULL,
       .SetCpuGov = SetCpuGov,
       .SetCpuHot = SetCpuHot,
       .GetCpuFreq = NULL,
       .GetCpuOnline = NULL,
};

GPU_SCENE_OPS gpu_ops = {
        .SetGpuFreq = NULL,
        .GetGpuFreq = NULL,
};

DRAM_SCENE_OPS dram_ops = {
        .SetDramScen = SetDramScen,
        .GetDramFreq = NULL,
};

int np_set_scene(NP_SCENE *scene)
{
    if (strlen(scene->cpu.bootlock) != 0)
        cpu_ops.SetBootLock(scene->cpu.bootlock);

    if (strlen(scene->cpu.roomage) != 0)
        cpu_ops.SetRoomAge(scene->cpu.roomage);

    if (strlen(scene->cpu.cpu_freq) != 0)
        cpu_ops.SetCpuFreq(scene->cpu.cpu_freq);

    if (strlen(scene->cpu.cpu_gov) != 0)
        cpu_ops.SetCpuGov(scene->cpu.cpu_gov);

    if (strlen(scene->cpu.cpu_hot) != 0)
        cpu_ops.SetCpuHot(scene->cpu.cpu_hot);

    if (strlen(scene->gpu.gpu_freq) != 0)
        gpu_ops.SetGpuFreq(scene->gpu.gpu_freq);

    if (strlen(scene->dram.dram_scene) != 0)
        dram_ops.SetDramScen(scene->dram.dram_scene);

    return 0;
}




