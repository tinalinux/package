#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "awstcrl.h"
#include "tinyxml.h"
#include "GeneralFuncs.h"

#define AWTESTCONTROL_VERSION "v1.0.0"
#define AWTESTCONTROL_AUTHOR ""
#define AWTESTCONTROL_TIME ""

#define CONFIG_FILE_NAME "awtestcontrol_cfg.xml"
#define LAST_TEST_RESULT_FILE_NAME "last_test_result.xml"

namespace AwsNP
{
    Awtestcontrol::Awtestcontrol()
        : m_pOutput(NULL)
    {
    }


    Awtestcontrol::~Awtestcontrol()
    {
    }


    int Awtestcontrol::parse(char** ppArguments, int iArgumentsCount,
                             StAwtestcontrolArguments& awsarg, CppString* pstrError)
    {
        awsarg.strId = "";
        awsarg.mainOP.strOption = "";
        awsarg.mainOP.strParam = "";
        awsarg.vecSubOpps.clear();

        if (ppArguments == NULL || iArgumentsCount < 4)
        {
            if (pstrError != NULL)
            {
                *pstrError = "Missing arguments";
            }

            return 1;
        }

        int iIndex = 0;

        while (iIndex < iArgumentsCount)
        {
            if (ppArguments[iIndex][0] == '-')
            {
                break;
            }

            iIndex++;
        }

        if (strcmp(ppArguments[iIndex], "-msgid") != 0)
        {
            if (pstrError != NULL)
            {
                *pstrError = "Missing -msgid";
            }

            return 2;
        }

        iIndex++;

        if (ppArguments[iIndex][0] == '-')
        {
            if (pstrError != NULL)
            {
                *pstrError = "Missing msgid";
            }

            return -3;
        }

        awsarg.strId = ppArguments[iIndex];
        iIndex++;

        if (ppArguments[iIndex][0] != '-')
        {
            if (pstrError != NULL)
            {
                *pstrError = "Missing option";
            }

            return 4;
        }

        awsarg.mainOP.strOption = ppArguments[iIndex];
        iIndex++;

        if (iIndex < iArgumentsCount)
        {
            if (ppArguments[iIndex][0] != '-')
            {
                awsarg.mainOP.strParam = ppArguments[iIndex];
                iIndex++;
            }

            int iOptionIndex = -1;

            while (iIndex < iArgumentsCount)
            {
                if (ppArguments[iIndex][0] == '-')
                {
                    if (iOptionIndex != -1)
                    {
                        StOptionParam oPp;
                        oPp.strOption = ppArguments[iOptionIndex];
                        oPp.strParam = "";
                        awsarg.vecSubOpps.push_back(oPp);
                    }

                    iOptionIndex = iIndex;
                }
                else // 参数
                {
                    if (iOptionIndex != -1)
                    {
                        StOptionParam oPp;
                        oPp.strOption = ppArguments[iOptionIndex];
                        oPp.strParam = ppArguments[iIndex];
                        awsarg.vecSubOpps.push_back(oPp);
                        iOptionIndex = -1;
                    }
                    else
                    {
                        if (pstrError != NULL)
                        {
                            *pstrError = "The command format is wrong.";
                        }

                        return 5;
                    }
                }

                iIndex++;
            } // while End

            if (iOptionIndex != -1)
            {
                StOptionParam oPp;
                oPp.strOption = ppArguments[iOptionIndex];
                oPp.strParam = "";
                awsarg.vecSubOpps.push_back(oPp);
            }
        }

        return 0;
    }


    int Awtestcontrol::createProcess(const char* szCmd, char** ppArgs)
    {
        pid_t pid = vfork();

        if (pid == 0)
        {
            if (execv(szCmd, ppArgs) == -1)
            {
                //cout << strCmdWithArgs << "  ret: " << pid << endl << endl;
            }
        }

        return pid;
    }


    bool Awtestcontrol::loadCfg(const CppString& strCfgFilePath)
    {
        bool bRet = AwsHelper::loadCfg(strCfgFilePath, m_cfg);

        if (!bRet)
        {
            AwsHelper::setDefaultConfig(m_cfg);
        }

        return bRet;
    }


    const StAwsTestCfg* Awtestcontrol::getCfg() const
    {
        return &m_cfg;
    }


    void Awtestcontrol::setOstream(AwsOStream& output)
    {
        m_pOutput = &output;
    }


    int Awtestcontrol::command(StAwtestcontrolArguments& arguments, CppString* pStrError)
    {
        if (m_pOutput == NULL)
        {
            return -1;
        }

        if (arguments.mainOP.strOption == "-t")
        {
            awsTest(arguments.strId, arguments.mainOP, arguments.vecSubOpps, pStrError);
        }
        else if (arguments.mainOP.strOption == "-f")
        {
            (*m_pOutput) << "还没有实现\n\n";
        }
        else if (arguments.mainOP.strOption == "-e")
        {
            awsExeCommand(arguments.strId, arguments.mainOP.strParam, pStrError);
        }
        else if (arguments.mainOP.strOption == "-i")
        {
            awsGetInfo(arguments.strId, pStrError);
        }
        else if (arguments.mainOP.strOption == "-r")
        {
            awsGetLastTestResult(arguments.strId, pStrError);
        }
        else if (arguments.mainOP.strOption == "-v")
        {
            awsGetVersion(arguments.strId, pStrError);
        }

        return 0;
    }


    int Awtestcontrol::command(char** ppArguments, int iArgumentsCount, CppString* pStrError)
    {
        StAwtestcontrolArguments awsarg;
        int iRet = parse(ppArguments, iArgumentsCount, awsarg, pStrError);

        if (iRet == 0)
        {
            iRet = command(awsarg, pStrError);
        }

        return iRet;
    }


    void Awtestcontrol::awsTest(const CppString& strId, StOptionParam& mainOP, const VecOptionParams& vecSubOpps, CppString* pStrError)
    {
        if (vecSubOpps.size() == 0)
        {
            if (pStrError != NULL)
            {
                *pStrError = "Missing param.";
            }

            return;
        }

        const CppString& strOption = vecSubOpps[0].strOption;

        if (strOption == "-p")
        {
            awsBeginTestHelper(strId, mainOP.strParam, vecSubOpps[0].strParam);
        }
        else if (strOption == "-a")
        {
            awsTerminateTestHelper(strId, m_cfg.strSemKey, m_cfg.strShmKey);
        }
        else if (strOption == "-o")
        {
            bool bPassed = (vecSubOpps[0].strParam == "1") ? true : false;
			CppString strMark = "";
			if (vecSubOpps.size() >= 2) {
				strMark = vecSubOpps[1].strParam;
			}
            awsTestResponseHelper(bPassed, strMark);
        }
    }


    void Awtestcontrol::awsFileOp(const CppString& strId, CppString* pStrError)
    {
        if (strId == "000")
        {
            if (pStrError != NULL)
            {
                *pStrError = "";
            }
        }
    }


    void Awtestcontrol::awsExeCommand(const CppString& strId, const CppString& strCmd, CppString* pStrError)
    {
        VecNameValue vecNameValues;
        StNameValue nv;
        CppString strCmdWithoutDQ;

        nv.strName = "result";
        nv.strValue = "0";
        vecNameValues.push_back(nv);

        nv.strName = "errcode";
        nv.strValue = "-1";
        vecNameValues.push_back(nv);

        nv.strName = "errstr";
        nv.strValue = "";
        vecNameValues.push_back(nv);

        if (strCmd.empty())
        {
            vecNameValues[2].strValue = "The command is empty.";

            if (pStrError != NULL)
            {
                *pStrError = vecNameValues[2].strValue;
            }

            AwsHelper::awsOutput(*m_pOutput, strId, true, EM_AWS_TYPE_COMMAND, EM_AWS_SUB_TYPE_NONE, vecNameValues, "");
            return;
        }

        if (strCmd[0] == '"')
        {
            if (strCmd[strCmd.size() - 1] != '"')
            {
                vecNameValues[2].strValue = "The command format is wrong.";

                if (pStrError != NULL)
                {
                    *pStrError = vecNameValues[2].strValue;
                }

                AwsHelper::awsOutput(*m_pOutput, strId, true, EM_AWS_TYPE_COMMAND, EM_AWS_SUB_TYPE_NONE, vecNameValues, "");
                return;
            }

            strCmdWithoutDQ = strCmd.c_str() + 1;
            strCmdWithoutDQ[strCmdWithoutDQ.size() - 1] = '\0';
        }
        else
        {
            strCmdWithoutDQ = strCmd;
        }

        CppVector<CppString> vecArgs;
        int iLen = strCmdWithoutDQ.size();
        int iBeginIndex = 0;

        for (int i = 0; i < iLen; i++)
        {
            if (strCmdWithoutDQ[i] == ' ')
            {
                vecArgs.push_back(strCmdWithoutDQ.substr(iBeginIndex, i - iBeginIndex));
                iBeginIndex = i + 1;
            }
        }

        if (iBeginIndex < iLen)
        {
            vecArgs.push_back(strCmdWithoutDQ.substr(iBeginIndex, iLen - iBeginIndex));
        }

        iLen = vecArgs.size();

        char** ppArgs = new char*[iLen + 1];
        ppArgs[iLen] = NULL;

        for (int i = 0; i < iLen; i++)
        {
            ppArgs[i] = (char*)vecArgs[i].c_str();
        }

        if (createProcess(ppArgs[0], ppArgs) > 0)
        {
            vecNameValues[0].strValue = "1";
            vecNameValues[1].strValue = "0";
        }
        else
        {
            vecNameValues[0].strValue = "0";
            vecNameValues[2].strValue = "Failed to create process.";
        }

        AwsHelper::awsOutput(*m_pOutput, strId, true, EM_AWS_TYPE_COMMAND, EM_AWS_SUB_TYPE_NONE, vecNameValues, "");
    }


    void Awtestcontrol::awsGetInfo(const CppString& strId, CppString* pStrError)
    {
        VecNameValue vecNameValues;
        StNameValue nv;

        nv.strName = "file_buff_size";
        nv.strValue = m_cfg.strFileBuffSize;
        vecNameValues.push_back(nv);

        nv.strName = "encode";
        nv.strValue = m_cfg.strEncode;
        vecNameValues.push_back(nv);

        nv.strName = "heartbeat";
        nv.strValue = m_cfg.strHeartbeatFreq;
        vecNameValues.push_back(nv);

        nv.strName = "port";
        nv.strValue = m_cfg.strSerialPortName;
        vecNameValues.push_back(nv);

        AwsHelper::awsOutput(*m_pOutput, strId, true, EM_AWS_TYPE_INFO,
                             EM_AWS_SUB_TYPE_NONE, vecNameValues, "");

        if (pStrError != NULL)
        {
            *pStrError = "";
        }
    }


    void Awtestcontrol::awsGetLastTestResult(const CppString& strId, CppString* pStrError)
    {
        CppString strLastResultFilePath = m_cfg.strFilesPath + LAST_TEST_RESULT_FILE_NAME;
        TiXmlDocument xml;
        VecNameValue infoNameValue;
        StNameValue nv;

        if (!xml.LoadFile(strLastResultFilePath.c_str()))
        {
            nv.strName = "error";
            nv.strValue = "Failed to load " + strLastResultFilePath;
            infoNameValue.push_back(nv);

            if (pStrError != NULL)
            {
                *pStrError = nv.strValue;
            }

            AwsHelper::awsOutput(*m_pOutput, strId, true, EM_AWS_TYPE_LAST_RESULT,
                                 EM_AWS_SUB_TYPE_NONE, infoNameValue, "");

            return;
        }

        // <LastTestResult></LastTestResult>
        TiXmlElement* pRootElement = xml.RootElement();

        if (pRootElement == NULL)
        {
            return;
        }

        // <Info></Info>
        TiXmlElement* pInfoElement = pRootElement->FirstChildElement();

        if (strcmp(pInfoElement->Value(), "Info") != 0)
        {
            nv.strName = "error";
            nv.strValue = "Failed to get last test result. 1";
            infoNameValue.push_back(nv);

            if (pStrError != NULL)
            {
                *pStrError = nv.strValue;
            }

            AwsHelper::awsOutput(*m_pOutput, strId, true, EM_AWS_TYPE_LAST_RESULT,
                                 EM_AWS_SUB_TYPE_NONE, infoNameValue, "");

            return;
        }

        // <Test></Test>
        TiXmlElement* pTestElement = pInfoElement->NextSiblingElement();
        pTestElement = pTestElement->FirstChildElement();

        if (pTestElement == NULL)
        {
            nv.strName = "error";
            nv.strValue = "Failed to get last test result. 1";
            infoNameValue.push_back(nv);

            if (pStrError != NULL)
            {
                *pStrError = nv.strValue;
            }

            AwsHelper::awsOutput(*m_pOutput, strId, true, EM_AWS_TYPE_LAST_RESULT,
                                 EM_AWS_SUB_TYPE_NONE, infoNameValue, "");

            return;
        }

        nv.strName = "author";
        nv.strValue = pInfoElement->Attribute("author");
        infoNameValue.push_back(nv);

        nv.strName = "createtime";
        nv.strValue = pInfoElement->Attribute("createtime");
        infoNameValue.push_back(nv);

        CppString strText;

        while (pTestElement != NULL)
        {
            // <id item_name=”” result=”” comment=””></id>
            strText += "<";
            strText += pTestElement->Value();
            strText += " item_name=\"";
            strText += pTestElement->Attribute("item_name");
            strText += "\" result=\"";
            strText += pTestElement->Attribute("result");
            strText += "\" comment=\"";
            strText += pTestElement->Attribute("comment");
            strText += "\"></";
            strText += pTestElement->Value();
            strText += ">";

            pTestElement= pTestElement->NextSiblingElement();
        }

        AwsHelper::awsOutput(*m_pOutput, strId, true,
                             EM_AWS_TYPE_LAST_RESULT, EM_AWS_SUB_TYPE_NONE,
                             infoNameValue, strText);
    }


    void Awtestcontrol::awsGetVersion(const CppString& strId, CppString* pStrError)
    {
        CppString strPluginVersion;
        CppString strPluginAuthor;
        CppString strPluginTime;

        VecNameValue vecNameValues;
        StNameValue nv;

        nv.strName = "version";
        nv.strValue = AWTESTCONTROL_VERSION;
        vecNameValues.push_back(nv);

        nv.strName = "author";
        nv.strValue = AWTESTCONTROL_AUTHOR;
        vecNameValues.push_back(nv);

        nv.strName = "time";
        nv.strValue = AWTESTCONTROL_TIME;
        vecNameValues.push_back(nv);

        // load plugin so, then get version, author and time.

        nv.strName = "plugin_ver";
        nv.strValue = strPluginVersion;
        vecNameValues.push_back(nv);

        nv.strName = "plugin_author";
        nv.strValue = strPluginAuthor;
        vecNameValues.push_back(nv);

        nv.strName = "plugin_time";
        nv.strValue = strPluginTime;
        vecNameValues.push_back(nv);

        AwsHelper::awsOutput(*m_pOutput, strId, true, EM_AWS_TYPE_VERSION,
                             EM_AWS_SUB_TYPE_NONE, vecNameValues, "");

        if (pStrError != NULL)
        {
            *pStrError = "";
        }
    }


    void Awtestcontrol::awsBeginTestHelper(const CppString& strId, const CppString& strAuthor, const CppString& strTestCfg)
    {
        char* ppArgs[5];
        ppArgs[0] = (char*)m_cfg.strTestProgramFileName.c_str();
        ppArgs[1] = (char*)strId.c_str();
        ppArgs[2] = (char*)strAuthor.c_str();
        ppArgs[3] = (char*)strTestCfg.c_str();
        ppArgs[4] = NULL;

        CppString strCommand = "ps -e | grep \'";
        strCommand += m_cfg.strTestProgramFileName;
        strCommand += "\' | busybox awk \'{print $1}\'";

        FILE* fp = popen(strCommand.c_str(), "r");//打开管道，执行shell 命令
        char szPid[16] = {0};
        bool bFlag = false;

        if (fp != NULL)
        {
            if (fgets(szPid, sizeof(szPid), fp) != NULL) //逐行读取执行结果并打印
            {
                if (strlen(szPid) != 0)
                {
                    (*m_pOutput) << "<awtest><base msgid=" << strId
                                 << " end=1 type=\"test\" subtype=\"start\"></base>"
                                 << "<content error=\"The test has already begun\"></content></awtest>\n";

                    bFlag = true;
                }
            }

            pclose(fp);
        }

        if (bFlag)
        {
            return;
        }

        if (access(m_cfg.strTestProgramFilePath.c_str(), F_OK) != 0)
        {
            StNameValue nv;
            VecNameValue vecNameValues;

            nv.strName = "result";
            nv.strValue = "0";
            vecNameValues.push_back(nv);

            nv.strName = "errcode";
            nv.strValue = "-1";
            vecNameValues.push_back(nv);

            nv.strName = "errstr";
            nv.strValue = "Failed to launch " + m_cfg.strTestProgramFilePath;
            vecNameValues.push_back(nv);

            AwsHelper::awsOutput(*m_pOutput, strId, true, EM_AWS_TYPE_TEST, EM_AWS_SUB_TYPE_TEST_END, vecNameValues, "");
            return;
        }

        createProcess(m_cfg.strTestProgramFilePath.c_str(), ppArgs);
    }


    void Awtestcontrol::awsTerminateTestHelper(const CppString& strId, const CppString& strSemKey, const CppString& strShmKey)
    {
       int iSemKey = atoi(strSemKey.c_str());
       int iShmKey = atoi(strShmKey.c_str());

       int iSemId = semget(iSemKey, 1, 0666 | IPC_CREAT);

       StShared* pShared = NULL;
       int iShmId = shmget(iShmKey, sizeof(StShared), SHM_R | SHM_W | IPC_CREAT);

       if (iShmId == -1)
       {
           return;
       }

       pShared = (StShared*)shmat(iShmId, NULL, 0);

       if (pShared == (void*)-1)
       {
           pShared = NULL;
           return;
       }

       UnSemun semUnion;
       semUnion.val = 1;

       if(semctl(iSemId, 0, SETVAL, semUnion) == -1)
       {
           shmctl(iShmId, IPC_RMID, 0);
           semctl(iSemKey, 0, IPC_RMID);
           return;
       }

       while (true)
       {
           GENFUNSemaphoreP(&iSemId);

           if (pShared->state == EM_SHM_STATE_NONE)
           {
               break;
           }

           GENFUNSemaphoreV(&iSemId);
           GENFUNSleep(1000);
       }

       strcpy(pShared->buffer, "test_stop");

       GENFUNSemaphoreV(&iSemId);

       shmdt(pShared);
       shmctl(iShmId, IPC_RMID, 0);
       semctl(iSemId, 0, IPC_RMID);

        //int pFile = -1;
        //int iCode = 100;
        //int iCount = 0;

        //while (pFile == -1 && iCount < 30)
        //{
            //pFile = open(m_cfg.strSemKey.c_str(), O_WRONLY|O_CREAT,S_IWUSR|S_IRUSR);
            //(*m_pOutput) << m_cfg.strSemKey.c_str() << "\n\n";
            //printf("interrupt open return pFile=%d, errcode=%d\n", pFile, errno);
            //if (pFile != -1)
            //{
                //write(pFile, &iCode, sizeof(int));
                //close(pFile);
                //break;
            //}
            //else
            //{
                //(*m_pOutput) << "awtestcontrol: Failed to open " << m_cfg.strSemKey.c_str() << "\n\n";
            //}

            //iCount++;
            //GENFUNSleep(1000);
        //}

        //if (iCount >= 30)
        //{
            //(*m_pOutput) << "awtestcontrol: TerminateTest failed.\n\n";

            //(*m_pOutput) << "<awtest><base msgid=\"" << strId
                         //<< "\" end=\"1\" type=\"test\" subtype=\"interrupt\"></base><content>"
                         //<< "</content result=\"false\"></awtest>\n";
        //}
    }


    void Awtestcontrol::awsTestResponseHelper(bool bPassed, const CppString& strMark)
    {
        int pFile = 0;
        int iCode = 0;
        CppString strResponseFilePath = m_cfg.strFilesPath + "rs";
        int iCount = 0;
		pFile = open(strResponseFilePath.c_str(), O_RDWR|O_CREAT,S_IWUSR|S_IRUSR);

		if (pFile != 0)
		{
			iCode = (bPassed == 1) ? AWSTEST_RESPONSE_PASSED_CODE : AWSTEST_RESPONSE_FAILED_CODE;
			write(pFile, &iCode, sizeof(int));
			iCount = strMark.size();
			write(pFile, &iCount, sizeof(int));
			if (iCount > 0)
			{
				write(pFile, strMark.c_str(), iCount);
			}

			printf("create rs file ok, write code=%d, mark size=%d, mark str=%s", iCode, iCount, strMark.c_str());
			close(pFile);
		}
		else
		{
			(*m_pOutput) << "awtestcontrol: Failed to open " << strResponseFilePath << "\n";
		}
    }
} // namespace End
