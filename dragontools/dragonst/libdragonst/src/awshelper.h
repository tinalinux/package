#ifndef AWSHELPER_H
#define AWSHELPER_H

#include "cppvector.h"
#include "awsostream.h"

#define AWSTEST_TERMINATE_TEST_CODE 100
#define AWSTEST_RESPONSE_PASSED_CODE 101
#define AWSTEST_RESPONSE_FAILED_CODE 102

namespace AwsNP
{
    typedef enum
    {
        EM_AWS_TYPE_NONE = 0,
        EM_AWS_TYPE_TEST,
        EM_AWS_TYPE_VERSION,
        EM_AWS_TYPE_FILE,
        EM_AWS_TYPE_COMMAND,
        EM_AWS_TYPE_INFO,
        EM_AWS_TYPE_LAST_RESULT,
    }EmAwsType;

    typedef enum
    {
        EM_AWS_SUB_TYPE_NONE = 0,
        EM_AWS_SUB_TYPE_TEST_HB,
        EM_AWS_SUB_TYPE_TEST_START,
        EM_AWS_SUB_TYPE_TEST_END,
        EM_AWS_SUB_TYPE_TEST_START_ONE,
        EM_AWS_SUB_TYPE_TEST_END_ONE,
        EM_AWS_SUB_TYPE_TEST_TIP,
        EM_AWS_SUB_TYPE_TEST_RESOURCE,
        EM_AWS_SUB_TYPE_TEST_RES_OP,
        EM_AWS_SUB_TYPE_TEST_OPERATOR,
        EM_AWS_SUB_TYPE_TEST_MATCH_START,
        EM_AWS_SUB_TYPE_TEST_MATCH_END,
        EM_AWS_SUB_TYPE_FILE_APPEND,
        EM_AWS_SUB_TYPE_FILE_CREATE,
        EM_AWS_SUB_TYPE_FILE_DEL,
        EM_AWS_SUB_TYPE_FILE_GET,
        EM_AWS_SUB_TYPE_FILE_PATH,
        EM_AWS_SUB_TYPE_FILE_MODE,
    }EmAwsSubType;

    typedef struct
    {
        CppString strName;
        CppString strValue;
    }StNameValue;

    typedef CppVector<StNameValue> VecNameValue;

    typedef struct
    {
        CppString strAwserdevPath;
        CppString strSemKey;
        CppString strShmKey;
        CppString strSerialPortName;
        CppString strTestProgramFileName;
        CppString strTestProgramFilePath;
        CppString strFilesPath;
        CppString strFileBuffSize;
        CppString strEncode;
        CppString strHeartbeatFreq;
    }StAwsTestCfg;

    class AwsHelper
    {
    public:
        AwsHelper();
        virtual ~AwsHelper();

    public:
        static CppString makeAwtestStr(const CppString& strText);

        static CppString makeAwsContentStr(const VecNameValue& vecNameValues, const CppString& strText);

        static CppString makeAwsBaseStr(const CppString& strId, bool bEnd,
                                        EmAwsType type, EmAwsSubType subType);

        static CppString makeAwtestStr(const CppString& strId, bool bEnd,
                                       EmAwsType type, EmAwsSubType subType,
                                       const VecNameValue& vecNameValues, const CppString& strText);

        static CppString getType(EmAwsType type);
        static CppString getSubType(EmAwsSubType subType);

        static void awsOutput(AwsOStream& output, const CppString& strId, bool bEnd, EmAwsType type,
                              EmAwsSubType subType, const VecNameValue& vecNameValues, const CppString& strText);


		static CppString makeAwFilterStr(const CppString& strText);
		static CppString makeAwFilterStr(const CppString& strId, bool bEnd,
                                       EmAwsType type, EmAwsSubType subType,
                                       const VecNameValue& vecNameValues, const CppString& strText);
		static void awsFilterOutput(AwsOStream& output, const CppString& strId, bool bEnd, EmAwsType type,
                              EmAwsSubType subType, const VecNameValue& vecNameValues, const CppString& strText);


        static bool loadCfg(const CppString& strCfgFilePath, StAwsTestCfg& cfg);
        static void setDefaultConfig(StAwsTestCfg& cfg);
    };
}

#endif // AWSHELPER_H
