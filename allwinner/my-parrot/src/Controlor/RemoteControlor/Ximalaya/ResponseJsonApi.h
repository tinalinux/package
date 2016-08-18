/*
 * ResponseJsonApi.h
 *
 *  Created on: 2016年5月30日
 *      Author: Administrator
 */

#ifndef PACKAGE_ALLWINNER_XIMALAYADEMO_SRC_RESPONSEJSONAPI_H_
#define PACKAGE_ALLWINNER_XIMALAYADEMO_SRC_RESPONSEJSONAPI_H_

#include <string>
#include <iostream>

using namespace std;

namespace allwinner
{

class ResponseJsonApi
{
private:
    string strJson;

public:
    ResponseJsonApi(const string& strJson);
    virtual ~ResponseJsonApi();

    virtual const string& getStrJson() const;

    virtual void setStrJson(const string& strJson);

    virtual bool isError();

    virtual bool getErrorMsg(int & error_no, string & error_code, string & error_desc, string & service);
};

} /* namespace allwinner */

#endif /* PACKAGE_ALLWINNER_XIMALAYADEMO_SRC_RESPONSEJSONAPI_H_ */
