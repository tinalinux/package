#include <stdio.h>
#include <stdlib.h>
#include "awsostream.h"
#include "awserdev.h"
#include "AwsDevPlugin.h"
#include "GenSerialPortStream.h"
#include "awshelper.h"
using AwsNP::AwsHelper;

#define CFG_XML_FILE_PATH "/home/awtestcontrol_cfg.xml"

static AwsNP::AwsDevPlugin g_awsPlugin;
static AwsNP::StAwsTestCfg g_cfg;
static AwsNP::GenSerialPortStream g_sp;
static AwsNP::AwsOStream g_out;
static unsigned int g_msgid = 0;

AWSERDEVHANDLE AwsInitSerDevLibrary(char* szError, int iLen)
{
    if (!g_awsPlugin.HasBeenInitialized())
    {
        if (!AwsHelper::loadCfg(CFG_XML_FILE_PATH, g_cfg))
        {
            if (iLen >= MAX_PATH)
            {
                sprintf(szError, "Failed to load %s", CFG_XML_FILE_PATH);
            }

            //return NULL;

            AwsHelper::setDefaultConfig(g_cfg);
        }

        AwsNP::AwsOStream* pOStream = NULL;

        if (g_cfg.strSerialPortName == "__std")
        {
            pOStream = &g_out;
        }
        else
        {
            if (!g_sp.init(g_cfg.strSerialPortName.c_str()))
            {
                if (iLen >= MAX_PATH)
                {
                    sprintf(szError, "Failed to open %s", g_cfg.strSerialPortName.c_str());
                }

                return NULL;
            }

            pOStream = &g_sp;
        }

        int iRet = g_awsPlugin.init(*pOStream, g_cfg);

        if (iRet != 0)
        {
            if (iLen >= MAX_PATH)
            {
                sprintf(szError, "Failed to initialize. Error code: %d", iRet);
            }

            return NULL;
        }
    }

    return &g_awsPlugin;
}


void AwsReleaseSerDevLibrary(PAWSERDEVHANDLE pHandle)
{
    *pHandle = NULL;
}


int AwsTestBegin(char* szId, char** testNames, int iTestCount)
{
    CppVector<CppString> vecTestNames;

    for (int i = 0; i < iTestCount; i++)
    {
        vecTestNames.push_back(CppString(testNames[i]));
    }

    return g_awsPlugin.awsTestBegin(CppString(szId), vecTestNames);
}


int AwsTestFinish(char* szAuthor, int iPassed, int iErrorCode, char* szError)
{
    CppString strError;

    if (szError != NULL)
    {
        strError = szError;
    }

    time_t timep;
    struct tm* p;

    time(&timep);
    p = localtime(&timep);

    char szCreateTime[128];
    CppString strAuthor;

    if (szAuthor != NULL)
    {
        strAuthor = szAuthor;
    }

    sprintf(szCreateTime, "%d-%d-%d %d:%d:%d", p->tm_year + 1900, p->tm_mon + 1,
            p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

    g_awsPlugin.awsSaveLastTestResult(g_cfg.strFilesPath + "last_test_result.xml",
                                      strAuthor, szCreateTime);

    char szId[16];
    sprintf(szId, "%u", g_msgid);
    g_msgid++;

    return g_awsPlugin.awsTestFinish(szId, iPassed == 1, iErrorCode, strError);
}


int AwsTestContinue()
{
    return g_awsPlugin.awsTestContinue() ? 1 : 0;
}


int AwsTestBeginItem(int iTestId, char* szTip)
{
    CppString strTip;

    if (szTip != NULL)
    {
        strTip = szTip;
    }

    char szId[16];
    sprintf(szId, "%u", g_msgid);
    g_msgid++;

    return g_awsPlugin.awsTestBeginItem(szId, iTestId, strTip);
}


int AwsTestFinishItem(int iTestId, int iPassed, int iErrorCode, char* szError)
{
    CppString strError;

    if (szError != NULL)
    {
        strError = szError;
    }

    char szId[16];
    sprintf(szId, "%u", g_msgid);
    g_msgid++;

    return g_awsPlugin.awsTestFinishItem(szId, iTestId, iPassed == 1, iErrorCode, strError);
}



int AwsTestResponse(int iTestId, int iTimeOut, const char* szTip, int* piResponse, char* szComment)
{
    char szId[16];
    sprintf(szId, "%u", g_msgid);
    g_msgid++;

    CppString strTip = "";

    if (szTip != NULL)
    {
        strTip = szTip;
    }

    int iResponse = 0;
	CppString strComment = "";
    int iRet = g_awsPlugin.awsTestResponse(szId, iTestId, iTimeOut, strTip, iResponse, strComment);

    if (piResponse != NULL)
    {
        *piResponse = iResponse;
    }

	if (szComment != NULL)
	{
		strcpy(szComment, strComment.c_str());
	}

    return iRet;
}
