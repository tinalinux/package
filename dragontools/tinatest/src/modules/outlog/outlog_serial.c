#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "outlog.h"
#include "fileops.h"

static int module_before_all(struct list_head *TASK_LIST)
{
    struct task *task;
    printf("\n");
    printf("============================ tasks list ============================\n");
    list_for_each_entry(task, TASK_LIST, lnode)
        printf("%s\n", task->keypath);
    printf("============================    end     ============================\n");
    return 0;
}

static int module_after_one_end(struct task *task)
{
    struct tm *time;
    printf("\n");
    printf("---------------------------- %s ----------------------------\n", task->keypath);
    printf("* task path : %s\n", task->keypath);
    printf("* task command : %s\n", task->command);
    printf("* run times(real/max) : %d/%d\n",
            task->run_times, task->env.limit.run_times);
    printf("* run parallel : %s\n", task->env.limit.run_parallel == true ? "yes" : "no");
    printf("* run alone : %s\n", task->env.limit.run_alone == true ? "yes" : "no");
    printf("* may reboot : %s\n", task->env.limit.may_reboot == true ? "yes" : "no");
    if (task->env.limit.may_reboot == true)
        printf("* reboot/run times : %d/%d\n", task->rebooted_times, task->run_times);
    if (task->env.info.date == true) {
        printf("* begin date : %s", ctime(&task->begin_date));
        printf("* end date : %s", ctime(&task->end_date));
    }
    printf("* result :\n");
    printf("*     num    pid    pgid    return     begin       end    note\n");
    for (int num = 0; num < task->run_times; num++) {
        printf("* %7d%7d%8d%10d",
                num, -task->pid[num],
                task->pgid[num] > 0 ? task->pgid[num] : -task->pgid[num], task->result[num]);
        time = gmtime(&task->begin_time[num]);
        printf("  %2.2d:%2.2d:%2.2d", time->tm_hour, time->tm_min, time->tm_sec);
        time = gmtime(&task->end_time[num]);
        printf("  %2.2d:%2.2d:%2.2d", time->tm_hour, time->tm_min, time->tm_sec);
        if (task->kill_by_signal)
            printf("    kill by signal %s(%d)\n",
                    strsignal(task->kill_by_signal), task->kill_by_signal);
        else
            printf("\n");
    }
    if (task->env.info.resource == true
            && task->res != NULL) {
        printf("* task resource :\n");
        printf("*     user cpu time = %ld.%ld\n",
                task->res->ru_utime.tv_sec, task->res->ru_utime.tv_usec/1000);
        printf("*     system cpu time = %ld.%ld\n",
                task->res->ru_stime.tv_sec, task->res->ru_stime.tv_usec/1000);
        printf("*     maximum resident size = %ldkB\n", task->res->ru_maxrss);
        printf("*     page faults break times (without I/O) = %ld\n", task->res->ru_minflt);
        printf("*     page faults break times (with I/O) = %ld\n", task->res->ru_majflt);
        printf("*     input times = %ld\n", task->res->ru_inblock);
        printf("*     output times = %ld\n", task->res->ru_oublock);
        printf("*     wait resource actively times = %ld\n", task->res->ru_nvcsw);
        printf("*     wait resource passively times = %ld\n", task->res->ru_nivcsw);
    }

    // log
    int fd = open(task->logpath, O_RDONLY);
    if (fd < 0) {
        ERROR("%s open %s failed\n", task->keypath, task->logpath);
        return -1;
    }

    printf("* run log :\n");
    printf("****************\n");
    cp_fd(fd, STDOUT_FILENO);
    printf("---------------------------- end ----------------------------\n");
    return 0;
}

static int module_after_all(struct list_head *TASK_LIST)
{
    struct task *task;
    int result = 0;
    int num = 0;
    printf("\n");
    printf("============================ tasks result ============================\n");
    list_for_each_entry(task, TASK_LIST, lnode) {
        for (num = 0; num < task->run_times; num++) {
            if (task->result[num] != 0) {
                result = task->result[num];
                break;
            }
        }
        printf("%s - ", task->keypath);
        if (result != 0)
            printf("NO (failed in %d times with %d back)\n", num, task->result[num]);
        else
            printf("YES\n");
    }
    printf("============================     end      ============================\n");
    return 0;
}

void module_init(void) {
    outlog_register(
            module_before_all,
            NULL,
            module_after_one_end,
            module_after_all);
}
