#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include "tinyxml.h"
#include "awshelper.h"
#include "awsfostream.h"
#include "AwsDevPlugin.h"

namespace AwsNP
{
    AwsDevPlugin::AwsDevPlugin()
    {
        m_bHasBeenInitialized = false;
        m_bNeedServiceWorking = false;
        m_bNeedTestContinue = false;
        m_pOutput = NULL;
        m_iHeartbeatFreqMillisecond = 0;
        m_semId = 0;
        m_shmId = 0;
        m_pShared = NULL;
    }


    AwsDevPlugin::~AwsDevPlugin()
    {
        release();
    }


    unsigned long AwsDevPlugin::serviceRoutineUL(void* pParam)
    {
        AwsDevPlugin* pPlugin = (AwsDevPlugin*)pParam;
        pPlugin->m_bNeedServiceWorking = true;

        int pFile = -1;
        int iCode = 0;
        int iMsgId = 100000;

        remove(pPlugin->m_cfg.strSemKey.c_str());

        while (pPlugin->m_bNeedServiceWorking)
        {
           GENFUNSemaphoreP(&pPlugin->m_semId);

           switch (pPlugin->m_pShared->state)
           {
           case EM_SHM_STATE_NONE:
               pPlugin->m_pShared->state = EM_SHM_STATE_READ;

               if (strcmp(pPlugin->m_pShared->buffer, "test_stop") == 0)
               {
                   pPlugin->m_bNeedTestContinue = false;
               }

               pPlugin->m_pShared->state = EM_SHM_STATE_NONE;

               break;

           default:
               break;
           }

           GENFUNSemaphoreV(&pPlugin->m_semId);
           GENFUNSleep(1000);

            pFile = open(pPlugin->m_cfg.strSemKey.c_str(), O_RDONLY,S_IWUSR|S_IRUSR);

            if (pFile != -1)
            {
                read(pFile, &iCode, sizeof(int));
                close(pFile);
				printf("read file=%s ok, fid=%d, read code=%d\n", pPlugin->m_cfg.strSemKey.c_str(), pFile, iCode);
                if (iCode == AWSTEST_TERMINATE_TEST_CODE)
                {
                    pFile = open(pPlugin->m_cfg.strSemKey.c_str(), O_RDWR|O_CREAT,S_IWUSR|S_IRUSR);

                    if (pFile != -1)
                    {
                        iCode = 0;
                        write(pFile, &iCode, sizeof(int));
                        close(pFile);
                        pPlugin->m_bNeedTestContinue = false;
                        break;
                    }
                    else
                    {
                        (*pPlugin->m_pOutput) << "awserdev: Failed to open " << pPlugin->m_cfg.strSemKey << "\n";
                    }
                }
            }

            GENFUNSleep(pPlugin->m_iHeartbeatFreqMillisecond);
            (*pPlugin->m_pOutput) << "<awheartbeat></awheartbeat>\n";
        }

        remove(pPlugin->m_cfg.strSemKey.c_str());

        (*pPlugin->m_pOutput) << "<awtest><base msgid=\"" << iMsgId
                              << "\" end=\"1\" type=\"test\" subtype=\"interrupt\"></base><content>"
                              << "</content result=\"true\"></awtest>\n";

        return 0;
    }


    void* AwsDevPlugin::serviceRoutineV(void* pParam)
    {
        unsigned long ulRet = serviceRoutineUL(pParam);

        if (ulRet != 0)
        {

        }

        return NULL;
    }


    int AwsDevPlugin::init(AwsOStream& output, const StAwsTestCfg &cfg)
    {
        key_t semKey;
        key_t shmKey;
        if (m_bNeedServiceWorking)
        {
            return -1;
        }

        m_cfg = cfg;
        m_iHeartbeatFreqMillisecond = atoi(m_cfg.strHeartbeatFreq.c_str());

        int iSemKey = atoi(m_cfg.strSemKey.c_str());
        int iShmKey = atoi(m_cfg.strShmKey.c_str());
        semKey = ftok(".", iSemKey);
        shmKey = ftok(".", iShmKey);

       if (!initSem(semKey))
       {
           return 1;
       }

       if (!initShm(shmKey))
       {
           return 2;
       }

        m_pOutput = &output;
        m_serviceThreadHandle = THREADHANDLENULL;

        if (!GENFUNCreateThread(&m_serviceThreadHandle,
                            #ifdef WIN32
                                serviceRoutineUL
                            #else
                                serviceRoutineV
                            #endif
                                , this))
        {
            return 3;
        }

        m_bHasBeenInitialized = true;

        return 0;
    }


    void AwsDevPlugin::release()
    {
        m_bHasBeenInitialized = false;
        m_bNeedServiceWorking = false;
        m_bNeedTestContinue = false;

        m_vecTestItems.clear();

        if (m_serviceThreadHandle != THREADHANDLENULL)
        {
            GENFUNReleaseThread(&m_serviceThreadHandle);
        }

       releaseShm();
       releaseSem();
        m_pOutput = NULL;
    }


    bool AwsDevPlugin::HasBeenInitialized()
    {
        return m_bHasBeenInitialized;
    }


    bool AwsDevPlugin::initSem(key_t semKey)
    {
       m_semId = semget(semKey, 1, 0666 | IPC_CREAT);

       UnSemun semUnion;
       semUnion.val = 1;

       if(semctl(m_semId, 0, SETVAL, semUnion) == -1)
       {
           return false;
       }

        return true;
    }


    void AwsDevPlugin::releaseSem()
    {
        semctl(m_semId, 0, IPC_RMID);
    }


    bool AwsDevPlugin::initShm(key_t shmKey)
    {
       m_pShared = NULL;
       m_shmId = shmget(shmKey, sizeof(StShared), SHM_R | SHM_W | IPC_CREAT /*| IPC_EXCL*/);

       if (m_shmId == -1)
       {
           return false;
       }

       m_pShared = (StShared*)shmat(m_shmId, NULL, 0);

       if (m_pShared == (void*)-1)
       {
           m_pShared = NULL;
           return false;
       }

       m_pShared->state = EM_SHM_STATE_NONE;

        return true;
    }


    void AwsDevPlugin::releaseShm()
    {
       if (m_pShared != NULL)
       {
           shmdt(m_pShared);
           shmctl(m_shmId, IPC_RMID, 0);
           m_shmId = -1;
           m_pShared = NULL;
       }
    }


    int AwsDevPlugin::awsTestBegin(const CppString& strId, const CppVector<CppString>& vecTestNames)
    {
        size_t len = vecTestNames.size();

        m_vecTestItems.clear();

        for (size_t i = 0; i < len; i++)
        {
            StAwsTestItem item;
            item.strTestName = vecTestNames[i];
            m_vecTestItems.push_back(item);
        }

        char szNum[16];
        VecNameValue vecNameValues;
        StNameValue nv;

        sprintf(szNum, "%d", (int)m_vecTestItems.size());
        nv.strName = "test_count";
        nv.strValue = szNum;
        vecNameValues.push_back(nv);

        AwsHelper::awsFilterOutput(*m_pOutput, strId, true, EM_AWS_TYPE_TEST,
                             EM_AWS_SUB_TYPE_TEST_START, vecNameValues, "");

        m_bNeedTestContinue = true;

        return (int)m_vecTestItems.size();
    }


    int AwsDevPlugin::awsTestFinish(const CppString& strId, bool bPassed, int iErrorCode, const CppString& strError)
    {
        char szNum[16];
        VecNameValue vecNameValues;
        StNameValue nv;

        nv.strName = "result";
        nv.strValue = bPassed ? "true" : "false";
        vecNameValues.push_back(nv);

        sprintf(szNum, "%d", iErrorCode);
        nv.strName = "errcode";
        nv.strValue = szNum;
        vecNameValues.push_back(nv);

        nv.strName = "errstr";
        nv.strValue = strError;
        vecNameValues.push_back(nv);

        AwsHelper::awsFilterOutput(*m_pOutput, strId, true, EM_AWS_TYPE_TEST,
                             EM_AWS_SUB_TYPE_TEST_END, vecNameValues, "");

        return (int)m_vecTestItems.size();
    }


    bool AwsDevPlugin::awsTestContinue() const
    {
        return m_bNeedTestContinue;
    }


    int AwsDevPlugin::awsTestBeginItem(const CppString& strId, int iTestId, const CppString& strTip)
    {
        if (iTestId < 0 || iTestId > (int)m_vecTestItems.size() - 1)
        {
            return -1;
        }

        VecNameValue vecNameValues;
        StNameValue nv;

        nv.strName = "testname";
        nv.strValue = m_vecTestItems[iTestId].strTestName;
        vecNameValues.push_back(nv);

        nv.strName = "tip";
        nv.strValue = strTip;
        vecNameValues.push_back(nv);

        AwsHelper::awsFilterOutput(*m_pOutput, strId, true, EM_AWS_TYPE_TEST,
                             EM_AWS_SUB_TYPE_TEST_START_ONE, vecNameValues, "");

        return iTestId;
    }


    int AwsDevPlugin::awsTestFinishItem(const CppString& strId, int iTestId, bool bPassed, int iErrorCode, const CppString& strError)
    {
        if (iTestId < 0 || iTestId > (int)m_vecTestItems.size() - 1)
        {
            return -1;
        }

        m_vecTestItems[iTestId].strResult = bPassed ? "true" : "false";
		m_vecTestItems[iTestId].strComment = strError;

        char szNum[16];
        VecNameValue vecNameValues;
        StNameValue nv;

        nv.strName = "testname";
        nv.strValue = m_vecTestItems[iTestId].strTestName;
        vecNameValues.push_back(nv);

        nv.strName = "result";
        nv.strValue = m_vecTestItems[iTestId].strResult;
        vecNameValues.push_back(nv);


        sprintf(szNum, "%d", iErrorCode);
        nv.strName = "errcode";
        nv.strValue = szNum;
        vecNameValues.push_back(nv);

        nv.strName = "errstr";
        nv.strValue = strError;
        vecNameValues.push_back(nv);

        AwsHelper::awsFilterOutput(*m_pOutput, strId, true, EM_AWS_TYPE_TEST,
                             EM_AWS_SUB_TYPE_TEST_END_ONE, vecNameValues, "");

        return iTestId;
    }


    int AwsDevPlugin::awsSaveLastTestResult(const CppString& strLastResultFilePath, const CppString& strAuthor, const CppString& strCreateTime)
    {
        AwsFOStream fout;
        size_t itemsCount = m_vecTestItems.size();

        if (!fout.open(strLastResultFilePath))
        {
			printf("failed to create last result xml file\n");
            return 1;
        }

        fout << "<LastTestResult>\n"
            << "\t<Info author=\"" << strAuthor
            << "\" createtime=\"" << strCreateTime << "\"></Info>\n"
            << "\t<Test>\n";

        for (size_t i = 0; i < itemsCount; i++)
        {
            fout << "\t\t<id" << (unsigned int)i << " item_name=\"" << m_vecTestItems[i].strTestName
                << "\" result=\"" << m_vecTestItems[i].strResult << "\" comment=\""
                << m_vecTestItems[i].strComment << "\"></id" << (unsigned int)i << ">\n";
        }

        fout << "\t</Test>\n"
            << "</LastTestResult>";

        fout.flush();
        fout.close();

        return 0;
    }


    int AwsDevPlugin::awsTestResponse(const CppString& strId, int iTestId, int iTimeOut, const CppString& strTip, int& iResponse, CppString& strError)
    {
        CppString strResponseFilePath = m_cfg.strFilesPath + "rs";
        remove(strResponseFilePath.c_str());

        if (iTestId < 0 || iTestId > (int)m_vecTestItems.size() - 1)
        {
            return -1;
        }

        VecNameValue vecNameValues;
        StNameValue nv;
        char szNum[16];

        nv.strName = "testname";
        nv.strValue = m_vecTestItems[iTestId].strTestName;
        vecNameValues.push_back(nv);

        nv.strName = "subject";
        nv.strValue = m_vecTestItems[iTestId].strResult;
        vecNameValues.push_back(nv);

        sprintf(szNum, "%d", iTimeOut);
        nv.strName = "timeout";
        nv.strValue = szNum;
        vecNameValues.push_back(nv);

        nv.strName = "tip";
        nv.strValue = strTip;
        vecNameValues.push_back(nv);

        AwsHelper::awsFilterOutput(*m_pOutput, strId, true, EM_AWS_TYPE_TEST,
                             EM_AWS_SUB_TYPE_TEST_OPERATOR, vecNameValues, "");

        int pFile = -1;
        int iCode = 0;
        int iWaitTime = 0;
		int iCount = 0;
		char szComment[256] = {0};

        iResponse = 0;

        while (m_bNeedTestContinue)
        {
            pFile = open(strResponseFilePath.c_str(), O_RDWR|O_CREAT,S_IWUSR|S_IRUSR);

            if (pFile != -1)
            {

                read(pFile, &iCode, sizeof(int));
				read(pFile, &iCount, sizeof(int));
				if (iCount > 0 && iCount < 256)
				{
					read(pFile, szComment, iCount);
				}
                close(pFile);
				printf("read rs file ok, icode=%d\n", iCode);

                if (iCode == AWSTEST_RESPONSE_PASSED_CODE)
                {
                    iResponse = 1;
					strError = szComment;
                    break;
                }
                else if (iCode == AWSTEST_RESPONSE_FAILED_CODE)
                {
                    iResponse = 2;
					strError = szComment;
                    break;
                }
                else
                {
                    GENFUNSleep(m_iHeartbeatFreqMillisecond);
                    iWaitTime += m_iHeartbeatFreqMillisecond;

                    if (iTimeOut != -1 && iWaitTime > iTimeOut)
                    {
                        break;
                    }

                    continue;
                }
            }
            else
            {
                (*m_pOutput) << "awserdev: Failed to open " << strResponseFilePath << "\n\n";
            }

            GENFUNSleep(m_iHeartbeatFreqMillisecond);
        }

        remove(strResponseFilePath.c_str());

        return 0;
    }
}
