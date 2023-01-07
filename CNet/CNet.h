#pragma once
#include "framework.h"

namespace cnet {
    typedef struct CNetProps
    {
        sockaddr_in addr;
        unsigned short packet_size;
        bool non_blocking;
    } CNetProps;

    typedef enum CNetStatus
    {
        CNET_NONE = -1,
        CNET_OK,
        CNET_ERROR
    } CNetStatus;

    extern CNET_EXPORTS int write(CNetSocket socket, const char* buff, int len, int flags);
    extern CNET_EXPORTS char* cnet_localhost();
    extern CNET_EXPORTS char* cnet_inaddr_any();
    extern CNET_EXPORTS CNetProps cnet_create_props(char* host, unsigned short int port);

    extern class CNET_EXPORTS CNetBuffer
    {
    public:
        CNetBuffer(size_t);

        template<typename T>
        CNetBuffer(T*);

        CNetBuffer(const char*);
        CNetBuffer(char*, size_t);

        CNetBuffer();
        ~CNetBuffer();
    
        bool resize(size_t);
        void clear();

        void copy(CNetBuffer);

        char& operator[](int i);

        void operator = (CNetBuffer);


        CNetBuffer operator + (CNetBuffer);

        void operator += (char);
        void operator += (CNetBuffer);

        void operator -= (size_t);

        template<typename T>
        operator T* ();

        bool is_null();
        size_t size();
        size_t alloc_count();
    private:
        bool alloc(size_t);

        char* m_buffer = nullptr;
        size_t m_length = 0;
        size_t m_allocCount = 0;
    };

    template<typename T>
    inline CNetBuffer::CNetBuffer(T* obj)
    {
        if (this->alloc(sizeof(T)))
        {
            memcpy(this->m_buffer, obj, this->m_length);
        }
    }

    template<typename T>
    inline CNetBuffer::operator T* ()
    {
        return static_cast<T*>((void*)this->m_buffer);
    }
}



