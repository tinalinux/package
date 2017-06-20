#ifndef AW_SER_DEV_H
#define AW_SER_DEV_H

typedef void* AWSERDEVHANDLE;
typedef AWSERDEVHANDLE* PAWSERDEVHANDLE;

/*
功能：
	初始化宿体插件，并返回插件实例。
参数：
    szError[out]---存放错误信息
    iLen[in]---szError大小，iLen不小于MAX_PATH
返回：
    初始化成功，返回插件句柄。否则，返回NULL
*/
AWSERDEVHANDLE AwsInitSerDevLibrary(char* szError, int iLen);


/*
功能：
	释放宿体插件句柄，并将pHandle置NULL。
参数：
	pHandle[in_out]---插件句柄
返回：
    无
*/
void AwsReleaseSerDevLibrary(PAWSERDEVHANDLE pHandle);


/*
功能：
    通知PC开始测试。
参数：
    szId[in]---msgid
    testNames[in]---所有测试项的名称
    iTestCount[in]---测试项名称的数量
返回：
    返回测试项的数量
*/
int AwsTestBegin(char* szId, char** testNames, int iTestCount);


/*
功能：
    通知PC测试已全部结束。
参数：
    szAuthor[in]---测试人员
    iPassed[in]---测试是否通过，1表示通过，0表示不通过
    iErrorCode[in]---错误号
    szError[in]---错误描述，该参数可为NULL
返回：
    返回测试项的数量
*/
int AwsTestFinish(char* szAuthor, int iPassed, int iErrorCode, char* szError);


/*
功能：
    通知设备端测试是否继续。在每次运行测试项前，必须调用本函数来判断是否继续测试。如果本函数返回0，则应该结束所有测试工作。
参数：
    无
返回：
    1---继续测试
    0---结束测试
*/
int AwsTestContinue();


/*
功能：
    通知PC开始运行某个测试项。
参数：
    iTestId[in]---测试项ID
    szTip[in]---提示信息，该参数可为NULL
返回：
    测试项ID
*/
int AwsTestBeginItem(int iTestId, char* szTip);


/*
功能：
    通知PC某个测试项已结束测试。
参数：
    iTestId[in]---测试项ID
    iPassed[in]---测试是否通过，1表示通过，0表示不通过
    iErrorCode[in]---错误号
    szError[in]---错误描述，该参数可为NULL
返回：
    测试项ID
*/
int AwsTestFinishItem(int iTestId, int iPassed, int iErrorCode, char* szError);


/*
功能：
    获取PC的操作响应。
参数：
    iTestId[in]---测试项ID
    iTimeOut[in]---等待PC响应的时间（毫秒），如果为-1则一直等待
    strTip[in]---提示信息，该参数可为NULL
    piResponse[in]---PC响应的结果，0表示超时未响应，1表示PASSED，2表示FAILED
返回：
    0---函数调用成功
    其他---函数调用失败
*/
int AwsTestResponse(int iTestId, int iTimeOut, const char* szTip, int* piResponse, char* szComment);
#endif
