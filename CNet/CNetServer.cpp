#include "pch.h"
#include "CNetServer.h"

namespace cnet
{

    CNetServer::CNetServer(CNetProps props)
    {
        this->m_props = props;
        this->m_socket = INVALID_SOCKET;
        this->m_running = false;
        this->m_id = 0;
    }

    CNetServer::~CNetServer()
    {
    }

    CNetStatus CNetServer::start()
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
        {
            return CNetStatus::CNET_ERROR;
        }

        this->m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (this->m_socket == SOCKET_ERROR)
        {
            return CNetStatus::CNET_ERROR;
        }

        if (this->m_props.non_blocking)
        {
            u_long iMode = 0;
            if (ioctlsocket(m_socket, FIONBIO, &iMode) != NOERROR)
            {
                return CNetStatus::CNET_ERROR;
            }
        }
        int opt = 1;
        if (setsockopt(this->m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR)
        {
            return CNetStatus::CNET_ERROR;
        }

        if (bind(this->m_socket, (const sockaddr*)&this->m_props.addr, sizeof(this->m_props.addr)) == SOCKET_ERROR)
        {
            return CNetStatus::CNET_ERROR;
        }

        if (listen(this->m_socket, SOMAXCONN) == SOCKET_ERROR)
        {
            return CNetStatus::CNET_ERROR;
        }

        this->m_running = true;
        this->m_thread = std::thread(&CNetServer::wait_connections, this);
        this->m_thread.detach();

        return CNetStatus::CNET_OK;
    }

    CNetStatus CNetServer::terminate()
    {
        this->m_running = false;
        if (this->m_thread.joinable())
        {
            this->m_thread.join();
        }

        if (this->m_socket != INVALID_SOCKET)
        {
            shutdown(this->m_socket, SD_BOTH);
            closesocket(this->m_socket);
        }

        if (WSACleanup() == SOCKET_ERROR)
        {
            return CNetStatus::CNET_ERROR;
        }

        return CNetStatus::CNET_OK;
    }

    int CNetServer::count_clients()
    {
        return this->m_clients.size();
    }

    bool CNetServer::is_alive()
    {
        return this->m_running;
    }

    bool CNetServer::on_client_new(std::shared_ptr<CNetClient> client)
    {
        return true;
    }

    void CNetServer::on_client_quit(std::shared_ptr<CNetClient> client)
    {
    }

    bool CNetServer::on_accept_error(int)
    {
        return false;
    }

    bool CNetServer::on_client_recieve(std::shared_ptr<CNetClient> client, char* buffer, size_t byteread)
    {
        if (this->m_clients.size() > 1)
        {
            this->m_mutex.lock();
            for (std::shared_ptr<CNetClient> cl : this->m_clients)
            {
                if (cl->get_id() != client->get_id())
                {
                    int written = 0;
                    CNetBuffer data(buffer, byteread);
                    cl->send(data, &written);
                    data.clear();
                }
            }
            this->m_mutex.unlock();
        }
        return true;
    }


    void CNetServer::wait_connections()
    {
        while (this->m_running)
        {
            CNetSocket socket = accept(this->m_socket, NULL, NULL);
            if (socket == INVALID_SOCKET)
            {
                if (!this->on_accept_error(WSAGetLastError()))
                {
                    break;
                }
            }
            else {
                std::thread(&CNetServer::handle_client, this, socket).detach();
            }
        }
        this->terminate();
    }

    void CNetServer::handle_client(CNetSocket socket)
    {
        std::shared_ptr<CNetClient> client = this->create_client(socket);

        if (!this->on_client_new(client))
        {
            this->on_client_quit(client);
            this->remove_client(client);
            return;
        }

        while (this->m_running || !client->is_alive())
        {
            char* buffer = new char[this->m_props.packet_size];
            int byteread = 0;

            if (client->recieve(buffer, &byteread) == CNetStatus::CNET_ERROR)
            {
                break;
            }

            int status = 0;
            if (client->get_status(&status) == CNetStatus::CNET_ERROR || byteread < 1 || byteread > this->m_props.packet_size)
            {
                break;
            }

            if (!this->on_client_recieve(client, buffer, byteread))
            {
                delete[] buffer;
                break;
            }

            delete[] buffer;
        }

        this->on_client_quit(client);
        this->remove_client(client);
    }

    std::shared_ptr<CNetClient> CNetServer::create_client(CNetSocket socket)
    {
        this->m_mutex.lock();
        this->m_id++;
        std::shared_ptr<CNetClient> client = std::make_shared<CNetClient>(this->m_props, socket, this->m_id);
        this->m_clients.push_back(client);
        this->m_mutex.unlock();

        return client;
    }

    void CNetServer::remove_client(std::shared_ptr<CNetClient> client)
    {
        this->m_mutex.lock();
        this->m_clients.erase(std::remove(this->m_clients.begin(), this->m_clients.end(), client), this->m_clients.end());
        this->m_mutex.unlock();

        client->terminate(false);
    }
}