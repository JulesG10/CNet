#include "pch.h"
#include "CNetClient.h"

namespace cnet
{



    CNetClient::CNetClient(CNetProps props)
    {
        this->m_props = props;
        this->m_id = -1;
        this->m_running = false;
        this->m_socket = INVALID_SOCKET;
    }

    CNetClient::CNetClient(CNetProps props, CNetSocket socket, int id)
    {
        this->m_props = props;
        this->m_socket = socket;
        this->m_id = id;
        this->m_maxQueue = this->m_queue.max_size();

        this->m_running = true;
    }

    CNetClient::~CNetClient()
    {
    }

    CNetStatus CNetClient::start()
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            return CNetStatus::CNET_ERROR;
        }

        this->m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (this->m_socket == INVALID_SOCKET)
        {
            return CNetStatus::CNET_ERROR;
        }

        if (this->m_props.non_blocking)
        {
            u_long iMode = 0;
            if (ioctlsocket(this->m_socket, FIONBIO, &iMode) != NOERROR)
            {
                return CNetStatus::CNET_ERROR;
            }
        }

        if (connect(this->m_socket, (const sockaddr*)&this->m_props.addr, sizeof(this->m_props.addr)) == INVALID_SOCKET)
        {
            return CNetStatus::CNET_ERROR;
        }

        this->m_running = true;
        this->m_thread = std::thread(&CNetClient::wait_data, this);
        this->m_thread.detach();

        return CNetStatus::CNET_OK;
    }

    CNetStatus CNetClient::terminate(bool cleanup)
    {
        if (!this->m_running)
        {
            return CNetStatus::CNET_ERROR;
        }

        this->m_running = false;
        this->m_queue.clear();

        closesocket(this->m_socket);
        if (cleanup)
        {
            WSACleanup();
        }


        return CNetStatus::CNET_ERROR;
    }


    CNetStatus CNetClient::send(const char* str)
    {
        int w = 0;
        CNetBuffer buffer(str);
        CNetStatus status = this->send(buffer, &w);
        buffer.clear();
        return status;
    }

    CNetStatus CNetClient::send(CNetBuffer& buffer, int* written)
    {
        *written = 0;
        int status = 0;
        if (this->get_status(&status) != CNetStatus::CNET_OK || buffer.is_null() || buffer.size() == 0)
        {
            return CNetStatus::CNET_ERROR;
        }

        *written = write(this->m_socket, buffer, buffer.size(), 0);
        if (*written == SOCKET_ERROR)
        {
            return CNetStatus::CNET_ERROR;
        }

        return CNetStatus::CNET_OK;
    }

    CNetStatus CNetClient::recieve(CNetBuffer& buffer)
    {
        this->m_mutex.lock();
        if (this->m_queue.size() > 0)
        {
            buffer = this->m_queue[0];
            this->m_queue.erase(this->m_queue.begin());
            this->m_mutex.unlock();
            return CNetStatus::CNET_OK;
        }
        this->m_mutex.unlock();
        return CNetStatus::CNET_ERROR;
    }

    CNetStatus CNetClient::recieve(char* buff, int* read)
    {
        *read = recv(this->m_socket, buff, this->m_props.packet_size, 0);
        return CNetStatus::CNET_OK;
    }

    bool CNetClient::is_alive()
    {
        return this->m_running;
    }

    int CNetClient::get_id()
    {
        return this->m_id;
    }

    size_t CNetClient::get_queue_size()
    {
        return this->m_queue.size();
    }

    CNetStatus CNetClient::recieve_all(std::vector<CNetBuffer>* out)
    {
        if (this->m_queue.size() == 0)
        {
            return CNetStatus::CNET_ERROR;
        }
        this->m_mutex.lock();
        *out = this->m_queue;
        this->m_queue.clear();
        this->m_queue.shrink_to_fit();
        this->m_mutex.unlock();
        return CNetStatus::CNET_OK;
    }

    CNetStatus CNetClient::set_queue_max(size_t m)
    {
        if (m < this->m_queue.max_size())
        {
            this->m_maxQueue = m;
            return CNetStatus::CNET_OK;
        }
        return CNetStatus::CNET_ERROR;
    }

    void CNetClient::wait_data()
    {
        this->m_running = true;

        CNetBuffer buffer;
        
        char* tmp = new char[this->m_props.packet_size];
        while (this->m_running)
        {
            int byteread = recv(this->m_socket, tmp, this->m_props.packet_size, 0);
            int status = 0;
            if (this->get_status(&status) == CNetStatus::CNET_ERROR || byteread <= 0 || byteread > this->m_props.packet_size)
            {
                delete[] tmp;
                this->terminate();
                return;
            }

            CNetBuffer recieved = CNetBuffer(tmp, byteread);
            buffer += recieved;
            recieved.clear();

            if (byteread < this->m_props.packet_size)
            {
                CNetBuffer copy;
                copy.copy(buffer);

                this->m_mutex.lock();
                this->m_queue.push_back(copy);
                if (this->m_queue.size() > this->m_maxQueue)
                {
                    this->m_queue.erase(this->m_queue.begin());
                }
                this->m_mutex.unlock();

                buffer.clear();
            }
        }

        if (tmp != nullptr)
        {
            delete[] tmp;
        }
    }

    CNetStatus CNetClient::get_status(int* error)
    {
        if (!this->m_running)
        {
            return CNetStatus::CNET_ERROR;
        }

        *error = 0;
        int len = sizeof(*error);

        int optres = getsockopt(this->m_socket, SOL_SOCKET, SO_ERROR, (char*)error, &len);
        if (optres == SOCKET_ERROR || *error != 0)
        {
            return CNetStatus::CNET_ERROR;
        }

        return CNetStatus::CNET_OK;
    }
}