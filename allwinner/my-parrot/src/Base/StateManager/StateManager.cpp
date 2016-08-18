#define TAG "StateManager"
#include <tina_log.h>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "StateManager.h"
#include "parrot_common.h"

const static char* default_name = "default";
namespace Parrot{

StateManager::StateManager(int size,const char *name, tStateString *map, int map_len){
    m_size = size;
    m_pos = 0;
    m_stack = new int[size];
    //init stop
    for(int i = 0; i < m_size; i++)
        m_stack[i] = -1;

    m_state_map = map;
    m_state_map_len = map_len;

    int name_len = strlen(name);
    if(name_len > 0){
        m_name = new char[strlen(name)];
        strcpy(m_name,name);
    }else{
        m_name = new char[1];
        *m_name = 32;
    }

}

StateManager::~StateManager(){
    m_size = 0;
    m_pos = 0;
    DELETE_POINTER(m_stack);
    DELETE_POINTER(m_name);
}
int StateManager::update(int state){
    locker::autolock l(m_lock);

    m_pos = (m_pos+1) == m_size?0:m_pos+1;
    m_stack[m_pos] = state;
    if(m_state_map == NULL)
        plogd("%s STATE: pos: %d  pre: 0x%x --> 0x%x",m_name,m_pos,get(PRE),get(CUR));
    else
        plogd("%s STATE: pos: %d  pre: %s --> %s",m_name,m_pos,get_state_string(get(PRE)),get_state_string(get(CUR)));
#if 1
    for(int i = 0; i < m_size; i++){
        if(m_state_map == NULL)
            plogd("%s STATE dump %s %d: 0x%x", m_name, i==m_pos?"->":"  ", i, m_stack[i]);
        else
            plogd("%s STATE dump %s %d: %s", m_name, i==m_pos?"->":"  ", i, get_state_string(m_stack[i]));
    }
#endif
    return SUCCESS;
}
int StateManager::get(int pos){
    //locker::autolock l(m_lock);
    if(pos > m_size) {
        ploge("get state faid, pos: %d, max_size: %d",pos,m_size);
        return FAILURE;
    }
    //TLOGD("get: m_pos:%d, pos: %d  %d  %d\n",m_pos,pos,(m_pos + m_size - pos)%m_size,m_stack[(m_pos + m_size - pos)%m_size]);

    return m_stack[(m_pos + m_size - pos)%m_size];
}

const char* StateManager::get_state_string(int state)
{
    int index = 0;
    for(index = 0; index < m_state_map_len; index++){
        if(m_state_map[index].state == state)
            return m_state_map[index].string;
    }
    return NULL;
}
int StateManager::reset(int state)
{
    for(int i = 0; i < m_size; i++)
        m_stack[i] = state;
}
}/* namespace Parrot */