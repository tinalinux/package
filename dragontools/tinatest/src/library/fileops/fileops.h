#ifndef __FILEOPS_H
#define __FILEOPS_H

#include <stdbool.h>
#include "log.h"

int cp(const char *from, const char *to);
int cp_fd(int from, int to);
int is_dir(const char *dir);
int is_existed(const char *path);
int mkdir_p(const char *dir);
int touch(const char *fpath);
unsigned long get_filesize(const char *fpath);

#endif
