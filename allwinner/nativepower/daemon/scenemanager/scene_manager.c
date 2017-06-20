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
#include <stdio.h>
#include <string.h>

#include "scene_manager.h"
#include "np_scene_utils.h"

char str_scene[NATIVEPOWER_SCENE_MAX][128] = {
        "boot_complete",
};

int np_scene_change(const char *scene_name)
{
    NP_SCENE scene;

    memset(&scene, 0, sizeof(NP_SCENE));

    if (np_get_scene_conifg(&scene, scene_name) < 0)
        return -1;

    return np_set_scene(&scene);
}
