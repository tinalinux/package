/*
 * ResponseJsonApi.cpp
 *
 *  Created on: 2016年5月30日
 *      Author: Administrator
 */


#include "ResponseJsonApi.h"
//#include <json/reader.h>
//#include <json/value.h>
#include <json-c/json.h>

namespace allwinner
{

ResponseJsonApi::ResponseJsonApi(const string& strJson)
{
    // TODO Auto-generated constructor stub
    setStrJson(strJson);
}

ResponseJsonApi::~ResponseJsonApi()
{
    // TODO Auto-generated destructor stub
}

const string& ResponseJsonApi::getStrJson() const
{
    return strJson;
}

void ResponseJsonApi::setStrJson(const string& strJson)
{
    this->strJson = strJson;
}

bool ResponseJsonApi::isError()
{
    struct json_object *json = json_tokener_parse(this->strJson.data());
    if (!is_error(json))
    {
        struct json_object *error_no;

        if (json_object_object_get_ex(json, "error_no", &error_no))
        {
            return true;
        }
    }

    json_object_put(json);

    return false;
/*
    Json::Reader reader;
    Json::Value value;

    if (reader.parse(strJson, value))
    {
        if (!value.isArray() && !value["error_no"].isNull())
        {
            return true;
        }
    }
    return false;
*/

}

bool ResponseJsonApi::getErrorMsg(int & error_no, string & error_code, string & error_desc, string & service)
{
    if (!this->isError())
    {
        return false;
    }

    struct json_object *json = json_tokener_parse(this->strJson.data());
    if (!is_error(json))
    {
        struct json_object *jError_no;
        if (json_object_object_get_ex(json, "error_no", &jError_no))
        {
            error_no = json_object_get_int(jError_no);
        }

        struct json_object *jError_code;
        if (json_object_object_get_ex(json, "error_code", &jError_code))
        {
            error_code = json_object_get_string(jError_code);
        }

        struct json_object *jError_desc;
        if (json_object_object_get_ex(json, "error_desc", &jError_desc))
        {
            error_desc = json_object_get_string(jError_desc);
        }

        struct json_object *jService;
        if (json_object_object_get_ex(json, "service", &jService))
        {
            service = json_object_get_string(jService);
        }
    }

    json_object_put(json);

    return true;
/*
    Json::Reader reader;
    Json::Value value;

    if (reader.parse(strJson, value))
    {
        error_no = value["error_no"].asInt();
        error_code = value["error_code"].asString();
        error_desc = value["error_desc"].asString();

        if (!value["service"].isNull())
        {
            service = value["service"].asString();
        }
        else
        {
            service = "";
        }
    }
*/
    return true;
}

} /* namespace allwinner */
