/*
 * XimalayaApi.h
 *
 *  Created on: 2016年5月27日
 *      Author: Administrator
 */

#ifndef PACKAGE_ALLWINNER_XIMALAYADEMO_SRC_XIMALAYAAPI_H_
#define PACKAGE_ALLWINNER_XIMALAYADEMO_SRC_XIMALAYAAPI_H_

#include <iostream>
#include "PostMsgPackage.h"

using namespace std;

namespace allwinner
{

class XimalayaApi
{
private:
    PostMsgPackage xmlyParam;
    virtual int getErrorCode(const string & strResponse);
    virtual bool CheckToken();
    virtual bool GetDataByIds(string & result, int & errorCode, const int * ids, const int idLen, const string apiInterface);

public:
    XimalayaApi();
    virtual ~XimalayaApi();

    /**设置公共参数
     * @param app_key 开放平台应用公钥 ，必填
     * @param device_id 设备唯一标识,可以是imei 或 MAC， 必填
     * @param client_os_type 客户端操作系统类型，1-IOS系统，2-Android系统，3-Web，4-Linux系统，5-ecos系统， 6-qnix系统， 必填
     * @param pack_id 客户端包名，对Android客户端是包名，对IOS客户端是Bundle ID， client_os_type为1或2时必填，其他选填
     * */
    virtual void setPublicParameters(const string & app_key, const string & device_id, const string & client_os_type,
            const string & pack_id = "");

    /**身份认证，获取token
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * */
    virtual bool SecureAccessToken(int & errorCode);

    /**根据分类和标签获取某个分类某个标签下的热门声音列表
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param category_id 分类ID，指定分类，为0时表示热门分类
     * @param tag_name 分类下对应的声音标签，不填则为热门分类
     * @param page 返回第几页，必须大于等于1，不填默认为1
     * @param count 每页多少条，默认20，最多不超过200
     * */
    virtual bool TracksHot(string & result, int & errorCode, const int category_id = 0, const string tag_name = "",
            const int page = 1, const int count = 20);

    /**批量获取声音
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param ids 声音ID列表，传参时用英文逗号分隔
     * @param idLen 传入的id数组的个数
     * */
    virtual bool TracksGetBatch(string & result, int & errorCode, const int * ids, const int idLen);

    /**批量获取专辑列表
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param ids 声音ID列表，传参时用英文逗号分隔
     * @param idLen 传入的id数组的个数
     * */
    virtual bool AlbumsGetBatch(string & result, int & errorCode, const int * ids, const int idLen);

    /**根据电台ID，批量获取电台列表
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param ids 声音ID列表，传参时用英文逗号分隔
     * @param idLen 传入的id数组的个数
     * */
    virtual bool LiveGetRadiosByIds(string & result, int & errorCode, const int * ids, const int idLen);

    /**获取直播电台
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param radio_type 电台类型：1-国家台，2-省市台，3-网络台
     * @param page 返回第几页，必须大于等于1，不填默认为1
     * @param count 每页多少条，默认20，最多不超过200
     * @param province_code 省份代码，radio_type为2时不能为空
     * */
    virtual bool LiveRadios(string & result, int & errorCode, const int radio_type, const int page = 1,
            const int count = 20, const int province_code = 0);

    /**获取直播电台排行榜
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param radio_count 需要获取排行榜中的电台数目
     * */
    virtual bool RanksRadios(string & result, int & errorCode, const int radio_count);

    /**按照关键词搜索直播
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param q 搜索查询词参数
     * @param page 返回第几页，必须大于等于1，不填默认为1
     * @param count 每页多少条，默认20，最多不超过200
     * */
    virtual bool SearchRadios(string & result, int & errorCode, const string q, const int page = 1,
            const int count = 20);

    /**搜索专辑
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param q 搜索查询词参数
     * @param page 返回第几页，必须大于等于1，不填默认为1
     * @param count 每页多少条，默认20，最多不超过200
     * @param category_id 分类ID，指定分类，为0时表示热门分类
     * */
    virtual bool SearchAlbums(string & result, int & errorCode, const string q, const int page = 1,
            const int count = 20, const int category_id = 0);

    /**根据专辑ID获取专辑下的声音列表，即专辑浏览
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param album_id 专辑ID
     * @param sort "asc"表示正序或"desc"表示倒序，默认为"asc"
     * @param page 返回第几页，必须大于等于1，不填默认为1
     * @param count 每页多少条，默认20，最多不超过200
     * */
    virtual bool AlbumsBrowse(string & result, int & errorCode, const int album_id, const string sort = "asc",
            const int page = 1, const int count = 20);

    /**搜索声音
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param q 搜索查询词参数
     * @param page 返回第几页，必须大于等于1，不填默认为1
     * @param count 每页多少条，默认20，最多不超过200
     * @param category_id 分类ID，指定分类，为0时表示热门分类
     * */
    virtual bool SearchTracks(string & result, int & errorCode, const string q, const int page = 1,
            const int count = 20, const int category_id = 0);

    /**上传单条直播播放数据
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param radio_id 必填，电台ID
     * @param duration 必填，播放时长，单位秒。即播放一个音频过程中，真正处于播放中状态的总时间。
     * @param played_secs 必填，播放第几秒或最后播放到的位置，是相对于这个音频开始位置的一个值。如果没有拖动播放条、快进、快退、暂停、单曲循环等操作，played_secs的值一般和duration一致
     * @param started_at 选填，播放开始时刻，Unix毫秒数时间戳
     * @param program_id 选填，节目ID
     * @param program_schedule_id 选填，节目排期ID
     * */
    virtual bool LiveSingleRecord(string & result, int & errorCode, const int radio_id, const double duration,
            const double played_secs, const long started_at = -1, const int program_id = -1,
            const int program_schedule_id = -1);

    /**上传单条直播播放数据
     * @param result 返回的结果
     * @param errorCode 返回的错误码， 没有错误时，返回-1
     * @param track_id 必填，声音ID
     * @param duration 必填，播放时长，单位秒。即播放一个音频过程中，真正处于播放中状态的总时间。
     * @param played_secs 必填，播放第几秒或最后播放到的位置，是相对于这个音频开始位置的一个值。如果没有拖动播放条、快进、快退、暂停、单曲循环等操作，played_secs的值一般和duration一致
     * @param play_type 必填，0：联网播放，1：断网播放
     * @param started_at 选填，播放开始时刻，Unix毫秒数时间戳
     * */
    virtual bool TrackSingleRecord(string & result, int & errorCode, const long track_id, const double duration,
            const double played_secs, const int play_type, const long started_at = -1);
};

} /* namespace allwinner */

#endif /* PACKAGE_ALLWINNER_XIMALAYADEMO_SRC_XIMALAYAAPI_H_ */
