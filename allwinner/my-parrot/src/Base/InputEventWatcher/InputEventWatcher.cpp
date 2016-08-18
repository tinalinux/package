#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#include "InputEventWatcher.h"
#include "parrot_common.h"

#define PRESS_SYSFS_PATH   "/sys/class/input"

namespace Parrot{

InputEventWatcher::InputEventWatcher(ref_EventWatcher watcher, const char *name, EventWatcher::WatcherListener *l)
{
    mWatcher = watcher;
    char class_path[30];
    int ret = get_input_event_classpath(name,class_path);
    if(ret < 0){
        ploge("get %s class path failed!",name);
        throw; //TODO:test
    }
    int len = strlen(class_path);
    sprintf(class_path, "/dev/input/event%c", class_path[len - 1]);

    //open event
    int fd = open(class_path, O_RDONLY);
    if (fd < 0) {
        ploge("can not open device %s !(%s)",class_path,strerror(errno));
        throw; //TODO:test
    }
    plogd("add io watcher: %s",class_path);
    mID = mWatcher->addIOEventWatcher(fd, l, EventWatcher::IO_READ);
}

InputEventWatcher::~InputEventWatcher()
{
    mWatcher->deleteIOEventWatcher(mID);
    close(mID);
}

int InputEventWatcher::get_input_event_classpath(const char *name, char *class_path)
{
    char dirname[] = PRESS_SYSFS_PATH;
    char buf[256];
    int res;
    DIR *dir;
    struct dirent *de;
    int fd = -1;
    int found = 0;

    dir = opendir(dirname);
    if (dir == NULL)
        return -1;

    while((de = readdir(dir))) {
        if (strncmp(de->d_name, "input", strlen("input")) != 0) {
            continue;
        }

        sprintf(class_path, "%s/%s", dirname, de->d_name);
        snprintf(buf, sizeof(buf), "%s/name", class_path);

        fd = open(buf, O_RDONLY);
        if (fd < 0) {
            continue;
        }
        if ((res = read(fd, buf, sizeof(buf))) < 0) {
            close(fd);
            continue;
        }
        buf[res - 1] = '\0';
        if (strcmp(buf, name) == 0) {
            found = 1;
            close(fd);
            break;
        }

        close(fd);
        fd = -1;
    }
    closedir(dir);
    if (found) {
        return 0;
    }else {
        *class_path = '\0';
        return -1;
    }
}
}