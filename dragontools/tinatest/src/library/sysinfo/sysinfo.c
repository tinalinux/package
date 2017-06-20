#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SYSINFO_SH "/usr/bin/sysinfo"
#define SYSINFO_SAVE_PATH "/tmp/sys.info"
#define OUT_MALLOC_LEN 100

#define BASE_GET_FUNC(line) \
    char *ret = NULL; \
    if (access(SYSINFO_SAVE_PATH, R_OK)) { \
        if (access(SYSINFO_SH, X_OK) || init_sysinfo() != 0) \
            goto out; \
    } \
    FILE *fp; \
    if ((fp = go_line(line)) == NULL) \
        goto out; \
    char *output = calloc(1, OUT_MALLOC_LEN); \
    if (output == NULL) \
        goto out1; \
    if ((ret = fgets(output, OUT_MALLOC_LEN, fp)) == NULL) \
        goto out2; \
    goto out1; \
out2: \
    free(output); \
out1: \
    fclose(fp); \
out: \
    return ret;

enum line {
    kernel_version = 1,
    platform,
    target,
    boot_media
};

static FILE *go_line(enum line line)
{
    FILE *fp = fopen(SYSINFO_SAVE_PATH, "r");
    if (fp == NULL)
        return NULL;
    char *tmp = malloc(OUT_MALLOC_LEN);
    if (tmp == NULL) {
        fclose(fp);
        return NULL;
    }
    for(int num = 1; num < line; num++)
        fgets(tmp, OUT_MALLOC_LEN, fp);
    free(tmp);
    return fp;
}

int init_sysinfo()
{
    return system(SYSINFO_SH " " SYSINFO_SAVE_PATH);
}

char *get_kernel_version()
{
    BASE_GET_FUNC(kernel_version);
}

char *get_boot_media()
{
    BASE_GET_FUNC(boot_media);
}

char *get_platform()
{
    BASE_GET_FUNC(platform);
}

char *get_target()
{
    BASE_GET_FUNC(target);
}
