#ifndef AWSOSTREAM_H
#define AWSOSTREAM_H

#include "cppstring.h"

namespace AwsNP
{
    class AwsOStream
    {
    public:
        AwsOStream();
        virtual ~AwsOStream();

        virtual AwsOStream& operator<<(const char* sz);
        virtual AwsOStream& operator<<(const CppString& str);

        virtual AwsOStream& operator<<(char ch);

        virtual AwsOStream& operator<<(int iNum);
        virtual AwsOStream& operator<<(unsigned int uiNum);

        virtual AwsOStream& operator<<(float fNum);
        virtual AwsOStream& operator<<(double dNum);
    };
}

#endif // AWSOSTREAM_H
