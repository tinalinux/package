#include <stdio.h>
#include <string.h>
#include "awsfostream.h"

namespace AwsNP
{
    AwsFOStream::AwsFOStream()
        : m_pOp(NULL)
    {
    }


    AwsFOStream::~AwsFOStream()
    {
        close();
    }


    bool AwsFOStream::open(const CppString& strFilePath)
    {
        if (m_pOp != NULL)
        {
            close();
        }

        FILE* pFile = fopen(strFilePath.c_str(), "w");

        if (pFile == NULL)
        {
            return false;
        }

        m_pOp = pFile;

        return true;
    }


    void AwsFOStream::close()
    {
        if (m_pOp != NULL)
        {
            fclose((FILE*)m_pOp);
            m_pOp = NULL;
        }
    }


    void AwsFOStream::flush()
    {
        if (m_pOp != NULL)
        {
            fflush((FILE*)m_pOp);
        }
    }


    AwsFOStream& AwsFOStream::operator<<(const char* sz)
    {
        if (m_pOp != NULL)
        {
            fwrite(sz, 1, strlen(sz), (FILE*)m_pOp);
        }

        return *this;
    }


    AwsFOStream& AwsFOStream::operator<<(const CppString& str)
    {
        if (m_pOp != NULL)
        {
            fwrite(str.c_str(), 1, str.size(), (FILE*)m_pOp);
        }

        return *this;
    }


    AwsFOStream& AwsFOStream::operator<<(char ch)
    {
        if (m_pOp != NULL)
        {
            char sz[2];
            sz[0] = ch;
            sz[1] = '\0';
            fwrite(sz, 1, strlen(sz), (FILE*)m_pOp);
        }

        return *this;
    }


    AwsFOStream& AwsFOStream::operator<<(int iNum)
    {
        if (m_pOp != NULL)
        {
            char sz[32];
            sprintf(sz, "%d", iNum);
            fwrite(sz, 1, strlen(sz), (FILE*)m_pOp);
        }

        return *this;
    }


    AwsFOStream& AwsFOStream::operator<<(unsigned int uiNum)
    {
        if (m_pOp != NULL)
        {
            char sz[32];
            sprintf(sz, "%u", uiNum);
            fwrite(sz, 1, strlen(sz), (FILE*)m_pOp);
        }

        return *this;
    }


    AwsFOStream& AwsFOStream::operator<<(float fNum)
    {
        if (m_pOp != NULL)
        {
            char sz[32];
            sprintf(sz, "%f", fNum);
            fwrite(sz, 1, strlen(sz), (FILE*)m_pOp);
        }

        return *this;
    }


    AwsFOStream& AwsFOStream::operator<<(double dNum)
    {
        if (m_pOp != NULL)
        {
            char sz[32];
            sprintf(sz, "%lf", dNum);
            fwrite(sz, 1, strlen(sz), (FILE*)m_pOp);
        }

        return *this;
    }
}
