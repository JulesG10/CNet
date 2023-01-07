#pragma once
#include "framework.h"
#include "CNet.h"
#include "CNetClient.h"

namespace cnet {

    extern class CNET_EXPORTS CNetServer
    {
    public:
        CNetServer(CNetProps);
        ~CNetServer();

        CNetStatus start();
        CNetStatus terminate();

        int count_clients();
        bool is_alive();

    protected:
        virtual bool on_client_recieve(std::shared_ptr <CNetClient> client, char* buffer, size_t byteread);
        virtual bool on_client_new(std::shared_ptr <CNetClient> client);
        virtual void on_client_quit(std::shared_ptr <CNetClient> client);
        virtual bool on_accept_error(int);

        std::vector<std::shared_ptr<CNetClient>> m_clients;
        std::mutex m_mutex;

        CNetProps m_props;
    private:
        void handle_client(CNetSocket);
        void wait_connections();

        std::shared_ptr<CNetClient> create_client(CNetSocket);
        void remove_client(std::shared_ptr<CNetClient>);

        CNetSocket m_socket;
        std::thread m_thread;
        int m_id;
        bool m_running;
    };
}
