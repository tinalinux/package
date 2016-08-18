#include "parrot_common.h"
#include "UserManager.h"

namespace Parrot{

int UserManager::addUser(string id, int bindtype)
{
    User *user;
    list<User*>::iterator it;
    for(it = mUserList.begin(); it != mUserList.end(); it++){
        if((*it)->id == id){
            (*it)->bindtype = bindtype;
            return SUCCESS;
        }
    }

    user = new User();
    user->bindtype = bindtype;
    user->id = id;
    user->lanfd = -1;
    mUserList.push_back(user);
    return SUCCESS;
}

User *UserManager::getUserbyId(string id)
{
    User *user;
    list<User*>::iterator it;
    for(it = mUserList.begin(); it != mUserList.end(); it++){
        if((*it)->id == id){
            return *it;
        }
    }
    return NULL;
}

User *UserManager::getUserbyLanfd(int fd)
{
    User *user;
    list<User*>::iterator it;
    for(it = mUserList.begin(); it != mUserList.end(); it++){
        if((*it)->lanfd == fd){
            return *it;
        }
    }
    return NULL;
}

int UserManager::removeUser(string id)
{
    list<User*>::iterator it;
    for(it = mUserList.begin(); it != mUserList.end(); it++){
        if((*it)->id == id){
            User *user = *it;
            if(user != NULL) delete user;
            mUserList.erase(it);
            return SUCCESS;
        }
    }
}

int UserManager::removeUser(User *user)
{
    return removeUser(user->id);
}

int UserManager::setLanUser(string id, int fd)
{
    User *user = getUserbyId(id);
    if(user == NULL) return FAILURE;

    user->lanfd = fd;
    return SUCCESS;
}

int UserManager::unsetLanUser(int fd)
{
    User *user = getUserbyLanfd(fd);
    if(user == NULL) return FAILURE;

    user->lanfd = -1;
    return SUCCESS;
}

}