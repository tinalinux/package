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

#define SCRIPT_NAME "/boot-res/test_config.fex"
static struct list_head no_wait_task_list;

static int total_testcases = 0;
static int total_testcases_auto=0;
static int total_testcases_manual=0;

static void wait_completion(pid_t pid)
{
    int status;

    if (waitpid(pid, &status, 0) < 0) {
        db_error("launcher: waitpid(%d) failed(%s)\n", pid, strerror(errno));
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status)) {
        db_error("launcher: error in process %d(%d)\n", pid,
                WEXITSTATUS(status));
    }
}

int run_one_testcase(int version, char *exdata, int script_shmid,
        struct testcase_base_info *info)
{
    FILE *cmd_pipe;
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
        db_error("testcase: fork %s process failed(%s)\n", info->binary,
                strerror(errno));
        return -1;
    }
    else if (pid == 0) {
        db_msg("testcase: starting %s\n", info->binary);
        execvp(info->binary, args);
        FILE *fail_pipe = fopen(CMD_PIPE_NAME, "w");
        setlinebuf(cmd_pipe);
        fprintf(fail_pipe, "%d 0\n", info->id);
        fclose(fail_pipe);
        db_error("testcase: can't run %s(%s)\n", info->binary, strerror(errno));
        _exit(-1);
    }

    usleep(1000);

    cmd_pipe = fopen(CMD_PIPE_NAME, "r");
    if (fgets(buffer, sizeof(buffer), cmd_pipe) == NULL) {
        db_dump("core: command: %s\n", buffer);
        return -1;
    }

    test_case_id_s = strtok(buffer, " \n");
    db_dump("test case id #%s\n", test_case_id_s);
    if (test_case_id_s == NULL)
        return -1;
    test_case_id = atoi(test_case_id_s);

    result_s = strtok(NULL, " \n");
    db_dump("result: %s\n", result_s);
    if (result_s == NULL)
        return -1;
    result = atoi(result_s);
    exdata = strtok(NULL, "\n");
    wait_completion(pid);
    fclose(cmd_pipe);
    return result;

#if 0
    if (info->run_type == WAIT_COMPLETION && info->category == CATEGORY_AUTO) {
        db_msg("launcher: wait %s completion\n", info->binary);
        wait_completion(pid);
    }
    else {
        struct no_wait_task *task;

        task = malloc(sizeof(struct no_wait_task));
        if (task == NULL)
            return -1;
        memset(task, 0, sizeof(struct no_wait_task));

        task->pid = pid;
        task->base_info = info;
        list_add(&task->list, &no_wait_task_list);
    }
#endif
    return 0;
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
                "information failed(%s)\n", strerror(errno));
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

    db_msg("core: total test cases #%d\n", total_testcases);
    db_msg("core: total test cases_auto #%d\n", total_testcases_auto);
    db_msg("core: total test cases_manual #%d\n", total_testcases_manual);
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

int init_script(void)
{
    int script_shmid;
    int ret;

    /* init script and view */
    db_msg("core: parse script %s...\n", SCRIPT_NAME);
    script_shmid = parse_script(SCRIPT_NAME);
    if (script_shmid == -1) {
        db_error("core: parse script failed\n");
        return -1;
    }

    db_msg("core: init script...\n");
    ret = init_script(script_shmid);
    if (ret) {
        db_error("core: init script failed(%d)\n", ret);
        return -1;
    }

    return script_shmid;
}

void exit_script(int shm)
{
    deparse_script(shm);
}

struct testcase_base_info *init_testcases(void)
{
    struct testcase_base_info *info;

    db_msg("init pipe...\n");
    /* create named pipe */
    unlink(CMD_PIPE_NAME);
    if (mkfifo(CMD_PIPE_NAME, S_IFIFO | 0666) == -1) {
        db_error("core: mkfifo error(%s)\n", strerror(errno));
        return NULL;
    }

    /* parse and draw all test cases to view */
    db_msg("core: parse test case from script...\n");
    info = parse_testcase();
    if (info == NULL) {
        db_error("core: parse all test case from script failed\n");
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
