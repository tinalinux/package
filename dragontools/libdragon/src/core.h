/*
 * \file        core.h
 * \brief
 *
 * \version     1.0.0
 * \date        2012年05月31日
 * \author      James Deng <csjamesdeng@allwinnertech.com>
 *
 * Copyright (c) 2012 Allwinner Technology. All Rights Reserved.
 *
 */

#ifndef __CORE_H__
#define __CORE_H__

#include <libubox/list.h>

#ifndef TEMP_FAILURE_RETRY
/* Used to retry syscalls that can return EINTR. */
#define TEMP_FAILURE_RETRY(exp) ({         \
    typeof (exp) _rc;                      \
    do {                                   \
        _rc = (exp);                       \
    } while (_rc == -1 && errno == EINTR); \
    _rc; })
#endif

struct no_wait_task
{
    pid_t pid;
    struct testcase_base_info *base_info;
    struct list_head list;
};

int get_auto_testcases_number(void);

int get_manual_testcases_number(void);

int get_testcases_number(void);

int init_script(const char *path);
void exit_script(int shm);

struct testcase_base_info *init_testcases(void);
void exit_testcase(struct testcase_base_info *info);

int waiting_for_bootcompeled(int timeout);

int test_failed(int test_id);

int run_one_testcase(int version, char *exdata, int script_shmid, struct testcase_base_info *info);
int wait_for_test_completion(void);
#endif /* __CORE_H__ */
