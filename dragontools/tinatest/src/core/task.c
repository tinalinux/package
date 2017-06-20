#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "mjson.h"
#include "mcollectd.h"
#include "syskey.h"
#include "sync.h"
#include "task.h"
#include "fileops.h"
#include "outlog.h"

int reboot_mode;        //标志重启模式
int reboot_run_times;   //重启的测试用例执行次数
int rebooted_times;     //重启的测试用例已经重启次数
char *reboot_keypath;   //引起重启的测试用例路径
char *tt_argv;          //调用tinatest的输入参数
char *fstdout, *fstderr;//标准输出/错误的文件

volatile int running_cnt; //记录正在执行的测试用例个数
volatile int end_cnt;  //记录已执行的测试用例个数
int tasks_cnt;   //记录有效任务个数
static pthread_mutex_t running_cnt_lock; //修改正在执行个数的变量锁
static pthread_mutex_t do_task_lock;     //执行测试用例的锁

struct global_env g_env; //全局环境
struct local_env l_env;  //局部环境(只有默认和/sys/local的配置,不包含测试用例定制的部分)

static struct list_head TASK_LIST; //任务列表

#define ADD 1
#define SUB 0

// *========================================
// *功能: 从/proc中获取stdout/stderr的文件路径
// *参数: STDOUT_FILENO or STDERR_FILENO
// *返回: 文件路径字符串
// *========================================
char *task_get_fstd(int fd)
{
    char *buf = NULL, *link = NULL;
    struct stat st;
    int len;
    if ((buf = malloc(100)) == NULL) {
        ERROR("malloc for fstd failed\n");
        goto out1;
    }
    snprintf(buf, 100, "/proc/%d/fd/%d", getpid(), fd);
    if (lstat(buf, &st) == -1) {
        ERROR("lstat for %s failed\n", buf);
        goto out1;
    }
    link = malloc(st.st_size + 1);
    if (link == NULL) {
        ERROR("malloc for fstd failed\n");
        goto out1;
    }
    len = readlink(buf, link, st.st_size + 1);
    if (len == -1) {
        ERROR("readlink %s failed\n", buf);
        goto out1;
    }
    if (len > st.st_size) {
        ERROR("%s symlink increased in size between lstat() and readlink()", buf);
        goto out1;
    }
    link[len] = '\0';
    goto out;

out1:
    free(link);
    link = NULL;
out:
    free(buf);
    return link;
}

// *========================================
// *功能: 执行加载/sys/global
// *参数: genv : 保存结果的变量
// *      key  : 配置项
// *      val  : 配置值
// *      type : 配置类型
// *返回: 0: 失败
// *      1: 成功
// *========================================
int task_load_global_env_do(
        struct global_env *genv,
        char *key,
        union mdata val,
        enum mjson_type type)
{
    //limit
    if (!strcmp(key, SYSKEY_GLOBAL_LIMIT_RUN_CNT_UP_TO) && type == mjson_type_int) {
        DEBUG(DATA, "limit: run_cnt_up_to: %d => %d\n",
                genv->limit.run_cnt_up_to,
                val.m_int > 0 ? val.m_int : genv->limit.run_cnt_up_to);
        genv->limit.run_cnt_up_to =
            val.m_int > 0 ? val.m_int : genv->limit.run_cnt_up_to;
    }
    else
        return 0;

    return 1;
}

// *========================================
// *功能: 加载/sys/global(调用 task_load_global_env_do)
// *参数: genv : 保存结果的变量
// *      keypath : 全局配置项的路径
// *返回: 加载成功的配置项数量
// *========================================
int task_load_global_env(struct global_env *genv, const char *keypath)
{
    int ret = 0;
    DEBUG(BASE, "Loading global env\n");
    //global limit
    mjson_foreach(keypath, info_key, limit_val, limit_mtype)
        ret += task_load_global_env_do(genv, info_key, limit_val, limit_mtype);

    //global info
    return ret;
}

// *========================================
// *功能: 执行加载/sys/local
// *参数: genv : 保存结果的变量
// *      key  : 配置项
// *      val  : 配置值
// *      type : 配置类型
// *返回: 0: 失败
// *      1: 成功
// *========================================
int task_load_local_env_do(
        struct local_env *lenv,
        char *key,
        union mdata val,
        enum mjson_type type)
{
    //info
    if (!strcmp(key, SYSKEY_LOCAL_INFO_DATE) && type == mjson_type_boolean) {
        DEBUG(DATA, "info: date: %d => %d\n", lenv->info.date, val.m_boolean);
        lenv->info.date = val.m_boolean;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_INFO_RESOURCE) && type == mjson_type_boolean) {
        DEBUG(DATA, "info: resource: %d => %d\n", lenv->info.resource, val.m_boolean);
        lenv->info.resource = val.m_boolean;
    }
    //limit
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_RUN_TIMES) && type == mjson_type_int) {
        DEBUG(DATA, "limit: run_times: %d => %d\n",
                lenv->limit.run_times,
                val.m_int > 0 ? val.m_int : lenv->limit.run_times);
        lenv->limit.run_times =
            val.m_int > 0 ? val.m_int : lenv->limit.run_times;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_RUN_ALONE) && type == mjson_type_boolean) {
        DEBUG(DATA, "limit: run_alone: %d => %d\n",
                lenv->limit.run_alone, val.m_boolean);
        lenv->limit.run_alone = val.m_boolean;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_MAY_REBOOT)
            && type == mjson_type_boolean) {
        DEBUG(DATA, "limit: may_reboot: %d => %d\n",
                lenv->limit.may_reboot, val.m_boolean);
        lenv->limit.may_reboot = val.m_boolean;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_RUN_PARALLEL)
            && type == mjson_type_boolean) {
        DEBUG(DATA, "limit: run_parallel: %d => %d\n",
                lenv->limit.run_parallel, val.m_boolean);
        lenv->limit.run_parallel = val.m_boolean;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_RUN_TIME_SEC) && type == mjson_type_int) {
        DEBUG(DATA, "limit: run_time_sec: %d => %d\n", lenv->limit.run_time_sec, val.m_int);
        lenv->limit.run_time_sec = val.m_int;
    }
    else if (!strcmp(key, SYSKEY_LOCAL_LIMIT_RUN_TIME_MIN) && type == mjson_type_int) {
        DEBUG(DATA, "limit: run_time_sec: %d => %d\n",
                lenv->limit.run_time_sec, lenv->limit.run_time_sec + val.m_int * 60);
        lenv->limit.run_time_sec += val.m_int * 60;
    }
    else if (!strcmp(key,SYSKEY_LOCAL_LIMIT_RUN_TIME_HOUR) && type == mjson_type_int) {
        DEBUG(DATA, "limit: run_time_sec: %d => %d\n",
                lenv->limit.run_time_sec, lenv->limit.run_time_sec + val.m_int * 60 * 60);
        lenv->limit.run_time_sec += val.m_int * 60 * 60;
    }
    else
        return 0;
    return 1;
}

// *========================================
// *功能: 加载/sys/global(调用 task_load_local_env_do)
// *参数: genv : 保存结果的变量
// *      keypath : 全局配置项的路径
// *返回: 加载成功的配置项数量
// *========================================
int task_load_local_env(struct local_env *lenv, const char *keypath)
{
    int ret = 0;
    if (lenv == NULL || keypath == NULL)
        return ret;
    mjson_foreach(keypath, info_key, info_val, info_mtype)
        ret += task_load_local_env_do(lenv, info_key, info_val, info_mtype);
    return ret;
}

// *========================================
// *功能: 加载/sys, 无配置则采用默认值
// *参数:
// *返回:
// *========================================
void task_init_sysenv(void)
{
    memset(&l_env, 0 , sizeof(struct local_env));
    memset(&g_env, 0 , sizeof(struct global_env));
    //default env
    g_env.limit.run_cnt_up_to = DEFAULT_GLOBAL_LIMIG_RUN_CNT_UP_TO;
    DEBUG(DATA, "env: set run_cnt_up_to to default %d\n",
            DEFAULT_GLOBAL_LIMIG_RUN_CNT_UP_TO);
    l_env.limit.run_times = DEFAULT_LOCAL_LIMIT_RUN_TIMES;
    DEBUG(DATA, "env: set run_times to default %d\n", DEFAULT_LOCAL_LIMIT_RUN_TIMES);
    l_env.limit.run_time_sec = DEFAULT_LOCAL_RUN_TIME_SEC;
    DEBUG(DATA, "env: set run_time_sec to default %d\n", DEFAULT_LOCAL_RUN_TIME_SEC);
    l_env.info.date = DEFAULT_LOCAL_INFO_DATE;
    DEBUG(DATA, "env: set date to default %d\n", DEFAULT_LOCAL_INFO_DATE);
    l_env.info.resource = DEFAULT_LOCAL_INFO_RESOURCE;
    DEBUG(DATA, "env: set resource to default %d\n", DEFAULT_LOCAL_INFO_RESOURCE);
    //load sys env
    DEBUG(BASE, "env: load %s\n", MJSON_LOCAL_ENV_LIMIT);
    task_load_local_env(&l_env, MJSON_LOCAL_ENV_LIMIT);
    DEBUG(BASE, "env: load %s\n", MJSON_LOCAL_ENV_INFO);
    task_load_local_env(&l_env, MJSON_LOCAL_ENV_INFO);
    DEBUG(BASE, "env: load %s\n", MJSON_GLOBAL_ENV_LIMIT);
    task_load_global_env(&g_env, MJSON_GLOBAL_ENV_LIMIT);
    //init collectd (sys/global/info)
    g_env.info.collectd =
        mjson_fetch_boolean(COLLECTD_ENABLE) == true ? true : false;
    DEBUG(DATA, "env: set collectd enable to %d\n", g_env.info.collectd);
    // init outlog (sys/global/info)
    g_env.info.outlog =
        mjson_fetch_boolean(OUTLOG_ENABLE) == false ? false : true;
    DEBUG(DATA, "env: set outlog enable to %d\n", g_env.info.collectd);
}

// *========================================
// *功能: 清除重启的所有缓存
// *参数:
// *返回:
// *注意: 会被新进程调用,由于此程序时多线程多进程应用,所以此函数只能使用线程安全函数
// *========================================
void task_clean_reboot(void)
{
    unlink(REBOOT_INIT);
    unlink(REBOOT_INIT_LINK);
    unlink(REBOOT_LOG);
    unlink(REBOOT_STATUS);
    reboot_mode = 0;
    reboot_run_times = 0;
    rebooted_times = 0;
    reboot_keypath = NULL;
}

// *========================================
// *功能: 创建重启脚本文件
// *参数:
// *返回: 0: 成功 ; 1: 失败
// *========================================
int task_create_fboot(void)
{
    int ret = -1;
    if (!access(REBOOT_INIT, F_OK) || !access(REBOOT_INIT_LINK, F_OK)) {
        remove(REBOOT_INIT);
        remove(REBOOT_INIT_LINK);
    }
    FILE *fp = fopen(REBOOT_INIT, "w");
    if (fp == NULL) {
        ERROR("open %s failed\n", REBOOT_INIT);
        goto out;
    }
    setbuf(fp, NULL);
    fprintf(fp, "#!/bin/sh /etc/rc.common\n");
    fprintf(fp, "\n");
    fprintf(fp, "START=99\n");
    fprintf(fp, "DEPEND=fstab\n");
    fprintf(fp, "\n");
    fprintf(fp, "start() {\n");
    fprintf(fp, "    tinatest -r %s %s\n", reboot_keypath, tt_argv);
    fprintf(fp, "}\n");
    fprintf(fp, "\n");
    fprintf(fp, "stop() {\n");
    fprintf(fp, "    return 0\n");
    fprintf(fp, "}\n");

    DEBUG(DATA, "reboot call : %s\n", tt_argv);
    fflush(fp);

    chmod(REBOOT_INIT, 0755);

    if (symlink(REBOOT_INIT, REBOOT_INIT_LINK) != 0) {
        ERROR("open %s failed\n", REBOOT_INIT);
        goto out;
    }

    ret = 0;
out:
    fclose(fp);
    return ret;
}

// *========================================
// *功能: 初始化
// *参数:
// *返回: -1: 失败 ; -127: 无需stdin ; >0: 文件描述符
// *注意: 会被新进程调用,由于此程序时多线程多进程应用,所以此函数只能使用线程安全函数
// *========================================
int task_init_fdin(struct task *task)
{
    int ret = -1;
    if (task->input_t == TASK_STDIN) {
        char **pp = (char **)task->input;
        if (pp == NULL)
            goto out;
        int len_array = atoi(pp[0]);
        int fd[2] = {-1, -1};

        if (pipe(fd))
            goto out;
        for (int cnt = 1; cnt <= len_array; cnt++) {
            write(fd[1], pp[cnt], strlen(pp[cnt]));
            write(fd[1], "\n", 1);
        }
        close(fd[1]);
        ret = fd[0];
    } else if (task->input_t == TASK_FSTDIN) {
        char *p = (char *)task->input;
        if (p == NULL)
            goto out;
        if (!access(p, R_OK)) {
            int fd_f = open(p, O_RDONLY);
            if (fd_f < 0)
                goto out; //open failed
            ret = fd_f;
        }
    } else {
        ret = -127; //not -1 but less than 0 means no need to set stdin
        goto out;
    }

out:
    return ret;
}

// *========================================
// *功能: 执行添加测试用例的核心部分
// *参数: keypath: 测试用例路径
// *      command: 调用测试用例的命令
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_add_do(const char *keypath, const char *command)
{
    int ret = -1, len = strlen(keypath) + 30;
    char *p = NULL, **pp = NULL, *buf = malloc(len);
    if (buf == NULL) {
        ERROR("%s: malloc failed when add task to list - %s\n",
                keypath, strerror(errno));
        goto out;
    }
    struct task *new = malloc(sizeof(struct task));
    if (new == NULL) {
        ERROR("%s: malloc failed when add task to list - %s\n",
                keypath, strerror(errno));
        goto out;
    }
    memset(new, 0 , sizeof(struct task));

    //keypath
    if ((new->keypath = malloc(strlen(keypath) + 1)) == NULL) {
        ERROR("%s: malloc failed when add task to list - %s\n",
                keypath, strerror(errno));
        goto out1;
    }
    memcpy(new->keypath, keypath, strlen(keypath) + 1);
    //task_env
    memcpy(&new->env, &l_env, sizeof(struct local_env)); //base on sysenv
    task_load_local_env(&new->env, keypath); //and load local env
    //command
    new->command = (char *)command;
    //*result
    new->result = calloc(new->env.limit.run_times, sizeof(int));
    if (new->result == NULL) {
        ERROR("%s: malloc failed when add task to list - %s\n",
                keypath, strerror(errno));
        goto out1;
    }
    //*pid
    new->pid = calloc(new->env.limit.run_times, sizeof(pid_t));
    if (new->pid == NULL) {
        ERROR("%s: malloc failed when add task to list - %s\n",
                keypath, strerror(errno));
        goto out1;
    }
    //*pgid
    new->pgid = calloc(new->env.limit.run_times, sizeof(pid_t));
    if (new->pgid == NULL) {
        ERROR("%s: malloc failed when add task to list - %s\n",
                keypath, strerror(errno));
        goto out1;
    }
    //*begin_time && *end_time
    new->begin_time = calloc(new->env.limit.run_times, sizeof(time_t));
    if (new->begin_time == NULL) {
        ERROR("%s: malloc failed when add task to list - %s\n",
                keypath, strerror(errno));
        goto out1;
    }
    new->end_time = calloc(new->env.limit.run_times, sizeof(time_t));
    if (new->end_time == NULL) {
        ERROR("%s: malloc failed when add task to list - %s\n",
                keypath, strerror(errno));
        goto out1;
    }
    //may_reboot
    if (new->env.limit.may_reboot == true)
        new->env.limit.run_alone = true;
    //logpath
    snprintf(buf, len, "%s", keypath);
    p = buf;
    while ((p = strchr(p, '/')) != NULL)
        *p = '-';
    new->logpath = calloc(1, len);
    if (new->logpath == NULL) {
        ERROR("%s: malloc failed when add task to list - %s\n",
                keypath, strerror(errno));
        goto out1;
    }
    if (new->env.limit.may_reboot == true)
        snprintf(new->logpath, len, "%s", REBOOT_LOG);
    else
        snprintf(new->logpath, len, "/tmp/tt%s.out", buf);
    DEBUG(BASE, "%s: logpath is %s\n", new->keypath, new->logpath);
    //resource
    if (new->env.info.resource == true) {
        new->res = malloc(sizeof(struct rusage));
        if (new->res == NULL) {
            ERROR("%s malloc for resource failed - %s\n",
                    new->keypath, strerror(errno));
            goto out1;
        }
        memset(new->res, 0, sizeof(struct rusage));
    }
    //input
    snprintf(buf, len, "%s/%s", new->keypath, SYSKEY_TASK_STDIN);
    if ((pp = mjson_fetch_array(buf)) != NULL) {
        new->input = pp;
        new->input_t = TASK_STDIN;
    } else {
        snprintf(buf, len, "%s/%s", new->keypath, SYSKEY_TASK_FSTDIN);
        if ((p = mjson_fetch_string(buf)) != NULL) {
            new->input = p;
            new->input_t = TASK_FSTDIN;
        } else {
            new->input = NULL;
            new->input_t = TASK_NO_INPUT;
        }
    }
    //status
    new->status = TASK_WAIT;
    //lnode
    list_add_tail(&new->lnode, &TASK_LIST);

    ret = 0;
    goto out;

out1:
    free(new->result);
    free(new->pid);
    free(new->logpath);
    free(new->res);
    free(new);
out:
    free(buf);
    return ret;
}

// *========================================
// *功能: 递归遍历所有节点,判断路径节点是否有效,调用_task_add_do添加
// *参数: keypath: 测试用例路径
// *返回: 添加测试用例个数
// *========================================
int task_add(const char *keypath)
{
    int len = strlen(keypath)+ 30;
    char *buf = malloc(len);
    int ret = 0;

    if (*keypath != '/') {
        ERROR("%s is not absolute path, skip it\n", keypath);
        goto out;
    }
    if (mjson_fetch(keypath).type == mjson_type_error) {
        ERROR("%s: fetch failed, skip it\n", keypath);
        goto out;
    }

    if (!strcmp(keypath, "/"))
        snprintf(buf, len, "/%s", SYSKEY_TASK_ENABLE);
    else
        snprintf(buf, len, "%s/%s", keypath, SYSKEY_TASK_ENABLE);
    if (mjson_fetch_boolean(buf) == FAILED){ //disable
        DEBUG(BASE, "%s is disable, skip it and it's childs\n", buf);
        goto out;
    }

    if (!strcmp(keypath, "/"))
        snprintf(buf, len, "/%s", SYSKEY_TASK_COMMAND);
    else
        snprintf(buf, len, "%s/%s", keypath, SYSKEY_TASK_COMMAND);

    char *command = NULL;
    if ((command = mjson_fetch_string(buf)) != NULL) { //find it
        struct task *task;
        list_for_each_entry(task, &TASK_LIST, lnode) {
            if (!strcmp(task->keypath, keypath))
                goto out;
        }
        if (reboot_mode == 1 &&
                TASK_LIST.next == &TASK_LIST &&
                TASK_LIST.prev == &TASK_LIST) {
            if (strcmp(reboot_keypath, keypath)) {
                DEBUG(BASE, "in reboot mode, waiting %s not %s, skip it\n", reboot_keypath, keypath);
                goto out;
            }
        }
        DEBUG(BASE, "fetch %s, try to add it to list\n", keypath);
        if (task_add_do(keypath, command) == 0)
            ret = 1;
    } else {
        mjson_foreach(keypath, key, val, mtype) {
            if (mtype == mjson_type_object) {
                if (!strcmp(keypath, "/"))
                    snprintf(buf, len, "/%s", key);
                else
                    snprintf(buf, len, "%s/%s", keypath, key);
                ret += task_add(buf);
            }
        }
    }

out:
    free(buf);
    return ret;
}

// *========================================
// *功能: 与task_add_do对应,清空指定测试用例申请的资源
// *参数: task: 测试用例
// *返回:
// *========================================
void task_del_do(struct task *task)
{
    remove(task->logpath);
    free(task->pid);
    task->pid = NULL;
    free(task->pgid);
    task->pgid = NULL;
    free(task->begin_time);
    task->begin_time = NULL;
    free(task->end_time);
    task->end_time = NULL;
    free(task->logpath);
    task->logpath = NULL;
    free(task->result);
    task->result = NULL;
    free(task->res);
    task->res = NULL;
    free(task->keypath);
    list_del(&(task->lnode));
    free(task);
}

// *========================================
// *功能: 清空路径下所有测试用例申请的资源 (调用task_del_do)
// *参数: 测试用例的路径
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_del(const char *keypath)
{
    if (TASK_LIST.prev == NULL && TASK_LIST.next == NULL)
        return -1;

    struct task *pre = NULL, *next = NULL;
    list_for_each_entry_safe(pre, next, &TASK_LIST, lnode) {
        if (!strcmp(pre->keypath, keypath))
            task_del_do(pre);
    }
    return 0;
}

// *========================================
// *功能: 修改变量running_cnt/end_cnt
// *参数: ADD & SUB
// *返回:
// *========================================
void task_mod_running_cnt(int num)
{
    pthread_mutex_lock(&running_cnt_lock);
    switch (num) {
        case ADD: {
                running_cnt++;
                DEBUG(BASE, "add running_cnt\n");
                DEBUG(DATA, "running_cnt: %d\n", running_cnt);
                break;
            }
        case SUB: {
                end_cnt++;
                running_cnt--;
                DEBUG(BASE, "sub running_cnt\n");
                DEBUG(DATA, "running_cnt: %d\n", running_cnt);
                DEBUG(DATA, "runned_cnt: %d\n", end_cnt);
                break;
            }
        default: break;
    }
    pthread_mutex_unlock(&running_cnt_lock);
}

// *========================================
// *功能: 测试用例为may_reboot,则调用此函数初始化重启相关的东西
// *参数: task: 测试用例
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_init_reboot_mode(struct task *task)
{
    FILE *fp = NULL;
    //only option -r will set reboot_mode
    //if reboot_mode is set, it means it had rebooted before or had run demo once at least
    if (reboot_mode == 1) { //old task (may reboot)
        DEBUG(DATA, "%s: task.run_times: %d => %d\n",
                task->keypath, task->run_times, reboot_run_times + 1);
        task->run_times = ++reboot_run_times;
        DEBUG(DATA, "%s: task.rebooted_times: %d => %d\n",
                task->keypath, task->rebooted_times, rebooted_times);
        task->rebooted_times = rebooted_times;
    } else { //new task (may reboot)
        DEBUG(BASE, "%s may reboot and is the first time to run\n", task->keypath);
        reboot_keypath = task->keypath;
        reboot_run_times = task->run_times;
        reboot_mode = 1;
        if (task_create_fboot() != 0) {
            ERROR("create %s(%s) failed, skip it\n", REBOOT_INIT, REBOOT_INIT_LINK);
            goto out1;
        }
    }
    //refresh or new write to REBOOT_STATUS
    if ((fp = fopen(REBOOT_STATUS, "w")) == NULL) {
        ERROR("%s: open failed, skip it\n", REBOOT_STATUS);
        goto out1;
    }
    DEBUG(BASE, "%s: write to %s - run times: %d; reboot times: %d; "
            "fstdout: %s; fstderr: %s\n", task->keypath, REBOOT_STATUS,
            task->run_times, task->rebooted_times, fstdout, fstderr);
    if (fprintf(fp, "%d\n%d\n%s\n%s",
                task->run_times, task->rebooted_times, fstdout, fstderr) < 0 ) {
        ERROR("%s: write %s failed, skip it\n",task->keypath, REBOOT_STATUS);
        goto out1;
    }
    fsync(fileno(fp));
    fclose(fp);

    return 0;
out1:
    fclose(fp);
    task_clean_reboot();
    return -1;
}

// *========================================
// *功能: 创建新进程作为测试用例的控制进程
// *参数: task: 测试用例
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_do_limit(struct task *task)
{
    if ((task->pgid[task->run_times] = fork()) < 0) {
        ERROR("%s: fork for limit failed - %s\n", task->keypath, strerror(errno));
        goto err;
    } else if (task->pgid[task->run_times] == 0) { //child
        // multi thread and multi process
        // !!! should only use signal-safe functions
        // !!! CAN'T use the series of printf
        // !!! see more signal-safe functions by : man 7 signal
        setpgid(getpid(), getpid());
        sleep(task->env.limit.run_time_sec);
        if (kill(-getpid(), SIGALRM) != 0)
        _exit(0);
    }
    setpgid(task->pgid[task->run_times] > 0 ?
            task->pgid[task->run_times] : -task->pgid[task->run_times],
            task->pgid[task->run_times] > 0 ?
            task->pgid[task->run_times] : -task->pgid[task->run_times]);
    DEBUG(DATA, "%s: pgid: 0 => %d\n", task->keypath,
            task->pgid[task->run_times] > 0 ?
            task->pgid[task->run_times] : -task->pgid[task->run_times]);

    return 0;
err:
    return -1;
}

// *========================================
// *功能: 创建新进程执行测试用例
// *参数: task: 测试用例
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_do_testcase(struct task *task)
{
    DEBUG(BASE, "%s: begin to run ...\n", task->keypath);
    do {
        //record time everytime run testcase
        task->begin_time[task->run_times] = time(NULL);
        if ((task->pid[task->run_times] = fork()) < 0) {
            kill(task->pgid[task->run_times] < 0 ?
                    task->pgid[task->run_times] : -task->pgid[task->run_times], SIGKILL);
            ERROR("%s: fork for testcase failed - %s\n", task->keypath, strerror(errno));
            goto err;
        } else if (task->pid[task->run_times] == 0) { //child
            // multi thread and multi process
            // !!! should only use signal-safe functions
            // !!! CAN'T use the series of printf
            // !!! see more signal-safe functions by : man 7 signal
            setpgid(getpid(), task->pgid[task->run_times] > 0 ?
                task->pgid[task->run_times] : -task->pgid[task->run_times]);

            //stdin
            int fin = task_init_fdin(task);
            if (fin == -1)
                _exit(127);

            //stdout
            int fout =  open(task->logpath, O_WRONLY | O_APPEND | O_CREAT, 00644);
            if (fout < 0)
                _exit(127);

            if (dup2(fout, STDOUT_FILENO) != STDOUT_FILENO)
                _exit(127);

            if (dup2(fout, STDERR_FILENO) != STDERR_FILENO)
                _exit(127);

            if (fin > 0 && dup2(fin, STDIN_FILENO) != STDIN_FILENO)
                _exit(127);

            execl("/bin/sh", "/bin/sh", "-c", task->command, task->command, NULL);
            _exit(127);
        }
        setpgid(task->pid[task->run_times] > 0 ?
                task->pid[task->run_times] : -task->pid[task->run_times],
                task->pgid[task->run_times] > 0 ?
                task->pgid[task->run_times] : -task->pgid[task->run_times]);
        task->run_times++;
        if (task->env.limit.run_parallel == true &&
                task->run_times < task->env.limit.run_times)
            task->pgid[task->run_times] = task->pgid[task->run_times - 1] > 0 ?
                task->pgid[task->run_times - 1] : -task->pgid[task->run_times - 1];
    }while(task->env.limit.run_parallel == true &&
            task->run_times < task->env.limit.run_times);

    return 0;
err:
    return -1;
}

// *========================================
// *功能: 创建新进程执行测试用例(调用 task_do_limit / task_do_testcase)
// *参数: task: 测试用例
// *返回: 0: 成功 ; -1: 失败
// *========================================
int task_do(struct task *task)
{
    int ret = -1;
    pthread_mutex_lock(&do_task_lock);
    if (task->env.limit.may_reboot == true) {
        DEBUG(BASE, "%s may reboot\n", task->keypath);
        if (task_init_reboot_mode(task) < 0) {
            reboot_mode = 0;
            ERROR("%s: init for reboot mode failed\n", task->keypath);
            goto out;
        }
        DEBUG(BASE, "%s: init for reboot mode successfully\n", task->keypath);
    }
    //the last times to run may-reboot task
    if (task->run_times >= task->env.limit.run_times)
        goto out;

    //save begin date
    if (task->env.info.date == true && task->begin_date == 0) {
        task->begin_date = time(NULL);
        DEBUG(DATA, "%s: begin date 0 => %s\n",
                task->keypath, ctime(&task->begin_date));
    }

    //run limit
    if (task_do_limit(task) < 0)
        goto out;

    //run testcase
    if (task_do_testcase(task) < 0)
        goto out;

    DEBUG(DATA, "%s: run_times: %d => %d\n",
            task->keypath, task->run_times - 1, task->run_times);
    DEBUG(DATA, "%s: pid[%d]: 0 => %d\n",
            task->keypath, task->run_times - 1, task->pid[task->run_times - 1]);
    DEBUG(BASE, "%s: run successfully (%d/%d)\n",
            task->keypath, task->run_times, task->env.limit.run_times);

    // call outlog func
    if (g_env.info.outlog == true)
        outlog_call_after_one_begin(task);

    ret = 0;
out:
    pthread_mutex_unlock(&do_task_lock);
    return ret;
}

// *========================================
// *功能: 获取测试用例使用的资源
// *参数: task: 测试用例
// *      res: 资源
// *返回:
// *========================================
void task_get_rusage(struct task *task, struct rusage *res)
{
    task->res->ru_utime.tv_sec += res->ru_utime.tv_sec;
    task->res->ru_utime.tv_usec += res->ru_utime.tv_usec;
    task->res->ru_stime.tv_sec += res->ru_stime.tv_sec;
    task->res->ru_stime.tv_usec += res->ru_stime.tv_usec;
    task->res->ru_maxrss = (res->ru_maxrss + task->res->ru_maxrss) / 2;
    task->res->ru_minflt += res->ru_minflt;
    task->res->ru_majflt += res->ru_majflt;
    task->res->ru_inblock += res->ru_inblock;
    task->res->ru_oublock += res->ru_oublock;
    task->res->ru_nvcsw += res->ru_nvcsw;
    task->res->ru_nivcsw += res->ru_nivcsw;
}

// *========================================
// *功能: 测试用例结束所有次数后的回收
// *参数: task: 测试用例
// *返回:
// *========================================
void task_recycle_end(struct task *task)
{
    DEBUG(BASE, "%s: recycling\n", task->keypath);
    if (task->env.info.date == true) {
        task->end_date = time(NULL);
        DEBUG(DATA, "%s: end time : %s\n", task->keypath, ctime(&task->end_date));
    }
    // call outlog
    if (g_env.info.outlog == true)
        outlog_call_after_one_end(task);
    if (reboot_mode == true && task->env.limit.may_reboot == true) {
        DEBUG(BASE, "%s: may reboot and has done, to clean reboot file\n",
                task->keypath);
        task_clean_reboot();
        if (running_cnt > 0) {
            DEBUG(BASE, "%s: it may reboot, but it hadn't reboot before\n",
                    task->keypath);
            task_mod_running_cnt(SUB);
        }
    }
    else
        task_mod_running_cnt(SUB);
    if (running_cnt + 1 == g_env.limit.run_cnt_up_to)
        TELL_PARENT(TASK_ADD_RECYCLE_ID);
    task->status = TASK_END;
}

// *========================================
// *功能: 测试用例结束一次后的回收
// *参数: task: 测试用例
// *      num: 第几次执行
// *      status: 返回的状态
// *      pid: 执行的pid
// *      res: 测试用例资源
// *返回:
// *========================================
void task_recycle_once(struct task *task, int num, int status, int pid, struct rusage *res)
{
    // <0 means finish
    task->pid[num] = -pid;
    // get usage resource
    task_get_rusage(task, res);
    // get end time everytime end once testcase
    task->end_time[num] = time(NULL);
    // get exit return
    if (WIFEXITED(status))
        task->result[num] = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) {
        task->result[num] = -1;
        task->kill_by_signal = WTERMSIG(status);
        DEBUG(BASE, "%s: killed by signal %s(%d)\n", task->keypath,
                strsignal(task->kill_by_signal), task->kill_by_signal);
    }
    else {
        ERROR("%s: Invail EXIT\n", task->keypath);
        task->result[num] = -1;
    }
    DEBUG(BASE, "%s: %d times exit with %d\n",
            task->keypath, num, task->result[num]);
    // kill other members in group
    if (kill(task->pgid[num] < 0 ? task->pgid[num] : -task->pgid[num], SIGKILL) != 0)
        DEBUG(BASE, "%s: kill pgid %d failed, maybe it died before - "
                "%s\n", task->keypath, task->pgid[num] < 0 ?
                task->pgid[num] : -task->pgid[num], strerror(errno));
}

// *========================================
// *功能: 检查测试用例是否结束(执行完所有次数)
// *参数: task: 测试用例
// *返回: 1: 结束 ; 0: 没结束
// *========================================
int task_check_end(struct task *task)
{
    // if parallel, check whether finish all
    if (task->env.limit.run_parallel == true) {
        int tmp;
        DEBUG(BASE, "%s: run parallel, check whether finish all\n",
                task->keypath);
        for (tmp = 0; tmp < task->run_times; tmp++) {
            if (task->pid[tmp] > 0)
                break;
        }
        if (tmp != task->run_times) {
            DEBUG(BASE, "%s: pid[%d] - %d havn't end\n",
                    task->keypath, tmp, task->pid[tmp]);
            return false;
        }
        DEBUG(BASE, "%s: run parallel, finish all\n", task->keypath);
    }
    // check whether end
    if (task->run_times < task->env.limit.run_times)
        return false;
    return true;
}

// *========================================
// *功能: 守护线程,回收死亡的子进程(调用task_recycle_once/task_recycle_end)
// *参数:
// *返回:
// *========================================
void *task_recycle(void *arg)
{
    struct rusage res;
    int status;
    pid_t pid;
    struct task *pre = NULL, *next = NULL;

    DEBUG(BASE, "recycle pthread begin\n");
    if (TELL_PARENT(TASK_ADD_RECYCLE_ID) < 0) {
        ERROR("reply to parent failed\n");
        exit(1);
    }

    while (true) {
        if (end_cnt == tasks_cnt)
            goto out;

        pid = wait4(-1, &status, 0, &res);
        if (pid == -1 && errno == ECHILD) {
            sleep(1);
            continue;
        }
        DEBUG(BASE, "get death of %d\n", pid);
        // walk for all tasks
        list_for_each_entry_safe(pre, next, &TASK_LIST, lnode) {
            if (pre->status != TASK_RUN) {
                DEBUG(BASE, "%s isn't running with status(%s)\n",
                        pre->keypath, pre->status == TASK_END ? "END" : "WAIT");
                continue;
            }
            // check in every times
            char match = false;
            for (int num = 0; num < pre->run_times; num++) {
                DEBUG(BASE, "To match pid - %s: pgid[%d] is %d, pid[%d] is %d\n",
                        pre->keypath, num, pre->pgid[num], num, pre->pid[num]);
                // match pgid
                if (pre->pgid[num] == pid) {
                    match = true;
                    DEBUG(BASE, "match death of %s's limit\n", pre->keypath);
                    pre->pgid[num] = -pid;
                    break;
                // match pid
                } else if (pid == pre->pid[num]) {
                    match = true;
                    DEBUG(BASE, "match death of %s(pid:%d)\n", pre->keypath, pid);
                    // recycle once
                    task_recycle_once(pre, num, status, pid, &res);
                    // call outlog
                    if (g_env.info.outlog == true)
                        outlog_call_after_one_once(pre);
                    // check end
                    if (task_check_end(pre) == true) {
                        task_recycle_end(pre);
                    } else if (pre->env.limit.run_parallel != true) {
                        DEBUG(BASE, "%s: not enough, do again - %d/%d\n",
                                pre->keypath, pre->run_times, pre->env.limit.run_times);
                        if (task_do(pre) != 0)
                            ERROR("%s: do again failed\n", pre->keypath);
                    }
                    break;
                }
            }
            if (match == true)
                break;
        }
    }

out:
    if (g_env.info.outlog == true)
        outlog_call_after_all(&TASK_LIST);
    DEBUG(BASE, "recycle pthread exit(task cnt: %d)\n", running_cnt);
    pthread_exit(0);
}

// *========================================
// *功能: 中止信号处理
// *参数: sig: 信号编号
// *返回:
// *========================================
void task_signal_quit(int sig)
{
    struct task *task;
    list_for_each_entry(task, &TASK_LIST, lnode) {
        if (task->status != TASK_RUN)
            continue;
        for (int num = 0; num < task->run_times; num++) {
            if (task->pid[num] > 0) {
                kill(task->pgid[num] < 0 ? task->pgid[num] : -task->pgid[num], sig);
                remove(task->logpath);
            }
        }
    }
    task_clean_reboot();
    if (g_env.info.collectd == true)
        mcollectd_exit();
    _exit(sig);
}

// *========================================
// *功能: 开始执行前的初始化
// *参数:
// *返回:
// *========================================
int task_begin_init(void)
{
    //get stdout/stderr
    if (fstdout == NULL) {
        fstdout = task_get_fstd(STDOUT_FILENO);
        DEBUG(DATA, "stdout is %s\n", fstdout);
    }
    if (fstderr == NULL) {
        fstderr = task_get_fstd(STDERR_FILENO);
        DEBUG(DATA, "stderr is %s\n", fstderr);
    }

    //init lock for running_cnt
    running_cnt = 0;
    pthread_mutex_init(&running_cnt_lock, NULL);
    pthread_mutex_init(&do_task_lock, NULL);

    // init synchronous communication between father process and child process
    if (TELL_WAIT(TASK_ADD_RECYCLE_ID) < 0) {
        ERROR("init sync failed - %s\n", strerror(errno));
        goto err;
    }

    // init collectd
    if (g_env.info.collectd == true) {
        if (mcollectd_load_json() == 0) {
            if (TELL_WAIT(COLLECTD_DO_ID) < 0) {
                ERROR("init sync failed - %s\n", strerror(errno));
                goto err;
            }
            //run global env (collectd)
            // collectd will be call by mcollectd_do
            // so, if reboot before, it should no create conf once again
            if (reboot_mode == 0) {
                if (mcollectd_make_conf() < 0) {
                    ERROR("create collectd conf failed\n");
                    goto err;
                }
            }
            if (mcollectd_do() < 0) {
                ERROR("do collectd failed\n");
                goto err;
            }
            DEBUG(BASE, "start callectd successfully\n");
        } else {
            g_env.info.collectd = false;
        }
    }

    // init outlog
    if (g_env.info.outlog == true && outlog_init() < 0) {
        ERROR("init outlog failed\n");
        goto err;
    }

    // catch signal
    if (SIG_ERR == signal(SIGTERM, task_signal_quit))
        goto err;
    if (SIG_ERR == signal(SIGQUIT, task_signal_quit))
        goto err;
    if (SIG_ERR == signal(SIGINT, task_signal_quit))
        goto err;

    return 0;
err:
    return -1;
}

int task_begin(void)
{
    // initialize before begin
    if (task_begin_init() < 0)
        goto out;

    // recycle child tasks
    pthread_t rtid;
    if (pthread_create(&rtid, NULL, task_recycle, NULL) != 0) {
        ERROR("create recycle thread failed - %s\n", strerror(errno));
        goto out;
    }
    if (WAIT_CHILD(TASK_ADD_RECYCLE_ID) < 0) {
        ERROR("wait recycle child reply failed\n");
        goto out;
    }
    DEBUG(BASE, "start recycle pthread successfully\n");

    //run task
    DEBUG(BASE, "All is READY!! Begin to run tasks in list\n");
    DEBUG(BASE, "=========================================\n");
    if (g_env.info.outlog == true)
        outlog_call_before_all(&TASK_LIST);
    struct task *task;
    list_for_each_entry(task, &TASK_LIST, lnode) {
        DEBUG(BASE, "In list node %s\n", task->keypath);
        while (running_cnt >= g_env.limit.run_cnt_up_to) {
            DEBUG(BASE, "up to max tasks running cnt (cur/max) - %d/%d, wait\n",
                    running_cnt, g_env.limit.run_cnt_up_to);
            WAIT_CHILD(TASK_ADD_RECYCLE_ID);
        }
        while (task->env.limit.run_alone == true && running_cnt > 0) {
            DEBUG(BASE, "%s wants to run alone(alive tasks %d), sleep 1s for wait\n",
                    task->keypath, running_cnt);
            sleep(1);
        }

        if (reboot_mode == 1 && strcmp(reboot_keypath, task->keypath)) {
            DEBUG(BASE, "%s: reboot mode and not this task reboot before, skip it\n",
                    task->keypath);
            continue;
        }

        task->status = TASK_RUN;
        if (g_env.info.outlog == true)
            outlog_call_before_one(task);
        if (task_do(task) == 0) {
            DEBUG(BASE, "start %s successfully\n", task->keypath);
            task_mod_running_cnt(ADD);
        }
        //the last time to run task which had reboot before
        else if (reboot_mode == 1 && task->run_times >= task->env.limit.run_times) {
            DEBUG(BASE, "%s: may reboot and has done the last time\n",
                    task->keypath);
            task->result[task->run_times - 1] = 0;
            task_recycle_end(task);
        }
        else {
            ERROR("%s: do task failed, skip it\n", task->keypath);
            task->result[task->run_times] = -1;
            task->status = TASK_END;
            continue;
        }

        //wait for end of task which want to run alone and not the last testcase
        while (task->env.limit.run_alone == true
                && running_cnt > 0
                && end_cnt + 1 != tasks_cnt) {
            DEBUG(BASE, "%s wants to run alone, so, do next before it finish, wait\n",
                    task->keypath);
            sleep(1);
        }
    }
    DEBUG(BASE, "all tasks have been added\n");
    pthread_join(rtid, NULL);
    if (g_env.info.collectd == true)
        mcollectd_exit();
    list_for_each_entry(task, &TASK_LIST, lnode)
        remove(task->logpath);
    return 0;

out:
    return 1;
}


int task_recover_after_reboot(char *keypath)
{
    int ret = -1;
    FILE *fp = NULL;
    int fd = -1;
    char *line = NULL;
    size_t len = 0;
    if (is_existed(REBOOT_STATUS) == true) { //existed
        if ((fp = fopen(REBOOT_STATUS, "r")) == NULL) {
            ERROR("open %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        }
        // recover reboot_run_times
        if (getline(&line, &len, fp) == -1) {
            ERROR("read %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        }
        reboot_run_times = atoi(line);

        // recover rebooted_times
        if (getline(&line, &len, fp) == -1) {
            ERROR("read %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        }
        rebooted_times = atoi(line) + 1;

        int bytes = len = 0;
        fstdout = fstderr = NULL;
        // recover fstdout
        if ((bytes = getline(&fstdout, &len, fp)) == -1) {
            ERROR("read %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        } else {
            if (fstdout[bytes - 1] == '\n')
                fstdout[bytes - 1] = '\0';
        }

        // recover fstderr
        if ((bytes = getline(&fstderr, &len, fp)) == -1) {
            ERROR("read %s failed - %s\n", REBOOT_STATUS, strerror(errno));
            goto err;
        } else {
            if (fstderr[bytes - 1] == '\n')
                fstderr[bytes - 1] = '\0';
        }

        DEBUG(DATA, "In %s : run times %d; rebooted times %d; "
                "fstdout is %s; fstderr is %s\n",
                REBOOT_STATUS, reboot_run_times, rebooted_times, fstdout, fstderr);

        // recover stdout
        if ((fd = open(fstdout, O_WRONLY | O_APPEND)) < 0) {
            ERROR("open %s for stdout failed - %s\n", fstdout, strerror(errno));
            goto err;
        }
        if (dup2(fd, STDOUT_FILENO) != STDOUT_FILENO) {
            ERROR("dup %s as stdout failed - %s\n", fstdout, strerror(errno));
            close(fd);
            goto err;
        }
        close(fd);
        setbuf(stdout, NULL);
        DEBUG(BASE, "redirect stdout to %s successfully\n", fstdout);

        // recover stdout
        if ((fd = open(fstderr, O_WRONLY | O_APPEND)) < 0) {
            ERROR("open %s for stdout failed - %s\n", fstderr, strerror(errno));
            goto err;
        }
        if (dup2(fd, STDERR_FILENO) != STDERR_FILENO) {
            ERROR("dup %s to stderr failed - %s\n", fstderr, strerror(errno));
            goto err;
        }
        close(fd);
        setbuf(stdout, NULL);
        DEBUG(BASE, "redirect stderr to %s successfully\n", fstderr);
    } else { //noexisted
        ERROR("%s: missing %s\n", reboot_keypath, REBOOT_STATUS);
        goto err;
    }

    ret = 0;
err:
    if (ret == -1) {
        free(fstdout);
        free(fstderr);
    }
    free(line);
    fclose(fp);
    return ret;
}

int task_init(int argc, char **argv)
{
    // init global variables
    reboot_mode = false;
    reboot_run_times = 0;
    rebooted_times = 0;
    reboot_keypath = NULL;
    tt_argv = NULL;
    fstdout = fstderr = NULL;
    running_cnt = 0;
    end_cnt = 0;
    tasks_cnt = 0;
    INIT_LIST_HEAD(&TASK_LIST);

    task_init_sysenv();

    // add tasks
    if ((tt_argv = malloc(1)) == NULL) {
        ERROR("malloc for tasks_argv failed\n");
        return -1;
    }
    *tt_argv = '\0';
    if (argc == 0)
        tasks_cnt = task_add("/");
    else {
        for (int num = 0, len = 0; num < argc; num++) {
            len = strlen(argv[num]) + strlen(tt_argv);
            tt_argv = realloc(tt_argv,  len + 1);
            tt_argv = strcat(tt_argv, argv[num]);
            tt_argv[len] = ' ';
            tt_argv[len + 1] = '\0';
            if ((tasks_cnt += task_add(argv[num])) == 0)
                ERROR("No any ACTIVE testcase in %s\n", argv[num]);
            else
                DEBUG(BASE, "Found %d ACTIVE testcases in %s\n", tasks_cnt, argv[num]);
        }
        tt_argv[strlen(tt_argv) - 1] = '\0';
        DEBUG(DATA, "tinatest argv is |%s|\n", tt_argv);
    }
    DEBUG(DATA, "cnt of active testcase : %d\n", tasks_cnt);

    return tasks_cnt;
}
