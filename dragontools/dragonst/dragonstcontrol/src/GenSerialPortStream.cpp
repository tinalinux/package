#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "GenSerialPortStream.h"

namespace AwsNP
{
    GenSerialPortStream::GenSerialPortStream()
    {
        m_fd = -1;
    }

    GenSerialPortStream::GenSerialPortStream(const char* szSerialPort)
    {
        m_fd = -1;
        init(szSerialPort);
    }


    GenSerialPortStream::~GenSerialPortStream()
    {
         release();
    }


    bool GenSerialPortStream::init(const char* szSerialPort)
    {
        if (m_fd != -1)
        {
            release();
        }

        m_fd = open(szSerialPort, O_WRONLY);
        return m_fd != -1;
    }


    void GenSerialPortStream::release()
    {
        if (m_fd != -1)
        {
            close(m_fd);
            m_fd = -1;
        }
    }


    void GenSerialPortStream::output(const CppString& str)
    {
        if (m_fd != -1)
        {
            ::write(m_fd, str.c_str(), str.size());
        }
    }


    GenSerialPortStream& GenSerialPortStream::operator<<(const CppString& str)
    {
        output(str);
        return *this;
    }
}
