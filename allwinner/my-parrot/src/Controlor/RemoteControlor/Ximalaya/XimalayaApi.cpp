/*
 * XimalayaApi.cpp
 *
 *  Created on: 2016年5月27日
 *      Author: Administrator
 */

#include "XimalayaApi.h"
//#include <json/reader.h>
//#include <json/value.h>
#include "CHttpClient.h"
#include <sstream>
#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

namespace allwinner
{

XimalayaApi::XimalayaApi()
{
    // TODO Auto-generated constructor stub

}

XimalayaApi::~XimalayaApi()
{
    // TODO Auto-generated destructor stub
}

bool XimalayaApi::CheckToken()
{
    if (xmlyParam.isExpires())
    {
        int errorCode;
        if (!SecureAccessToken(errorCode))
        {
            return false;
        }
    }

    return true;
}

bool XimalayaApi::SecureAccessToken(int & errorCode)
{
    CHttpClient cHttp;

    string strPost = xmlyParam.package("", true);

    string strResponse;
    cHttp.Post("http://api.ximalaya.com/oauth2/secure_access_token", strPost, strResponse);

    struct json_object *json = json_tokener_parse(strResponse.data());
    if (is_error(json))
    {
        return false;
    }

    struct json_object *error_no;

    if (!json_object_object_get_ex(json, "error_no", &error_no))
    {
        struct json_object *access_token;
        if (json_object_object_get_ex(json, "access_token", &access_token))
        {
            xmlyParam.setAccessToken(json_object_get_string(access_token));

            struct json_object *expires_in;
            if (json_object_object_get_ex(json, "expires_in", &expires_in))
            {
                xmlyParam.setExpiresIn(json_object_get_int(expires_in));

                cout << json_object_get_string(access_token) << endl;
                cout << xmlyParam.getExpiresIn() << endl;
                return true;
            }
        }
    }
    else
    {
        errorCode = json_object_get_int(error_no);
    }

    json_object_put(json);
    return false;
}

bool XimalayaApi::GetDataByIds(string & result, int & errorCode, const int * ids, const int idLen, const string apiInterface)
{
    CHttpClient cHttp;
    string strIds = "ids=";

    CheckToken();

    for (int i = 0; i < idLen; ++i)
    {
        stringstream ss;
        string stemp;
        ss << ids[i];
        ss >> stemp;
        strIds += stemp;
        if (i < (idLen - 1))
        {
            strIds += ",";
        }
    }

    string strPost = xmlyParam.package(strIds);

    string strResponse = "";
    cHttp.Get("http://api.ximalaya.com" + apiInterface + "?" + strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

bool XimalayaApi::TracksGetBatch(string & result, int & errorCode, const int * ids, const int idLen)
{
    return GetDataByIds(result, errorCode, ids, idLen, "/tracks/get_batch");
}

bool XimalayaApi::AlbumsGetBatch(string & result, int & errorCode, const int * ids, const int idLen)
{
    return GetDataByIds(result, errorCode, ids, idLen, "/albums/get_batch");
}

bool XimalayaApi::LiveGetRadiosByIds(string & result, int & errorCode, const int * ids, const int idLen)
{
    return GetDataByIds(result, errorCode, ids, idLen, "/live/get_radios_by_ids");
}

bool XimalayaApi::TracksHot(string & result, int & errorCode, const int category_id, const string tag_name,
        const int page, const int count)
{
    CHttpClient cHttp;

    char acRequst[1024];

    CheckToken();

    if (tag_name.empty())
    {
        sprintf(acRequst, "category_id=%d&page=%d&count=%d", category_id, page, count);

    }
    else
    {
        sprintf(acRequst, "category_id=%d&tag_name=%s&page=%d&count=%d", category_id, tag_name.c_str(), page, count);
    }
    string strRequst(acRequst);

    string strPost = xmlyParam.package(strRequst);

    string strResponse = "";
    cHttp.Get("http://api.ximalaya.com/tracks/hot?" + strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

bool XimalayaApi::LiveRadios(string & result, int & errorCode, const int radio_type, const int page, const int count,
        const int province_code)
{
    CHttpClient cHttp;

    char acRequst[1024];

    CheckToken();

    if (radio_type == 2)
    {
        sprintf(acRequst, "radio_type=%d&province_code=%d&page=%d&count=%d", radio_type, province_code, page, count);

    }
    else
    {
        sprintf(acRequst, "radio_type=%d&page=%d&count=%d", radio_type, page, count);
    }
    string strRequst(acRequst);

    string strPost = xmlyParam.package(strRequst);

    string strResponse = "";
    cHttp.Get("http://api.ximalaya.com/live/radios?" + strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

bool XimalayaApi::RanksRadios(string & result, int & errorCode, const int radio_count)
{
    CHttpClient cHttp;

    char acRequst[1024];

    CheckToken();

    sprintf(acRequst, "radio_count=%d", radio_count);

    string strRequst(acRequst);

    string strPost = xmlyParam.package(strRequst);

    string strResponse = "";
    cHttp.Get("http://api.ximalaya.com/ranks/radios?" + strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

bool XimalayaApi::SearchRadios(string & result, int & errorCode, const string q, const int page, const int count)
{
    CHttpClient cHttp;

    char acRequst[1024];

    CheckToken();

    sprintf(acRequst, "q=%s&page=%d&count=%d", q.c_str(), page, count);

    string strRequst(acRequst);

    string strPost = xmlyParam.package(strRequst);

    string strResponse = "";
    cHttp.Get("http://api.ximalaya.com/search/radios?" + strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

bool XimalayaApi::SearchAlbums(string & result, int & errorCode, const string q, const int page, const int count,
        const int category_id)
{
    CHttpClient cHttp;

    char acRequst[1024];

    CheckToken();

    sprintf(acRequst, "q=%s&category_id=%d&page=%d&count=%d", q.c_str(), category_id, page, count);

    string strRequst(acRequst);

    string strPost = xmlyParam.package(strRequst);

    string strResponse = "";
    cHttp.Get("http://api.ximalaya.com/search/albums?" + strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

bool XimalayaApi::AlbumsBrowse(string & result, int & errorCode, const int album_id, const string sort, const int page,
        const int count)
{
    CHttpClient cHttp;

    char acRequst[1024];

    CheckToken();

    sprintf(acRequst, "album_id=%d&sort=%s&page=%d&count=%d", album_id, sort.c_str(), page, count);

    string strRequst(acRequst);

    string strPost = xmlyParam.package(strRequst);

    string strResponse = "";
    cHttp.Get("http://api.ximalaya.com/albums/browse?" + strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

bool XimalayaApi::SearchTracks(string & result, int & errorCode, const string q, const int page, const int count,
        const int category_id)
{
    CHttpClient cHttp;

    char acRequst[1024];

    CheckToken();

    sprintf(acRequst, "q=%s&category_id=%d&page=%d&count=%d", q.c_str(), category_id, page, count);

    string strRequst(acRequst);

    string strPost = xmlyParam.package(strRequst);

    string strResponse = "";
    cHttp.Get("http://api.ximalaya.com/search/tracks?" + strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

void XimalayaApi::setPublicParameters(const string & app_key, const string & device_id, const string & client_os_type,
        const string & pack_id)
{
    xmlyParam.setAppKey(app_key);
    xmlyParam.setDeviceId(device_id);
    xmlyParam.setClientOsType(client_os_type);
    xmlyParam.setPackId(pack_id);
}

int XimalayaApi::getErrorCode(const string & strResponse)
{
    int errorCode = -1;

    struct json_object *json = json_tokener_parse(strResponse.data());
    if (!is_error(json))
    {
        struct json_object *error_no;

        if (json_object_object_get_ex(json, "error_no", &error_no))
        {
            errorCode = json_object_get_int(error_no);
        }
    }

    json_object_put(json);

    return errorCode;
}

bool XimalayaApi::LiveSingleRecord(string & result, int & errorCode, const int radio_id, const double duration,
        const double played_secs, const long started_at, const int program_id, const int program_schedule_id)
{
    CHttpClient cHttp;

    char acRequst[1024];

    int strCount = sprintf(acRequst, "radio_id=%d&duration=%.2f&played_secs=%.2f", radio_id, duration, played_secs);

    if (started_at != -1)
    {
        strCount += sprintf(acRequst + strCount, "&started_at=%ld", started_at);
    }

    if (program_id != -1)
    {
        strCount += sprintf(acRequst + strCount, "&program_id=%d", program_id);
    }

    if (program_schedule_id != -1)
    {
        strCount += sprintf(acRequst + strCount, "&program_schedule_id=%d", program_schedule_id);
    }


    string strRequst(acRequst);

    string strPost = xmlyParam.package(strRequst);

    string strResponse;
    cHttp.Post("http://api.ximalaya.com/openapi-collector-app/live_single_record", strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

bool XimalayaApi::TrackSingleRecord(string & result, int & errorCode, const long track_id, const double duration,
        const double played_secs, const int play_type, const long started_at)
{
    CHttpClient cHttp;

    char acRequst[1024];

    int strCount = sprintf(acRequst, "track_id=%ld&duration=%.2f&played_secs=%.2f&play_type=%d", track_id, duration,
            played_secs, play_type);

    if (started_at != -1)
    {
        strCount += sprintf(acRequst + strCount, "&started_at=%ld", started_at);
    }

    string strRequst(acRequst);

    string strPost = xmlyParam.package(strRequst);

    string strResponse;
    cHttp.Post("http://api.ximalaya.com/openapi-collector-app/track_single_record", strPost, strResponse);

    result = strResponse;
    errorCode = getErrorCode(strResponse);

    if (errorCode == -1)
    {
        return true;
    }

    return false;
}

} /* namespace allwinner */
