#ifndef __STATE_MANAGET_H__
#define __STATE_MANAGET_H__
#include <locker.h>
#include <stdio.h>

namespace Parrot{

typedef struct StateString
{
    int state;
    const char *string;
}tStateString;
class StateManager{
public:
    StateManager(int size,const char* name,tStateString *map = NULL, int map_len = 0);
    ~StateManager();
    int update(int state);
    int get(int pos);
    int reset(int state);

    static const int CUR = 0;
    static const int PRE = 1;

private:
    const char* get_state_string(int state);
private:
    int m_size;
    int m_pos;
    int* m_stack;
    locker m_lock;
    tStateString *m_state_map;
    int m_state_map_len;
    char *m_name;

};
}/* namespace Parrot */
#endif /*__STATE_MANAGET_H__*/