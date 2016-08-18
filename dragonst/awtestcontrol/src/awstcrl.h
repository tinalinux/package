#ifndef AWSTCRL_H
#define AWSTCRL_H

#include "awshelper.h"

namespace AwsNP
{
    typedef struct
    {
        CppString strOption; // 命令选项
        CppString strParam; // 命令参数
    }StOptionParam;

    typedef CppVector<StOptionParam> VecOptionParams;

    typedef struct
    {
        CppString strId;
        StOptionParam mainOP;
        VecOptionParams vecSubOpps;
    }StAwtestcontrolArguments;

    class Awtestcontrol
    {
    public:
        Awtestcontrol();
        virtual ~Awtestcontrol();

    public:
        static int parse(char** ppArguments, int iArgumentsCount,
                         StAwtestcontrolArguments& awsarg, CppString* pStrError = NULL);

        static int createProcess(const char* szCmd, char** ppArgs);

    public:
        bool loadCfg(const CppString& strCfgFilePath);
        const StAwsTestCfg* getCfg() const;

        void setOstream(AwsOStream& output);

        int command(StAwtestcontrolArguments& arguments, CppString* pStrError = NULL);
        int command(char** ppArguments, int iArgumentsCount, CppString* pStrError = NULL);

    protected:
        void awsTest(const CppString& strId, StOptionParam& mainOP, const VecOptionParams& vecSubOpps, CppString* pStrError = NULL);
        void awsFileOp(const CppString& strId, CppString* pStrError = NULL);
        void awsExeCommand(const CppString& strId, const CppString& strCmd, CppString* pStrError = NULL);
        void awsGetInfo(const CppString& strId, CppString* pStrError = NULL);
        void awsGetLastTestResult(const CppString& strId, CppString* pStrError = NULL);
        void awsGetVersion(const CppString& strId, CppString* pStrError = NULL);

    protected:
        void awsBeginTestHelper(const CppString& strId, const CppString& strAuthor, const CppString& strTestCfg);
        void awsTerminateTestHelper(const CppString &strId, const CppString& strSemKey, const CppString& strShmKey);
        void awsTestResponseHelper(bool bPassed, const CppString& strMark);

    protected:
        StAwsTestCfg m_cfg;
        AwsOStream* m_pOutput;
    };
}

#endif // AWSTCRL_H
