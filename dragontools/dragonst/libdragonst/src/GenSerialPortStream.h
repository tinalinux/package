#ifndef GENSERIALPORTSTREAM_H
#define GENSERIALPORTSTREAM_H

#include "awsostream.h"

namespace AwsNP
{
    class GenSerialPortStream : public AwsOStream
    {
    public:
        GenSerialPortStream();
        explicit GenSerialPortStream(const char* szSerialPort);
        virtual ~GenSerialPortStream();

    public:
        virtual bool init(const char* szSerialPort);
        virtual void release();

        virtual void output(const CppString& str);
        virtual GenSerialPortStream& operator<<(const CppString& str);

    protected:
        int m_fd;
    };
}

#endif // GENSERIALPORTSTREAM_H
