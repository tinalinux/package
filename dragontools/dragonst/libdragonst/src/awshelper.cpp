#include "tinyxml.h"
#include "awshelper.h"

namespace AwsNP
{
    AwsHelper::AwsHelper()
    {
    }

    AwsHelper::~AwsHelper()
    {
    }


    CppString AwsHelper::makeAwtestStr(const CppString& strText)
    {
        return "<awtest>" + strText + "</awtest>";
    }

	CppString AwsHelper::makeAwFilterStr(const CppString& strText)
	{
		return "<awfilter>" + strText + "</awfilter>";
	}

    CppString AwsHelper::makeAwsContentStr(const VecNameValue& vecNameValues, const CppString& strText)
    {
        CppString str;
        size_t len = vecNameValues.size();

        str = "<content";

        for (size_t i = 0; i < len; i++)
        {
            str += ' ';
            str += vecNameValues[i].strName;
            str += "=\"";;
            str += vecNameValues[i].strValue;
            str += '"';
        }

        str += '>';
        str += strText;
        str += "</content>";

        return str;
    }


    CppString AwsHelper::makeAwsBaseStr(const CppString& strId, bool bEnd,
                                        EmAwsType type, EmAwsSubType subType)
    {
        CppString str;
        CppString strType = getType(type);
        CppString strSubType = getSubType(subType);

        str = "<base msgid=\"";
        str += strId;
        str += "\" end=\"";
        str += bEnd ? "1" : "0";
        str += "\" type=";
        str += strType;
        str += " subtype=";
        str += strSubType;
        str += "></base>";

        return str;
    }


    CppString AwsHelper::makeAwtestStr(const CppString& strId, bool bEnd,
                                       EmAwsType type, EmAwsSubType subType,
                                       const VecNameValue& vecNameValues, const CppString& strText)
    {
        CppString str;
        str = makeAwsBaseStr(strId, bEnd, type, subType);
        str += makeAwsContentStr(vecNameValues, strText);
        return makeAwtestStr(str);
    }


	CppString AwsHelper::makeAwFilterStr(const CppString& strId, bool bEnd,
                                       EmAwsType type, EmAwsSubType subType,
                                       const VecNameValue& vecNameValues, const CppString& strText)
    {
        CppString str;
        str = makeAwsBaseStr(strId, bEnd, type, subType);
        str += makeAwsContentStr(vecNameValues, strText);
        return makeAwFilterStr(str);
    }

    CppString AwsHelper::getType(EmAwsType type)
    {
        CppString strType = "\"\"";

        switch (type)
        {
        case EM_AWS_TYPE_TEST:
            strType = "\"test\"";
            break;

        case EM_AWS_TYPE_VERSION:
            strType = "\"version\"";
            break;

        case EM_AWS_TYPE_FILE:
            strType = "\"file\"";
            break;

        case EM_AWS_TYPE_COMMAND:
            strType = "\"command\"";
            break;

        case EM_AWS_TYPE_INFO:
            strType = "\"info\"";
            break;

        case EM_AWS_TYPE_LAST_RESULT:
            strType = "\"last-result\"";
            break;

        default:
            break;
        }

        return strType;
    }


    CppString AwsHelper::getSubType(EmAwsSubType subType)
    {
        CppString strSubType = "\"\"";

        switch (subType)
        {
        case EM_AWS_SUB_TYPE_TEST_HB:
           strSubType = "\"hb\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_START:
           strSubType = "\"start\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_END:
           strSubType = "\"end\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_START_ONE:
           strSubType = "\"start-one\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_END_ONE:
           strSubType = "\"end-one\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_TIP:
           strSubType = "\"tip\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_RESOURCE:
           strSubType = "\"resource\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_RES_OP:
           strSubType = "\"res-op\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_OPERATOR:
           strSubType = "\"operator\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_MATCH_START:
           strSubType = "\"match-start\"";
           break;

        case EM_AWS_SUB_TYPE_TEST_MATCH_END:
           strSubType = "\"match-end\"";
           break;

        case EM_AWS_SUB_TYPE_FILE_APPEND:
           strSubType = "\"append\"";
           break;

        case EM_AWS_SUB_TYPE_FILE_CREATE:
           strSubType = "\"create\"";
           break;

        case EM_AWS_SUB_TYPE_FILE_DEL:
           strSubType = "\"del\"";
           break;

        case EM_AWS_SUB_TYPE_FILE_GET:
           strSubType = "\"get\"";
           break;

        case EM_AWS_SUB_TYPE_FILE_PATH:
           strSubType = "\"path\"";
           break;

        case EM_AWS_SUB_TYPE_FILE_MODE:
           strSubType = "\"mode\"";
           break;

        default:
           break;
        }

        return strSubType;
    }


    void AwsHelper::awsOutput(AwsOStream& output, const CppString& strId, bool bEnd, EmAwsType type,
                              EmAwsSubType subType, const VecNameValue& vecNameValues, const CppString& strText)
    {
        CppString strOutput = makeAwtestStr(strId, bEnd, type, subType, vecNameValues, strText);
        output << strOutput << "\n";
    }

	void AwsHelper::awsFilterOutput(AwsOStream& output, const CppString& strId, bool bEnd, EmAwsType type,
                              EmAwsSubType subType, const VecNameValue& vecNameValues, const CppString& strText)
    {
        CppString strOutput = makeAwFilterStr(strId, bEnd, type, subType, vecNameValues, strText);
        output << strOutput << "\n";
    }


    bool AwsHelper::loadCfg(const CppString& strCfgFilePath, StAwsTestCfg& cfg)
    {
        TiXmlDocument xml;
        TiXmlElement* pRootElement = NULL;
        TiXmlElement* pElement= NULL;

        if (!xml.LoadFile(strCfgFilePath.c_str()))
        {
            return false;
        }

        pRootElement = xml.RootElement();

        if (pRootElement != NULL)
        {
            pElement = pRootElement->FirstChildElement();

            while (pElement != NULL)
            {
                if (strcmp(pElement->Value(), "awserdev") == 0)
                {
                    cfg.strAwserdevPath = pElement->Attribute("path");
                }
                else if (strcmp(pElement->Value(), "shm") == 0)
                {
                    cfg.strShmKey = pElement->Attribute("key");
                }
                else if (strcmp(pElement->Value(), "SerialPort") == 0)
                {
                    cfg.strSerialPortName = pElement->Attribute("name");
                }
                else if (strcmp(pElement->Value(), "TestProgram") == 0)
                {
                    cfg.strTestProgramFileName = pElement->Attribute("name");
                    cfg.strTestProgramFilePath = pElement->Attribute("path");
                }
                else if (strcmp(pElement->Value(), "FilesPath") == 0)
                {
                    cfg.strFilesPath = pElement->Attribute("path");
                }
                else if (strcmp(pElement->Value(), "FileBuffSize") == 0)
                {
                    cfg.strFileBuffSize = pElement->Attribute("size");
                }
                else if (strcmp(pElement->Value(), "Encode") == 0)
                {
                    cfg.strEncode = pElement->Attribute("name");
                }
                else if (strcmp(pElement->Value(), "Heartbeat") == 0)
                {
                    cfg.strHeartbeatFreq = pElement->Attribute("freq");
                }

                pElement = pElement->NextSiblingElement();
            } // while End
        }

        return true;
    }


    void AwsHelper::setDefaultConfig(StAwsTestCfg& cfg)
    {
        cfg.strAwserdevPath = "";
        cfg.strSemKey = "/tmp/sem";
        cfg.strShmKey = "";
        cfg.strSerialPortName = "__std";
        cfg.strTestProgramFileName = "awtest";
        cfg.strTestProgramFilePath = "/usr/bin/awtest";
        cfg.strFilesPath = "/tmp/";
        cfg.strFileBuffSize = "4096";
        cfg.strEncode = "";
        cfg.strHeartbeatFreq = "2000";
    }
}
