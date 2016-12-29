/*
 * Copyright (C) 2008 The Android Open Source Project
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
#include <power.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <ctype.h>

enum {
    ACQUIRE_PARTIAL_WAKE_LOCK = 0,
    RELEASE_WAKE_LOCK,
    OUR_FD_COUNT
};

const char * const OLD_PATHS[] = {
    "/sys/android_power/acquire_partial_wake_lock",
    "/sys/android_power/release_wake_lock",
};

const char * const NEW_PATHS[] = {
    "/sys/power/wake_lock",
    "/sys/power/wake_unlock",
};

//XXX static pthread_once_t g_initialized = THREAD_ONCE_INIT;
static int g_initialized = 0;
static int g_fds[OUR_FD_COUNT];
static int g_error = 1;

static int64_t systemTime()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000000LL + t.tv_nsec;
}

static int open_file_descriptors(const char * const paths[])
{
    int i;
    for (i = 0; i < OUR_FD_COUNT; i++)
    {
        int fd = open(paths[i], O_RDWR | O_CLOEXEC);
        if (fd < 0)
        {
            fprintf(stderr, "fatal error opening \"%s\"\n", paths[i]);
            g_error = errno;
            return -1;
        }
        g_fds[i] = fd;
    }

    g_error = 0;
    return 0;
}

static inline void initialize_fds(void)
{
    // XXX: should be this:
    //pthread_once(&g_initialized, open_file_descriptors);
    // XXX: not this:
    if (g_initialized == 0)
    {
        if (open_file_descriptors(NEW_PATHS) < 0)
            open_file_descriptors(OLD_PATHS);
        g_initialized = 1;
    }
}

int acquire_wake_lock(int lock, const char* id)
{
    int fd;
	ssize_t len;

	len = strlen(id);
    initialize_fds();
    if (g_error)
        return g_error;


    if (lock == PARTIAL_WAKE_LOCK)
    {
        fd = g_fds[ACQUIRE_PARTIAL_WAKE_LOCK];
    }
    else
    {
        return EINVAL;
    }

    if(write(fd, id, len) != len)
		return -1;
	return 0;
}

int release_wake_lock(const char* id)
{
	ssize_t len;
    initialize_fds();

//    ALOGI("release_wake_lock id='%s'\n", id);

    if (g_error)
        return g_error;

	len = strlen(id);
	if(write(g_fds[RELEASE_WAKE_LOCK], id, len) != len)
		return -1;
	return 0;
}

int get_wake_lock_count()
{
    char buf[2048];
    char *d = " ";
    char *p;
    int count = 0;

    initialize_fds();
    if (g_error)
    {
        return -1;
    }

    memset(buf, 0, 1024);
    lseek(g_fds[ACQUIRE_PARTIAL_WAKE_LOCK], 0L, SEEK_SET);
    if (read(g_fds[ACQUIRE_PARTIAL_WAKE_LOCK], buf, 2048) < 0)
    {
        return -1;
    }
    p = strtok(buf, d);

    while (p)
    {
        if(isspace(*p))
            break;
        count++;
        p = strtok(NULL, d);
    }
    return count;
}
