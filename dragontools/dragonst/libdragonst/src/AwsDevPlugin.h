#ifndef AWS_DEV_PLUGIN_H
#define AWS_DEV_PLUGIN_H

#include "cppvector.h"
#include "awsostream.h"
#include "GeneralFuncs.h"
#include "awshelper.h"

namespace AwsNP
{
    typedef struct
    {
        CppString strTestName;
        CppString strResult;
        CppString strComment;
    }StAwsTestItem;

    typedef CppVector<StAwsTestItem> VecAwsTestItems;

    class AwsDevPlugin
    {
    public:
        AwsDevPlugin();
        virtual ~AwsDevPlugin();

    public:
        static unsigned long THREADROUTINE serviceRoutineUL(void* pParam);
        static void* serviceRoutineV(void* pParam);

    public:
        virtual int init(AwsOStream& output, const StAwsTestCfg& cfg);
        virtual void release();
        bool HasBeenInitialized();

        virtual int awsTestBegin(const CppString& strId, const CppVector<CppString>& vecTestNames);
        virtual int awsTestFinish(const CppString& strId, bool bPassed, int iErrorCode, const CppString& strError);
        virtual bool awsTestContinue() const;

        virtual int awsTestBeginItem(const CppString& strId, int iTestId, const CppString& strTip);
        virtual int awsTestFinishItem(const CppString& strId, int iTestId, bool bPassed, int iErrorCode, const CppString& strError);

        virtual int awsSaveLastTestResult(const CppString& strLastResultFilePath, const CppString& strAuthor, const CppString& strCreateTime);
        virtual int awsTestResponse(const CppString& strId, int iTestId, int iTimeOut, const CppString& strTip, int& iResponse, CppString& strError);

    protected:
        bool initSem(key_t semKey);
        void releaseSem();

        bool initShm(key_t shmKey);
        void releaseShm();

    protected:
        bool m_bHasBeenInitialized;
        bool m_bNeedServiceWorking;
        bool m_bNeedTestContinue;

        AwsOStream* m_pOutput;
        VecAwsTestItems m_vecTestItems;
        THREADHANDLE m_serviceThreadHandle;

        StAwsTestCfg m_cfg;
        int m_iHeartbeatFreqMillisecond;

        int m_semId;
        int m_shmId;

        StShared* m_pShared;
    };
}

#endif
