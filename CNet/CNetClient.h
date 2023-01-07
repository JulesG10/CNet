#pragma once
#include "framework.h"
#include "CNet.h"

namespace cnet {
    extern class CNET_EXPORTS CNetClient
    {
    public:
        CNetClient(CNetProps);
        CNetClient(CNetProps, CNetSocket, int);

        ~CNetClient();


        CNetStatus start();
        CNetStatus terminate(bool = true);

        CNetStatus send(const char*);
        CNetStatus send(CNetBuffer&, int*);

        CNetStatus recieve(CNetBuffer&);
        CNetStatus recieve(char*, int*);

        CNetStatus get_status(int*);

        bool is_alive();
        int get_id();

        size_t get_queue_size();
        CNetStatus recieve_all(std::vector<CNetBuffer>*);
        CNetStatus set_queue_max(size_t);
    private:
        void wait_data();

        int m_maxQueue;
        CNetProps m_props;
        std::vector<CNetBuffer> m_queue;

        std::mutex m_mutex;
        std::thread m_thread;
        CNetSocket m_socket;
        bool m_running;
        int m_id;
    };

}