#include "pch.h"
#include "CNet.h"

namespace cnet
{



    int write(CNetSocket socket, const char* buff, int len, int flags)
    {
        return send(socket, buff, len, flags);
    }

    char* cnet_localhost()
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
        {
            return nullptr;
        }

        hostent* localHost = gethostbyname(NULL);
        if (localHost)
        {
            return inet_ntoa(*(struct in_addr*)*localHost->h_addr_list);
        }
        return nullptr;
    }

    CNET_EXPORTS char* cnet_inaddr_any()
    {
        in_addr inaddr;
        inaddr.s_addr = INADDR_ANY;
        return inet_ntoa(inaddr);
    }

    CNetProps cnet_create_props(char* host, unsigned short int port)
    {
        CNetProps props;

        props.addr.sin_family = AF_INET;
        props.addr.sin_addr.s_addr = inet_addr(host);// htonl(INADDR_ANY);
        props.addr.sin_port = htons(port);

        props.packet_size = 1024;
        props.non_blocking = false;

        return props;
    }

    CNetBuffer::CNetBuffer(size_t size)
    {
        this->resize(size);
    }

    CNetBuffer::CNetBuffer(const char* s)
    {
        if (this->alloc(strlen(s) + 1))
        {
            memcpy(this->m_buffer, s, strlen(s) + 1);
        }
    }


    CNetBuffer::CNetBuffer(char* data, size_t size)
    {
        if(this->resize(size))
        {
            memcpy(this->m_buffer, data, size);
        }
    }

    CNetBuffer::CNetBuffer()
    {
    }

    CNetBuffer::~CNetBuffer()
    {
    }

    bool CNetBuffer::resize(size_t size)
    {
        bool was_alloc = this->m_length != 0 && this->m_buffer != nullptr;
        if (size <= this->m_length)
        {
            return true;
        }
        if (this->alloc(size))
        {
            if (was_alloc)
            {
                memset(this->m_buffer + size, 0, size);
            }
            else {
                memset(this->m_buffer, 0, this->m_length);
            }

            return true;
        }

        return false;
    }

    void CNetBuffer::clear()
    {
        this->m_length = 0;
        if (this->m_buffer != nullptr && this->m_allocCount != 0)
        {
            delete[] this->m_buffer;
            this->m_buffer = nullptr;
        }
        this->m_allocCount = 0;
    }

    void CNetBuffer::copy(CNetBuffer obj)
    {
        if (obj.m_buffer == nullptr)
        {
            return;
        }

        if (!this->alloc(obj.m_length))
        {
            return;
        }
        memcpy(this->m_buffer, obj.m_buffer, obj.m_length);
    }

    char& CNetBuffer::operator[](int i)
    {
        return this->m_buffer[i];
    }

    void CNetBuffer::operator=(CNetBuffer obj)
    {
        this->m_length = obj.m_length;
        this->m_buffer = obj.m_buffer;
    }


    CNetBuffer CNetBuffer::operator+(CNetBuffer obj)
    {
        if (this->m_buffer == nullptr || obj.m_buffer == nullptr)
        {
            return CNetBuffer();
        }

        CNetBuffer buffer(obj.m_length + this->m_length);
        memcpy(buffer.m_buffer, this->m_buffer, this->m_length);
        memcpy(buffer.m_buffer + this->m_length, obj.m_buffer, obj.m_length);
        return buffer;
    }

    void CNetBuffer::operator+=(char c)
    {
        if (this->m_buffer == nullptr || !this->alloc(this->m_length + 1))
        {
            return;
        }
        this->m_buffer[this->m_length - 1] = c;
    }

    void CNetBuffer::operator+=(CNetBuffer obj)
    {
        int len = this->m_length;
        if (!this->alloc(this->m_length + obj.m_length))
        {
            return;
        }

        memcpy(this->m_buffer + len, obj.m_buffer, obj.m_length);
    }

    void CNetBuffer::operator-=(size_t size)
    {

        if (this->m_length - size > 0)
        {
            this->alloc(this->m_length - size);
        }
    }

    bool CNetBuffer::is_null()
    {
        return this->m_buffer == nullptr;
    }

    size_t CNetBuffer::size()
    {
        return this->m_length;
    }

    size_t CNetBuffer::alloc_count()
    {
        return this->m_allocCount;
    }

    bool CNetBuffer::alloc(size_t len)
    {
        if (this->m_length == len)
        {
            return true;
        }

        this->m_length = len;
        if (this->m_buffer == nullptr || this->m_allocCount == 0)
        {
            this->m_buffer = (char*)malloc(this->m_length);
        }
        else
        {
            this->m_buffer = (char*)realloc(this->m_buffer, this->m_length);
        }
        this->m_allocCount++;
        return this->m_buffer != nullptr;
    }


}