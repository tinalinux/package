#ifndef __USER_MANAGER_H__
#define __USER_MANAGER_H__

#include <string>
#include <list>
using namespace std;
namespace Parrot{

class User
{
public:
    string id;
    int bindtype;   //0: 永久绑定 可能存在lan、lanfd，1: 临时绑定，必定存在lan、lanfd

    int lanfd; // -1 不可用， >0 可用
};

class UserManager
{
public:

    int addUser(string id, int bindtpye);
    User *getUserbyId(string id);
    User *getUserbyLanfd(int fd);

    int removeUser(User *user);
    int removeUser(string id);

    int setLanUser(string id, int fd);
    int unsetLanUser(int fd);

private:
    list<User*> mUserList;

};
}


#endif /*__USER_MANAGER_H__*/