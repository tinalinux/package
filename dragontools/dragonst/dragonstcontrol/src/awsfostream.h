#ifndef AWSFOSTREAM_H
#define AWSFOSTREAM_H

#include "awsostream.h"

namespace AwsNP
{
    class AwsFOStream : public AwsOStream
    {
    public:
        AwsFOStream();
        virtual ~AwsFOStream();

        virtual bool open(const CppString& strFilePath);
        virtual void close();
        virtual void flush();

        virtual AwsFOStream& operator<<(const char* sz);
        virtual AwsFOStream& operator<<(const CppString& str);

        virtual AwsFOStream& operator<<(char ch);

        virtual AwsFOStream& operator<<(int iNum);
        virtual AwsFOStream& operator<<(unsigned int uiNum);

        virtual AwsFOStream& operator<<(float fNum);
        virtual AwsFOStream& operator<<(double dNum);

    protected:
        void* m_pOp;
    };
}

#endif // AWSFOSTREAM_H
