/*
 * PostMsgPackage.h
 *
 *  Created on: 2016年5月18日
 *      Author: Administrator
 */

#ifndef PACKAGE_ALLWINNER_XIMALAYADEMO_SRC_POSTMSGPACKAGE_H_
#define PACKAGE_ALLWINNER_XIMALAYADEMO_SRC_POSTMSGPACKAGE_H_

#include <string>

using namespace std;

namespace allwinner
{

class PostMsgPackage
{
private:
    static string app_key;
    static string device_id;
    static string client_os_type;
    static string pack_id;
    static string access_token;
    static string app_secret;
    static int expires_in;
    static int gotTokenTime;

public:
    PostMsgPackage();
    virtual ~PostMsgPackage();

    virtual void setAccessToken(const string& accessToken);

    virtual bool isExpires();

    virtual int getExpiresIn() const;

    virtual void setExpiresIn(int expiresIn);

    virtual void setDeviceId(const string& deviceId);

    virtual string package(const string& strPost, bool isAccessToken = false);

    virtual void setAppKey(const string& appKey);

    virtual void setAppSecret(const string& appSecret);

    virtual void setClientOsType(const string& clientOsType);

    virtual void setPackId(const string& packId);
};

} /* namespace allwinner */

#endif /* PACKAGE_ALLWINNER_XIMALAYADEMO_SRC_POSTMSGPACKAGE_H_ */
