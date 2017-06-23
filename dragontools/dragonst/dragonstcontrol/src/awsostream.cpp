#include <stdio.h>
#include "awsostream.h"

namespace AwsNP
{
    AwsOStream::AwsOStream()
    {
    }


    AwsOStream::~AwsOStream()
    {
    }


    AwsOStream& AwsOStream::operator<<(const char* sz)
    {
       printf("%s", sz) ;
        return *this;
    }


    AwsOStream& AwsOStream::operator<<(const CppString& str)
    {
        printf("%s", str.c_str());
        return *this;
    }


    AwsOStream& AwsOStream::operator<<(char ch)
    {
        printf("%c", ch);
        return *this;
    }


    AwsOStream& AwsOStream::operator<<(int iNum)
    {
        printf("%d", iNum);
        return *this;
    }


    AwsOStream& AwsOStream::operator<<(unsigned int uiNum)
    {
        printf("%u", uiNum);
        return *this;
    }


    AwsOStream& AwsOStream::operator<<(float fNum)
    {
        printf("%f", fNum);
        return *this;
    }


    AwsOStream& AwsOStream::operator<<(double dNum)
    {
        printf("%lf", dNum);
        return *this;
    }
}
