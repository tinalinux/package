#ifndef __INPUT_EVENT_WATCHER_H__
#define __INPUT_EVENT_WATCHER_H__
#include "EventWatcher.h"
namespace Parrot{

class InputEventWatcher
{
public:
    InputEventWatcher(ref_EventWatcher watcher, const char *name, EventWatcher::WatcherListener *l);
    ~InputEventWatcher();

private:
    int get_input_event_classpath(const char *name, char *class_path);

private:
    ref_EventWatcher mWatcher;
    int mID;

};

}/*namespace Parrot*/

#endif/*__INPUT_EVENT_WATCHER_H__*/