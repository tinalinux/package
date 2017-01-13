/*
 * \file        core.c
 * \brief
 *
 * \version     1.0.0
 * \date        2012年05月31日
 * \author      James Deng <csjamesdeng@allwinnertech.com>
 *
 * Copyright (c) 2012 Allwinner Technology. All Rights Reserved.
 *
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "dragonboard.h"
#include "test_case.h"
#include "script_parser.h"
#include "script.h"
#include "core.h"

#undef LOG_TAG
#define LOG_TAG "libdragon"

static struct list_head no_wait_task_list;

static int total_testcases = 0;
static int total_testcases_auto=0;
static int total_testcases_manual=0;

static void wait_completion(pid_t pid)
{
    int status;

    if (waitpid(pid, &status, 0) < 0) {
        db_error("launcher: waitpid(%d) failed(%s)", pid, strerror(errno));
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status)) {
        db_error("launcher: error in process %d(%d)", pid,
                WEXITSTATUS(status));
    }
}

int run_one_testcase(int version, char *exdata, int script_shmid,
        struct testcase_base_info *info)
{
    char *test_case_id_s;
    int test_case_id;
    char *result_s;
    int result;
    char **args;
    pid_t pid;
    char buffer[128];

    args = (char **)malloc(sizeof(char *) * 5);
    args[0] = info->binary;
    args[1] = (char *)malloc(10);
    sprintf(args[1], "%d", version);
    args[2] = (char *)malloc(10);
    sprintf(args[2], "%d", script_shmid);
    args[3] = (char *)malloc(10);
    sprintf(args[3], "%d", info->id);
    args[4] = NULL;

    pid = fork();
    if (pid < 0) {
        db_error("testcase: fork %s process failed(%s)", info->binary,
                strerror(errno));
        return -1;
    } else if (pid == 0) {
        db_msg("testcase: starting %s, args:0-%s,1-%s,2-%s,3-%s\n", info->binary, args[0],args[1],args[2],args[3]);
        execvp(info->binary, args);
        FILE *fail_pipe = fopen(CMD_PIPE_NAME, "we");
        setlinebuf(fail_pipe);
        fprintf(fail_pipe, "%d 0\n", info->id);
        fclose(fail_pipe);
        db_error("testcase: can't run %s(%s)", info->binary, strerror(errno));
        _exit(-1);
    }

    usleep(1000);
    /* now: we only wait auto test case due to system performance */
    if (info->run_type == WAIT_COMPLETION && info->category == CATEGORY_AUTO) {
        db_msg("launcher: wait %s completion(pid=%d)", info->binary, pid);
        wait_completion(pid);
    }
    else {
        struct no_wait_task *task;

        task = (struct no_wait_task *)malloc(sizeof(struct no_wait_task));
        if (task == NULL) {
            db_error("malloc no wait task failed");
            return -1;
        }
        memset(task, 0, sizeof(struct no_wait_task));

        task->pid = pid;
        task->base_info = info;
        list_add(&task->list, &no_wait_task_list);
    }
    return 0;
}

void task_remove(struct no_wait_task *task)
{
	if(!task)
		return;
	db_msg("task revmoe!!pid=%d", task->pid);
	list_del(&task->list);
	free(task);
}

int get_auto_testcases_number(void)
{
    return total_testcases_auto;
}

int get_manual_testcases_number(void)
{
    return total_testcases_manual;
}

int get_testcases_number(void)
{
    return total_testcases;
}

static struct testcase_base_info *parse_testcase(void)
{
    int i, j, mainkey_cnt;
    struct testcase_base_info *info;
    char mainkey_name[32], display_name[64], binary[16];
    int activated, category, run_type;
    int len;

    mainkey_cnt = script_mainkey_cnt();
    info = (struct testcase_base_info *)malloc(sizeof(struct testcase_base_info) * mainkey_cnt);
    if (info == NULL) {
        db_error("core: allocate memory for temporary test case basic "
                "information failed(%s)", strerror(errno));
        return NULL;
    }
    memset(info, 0, sizeof(struct testcase_base_info) * mainkey_cnt);

    for (i = 0, j = 0; i < mainkey_cnt; i++) {
        memset(mainkey_name, 0, 32);
        script_mainkey_name(i, mainkey_name);

        if (script_fetch(mainkey_name, "display_name", (int *)display_name, 16))
            continue;

        if (script_fetch(mainkey_name, "activated", &activated, 1))
            continue;

        if (display_name[0] && activated == 1) {
            strncpy(info[j].name, mainkey_name, 32);
            strncpy(info[j].display_name, display_name, 64);
            info[j].activated = activated;

            if (script_fetch(mainkey_name, "program", (int *)binary, 4) == 0) {
                strncpy(info[j].binary, binary, 16);
            }

            info[j].id = j;

            if (script_fetch(mainkey_name, "category", &category, 1) == 0) {
                info[j].category = category;

                if(category==0){
                    total_testcases_auto++;
                }else
                {
                    total_testcases_manual++;
                }
            }

            if (script_fetch(mainkey_name, "run_type", &run_type, 1) == 0) {
                info[j].run_type = run_type;
            }

            j++;
        }
    }
    total_testcases = j;

    db_msg("core: total test cases #%d", total_testcases);
    db_msg("core: total test cases_auto #%d", total_testcases_auto);
    db_msg("core: total test cases_manual #%d", total_testcases_manual);
    if (total_testcases == 0) {
        return NULL;
    }

    return info;
}

static void deparse_testcase(struct testcase_base_info *info)
{
    if (info != NULL)
        free(info);
    total_testcases = 0;
}

int init_script(const char *path)
{
    int script_shmid;
    int ret;

	if(!path)
		return -1;
    /* init script and view */
    db_msg("core: parse script %s...", path);
    script_shmid = parse_script(path);
    if (script_shmid == -1) {
        db_error("core: parse script failed");
        return -1;
    }

    db_msg("core: init script...");
    ret = init_script(script_shmid);
    if (ret) {
        db_error("core: init script failed(%d)", ret);
        return -1;
    }

    INIT_LIST_HEAD(&no_wait_task_list);
    return script_shmid;
}

void exit_script(int shm)
{
    deparse_script(shm);
}

struct testcase_base_info *init_testcases(void)
{
    struct testcase_base_info *info;

    db_msg("init pipe...");
    /* create named pipe */
    unlink(CMD_PIPE_NAME);
    if (mkfifo(CMD_PIPE_NAME, S_IFIFO | 0666) == -1) {
        db_error("core: mkfifo error(%s)", strerror(errno));
        return NULL;
    }

    /* parse and draw all test cases to view */
    db_msg("core: parse test case from script...");
    info = parse_testcase();
    if (info == NULL) {
        db_error("core: parse all test case from script failed");
        return NULL;
    }

    return info;
}

void exit_testcase(struct testcase_base_info *info)
{
    deparse_testcase(info);
    deinit_script();
}

int waiting_for_bootcompeled(int timeout)
{
    int fd = -1;
    char buf[16] = {0};
    int count = 0;
    while (timeout--) {
        fd = TEMP_FAILURE_RETRY(open("/tmp/booting_state", O_RDONLY));
        count = read(fd, buf, 16);
        if (count > 0 && !strncmp(buf, "done", 4))
            break;
        sleep(1);
    };
    return timeout;
}

int wait_for_test_completion(void)
{
    struct list_head *pos;
    list_for_each(pos, &no_wait_task_list) {
        struct no_wait_task *task = list_entry(pos, struct no_wait_task, list);
        db_msg("launcher: wait %s (pid=%d)completion", task->base_info->binary, task->pid);
        wait_completion(task->pid);
        task_remove(task);
    }

    FILE *cmd_pipe = fopen(CMD_PIPE_NAME, "we");
    setlinebuf(cmd_pipe);
    fprintf(cmd_pipe, "%s\n", TEST_COMPLETION);
    fclose(cmd_pipe);
}
