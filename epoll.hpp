
#pragma once

struct epoll_event;

namespace example
{
    class epoll;
    class thread_pool;

    class logic
    {
        friend class epoll;
        epoll* m_epoll;
    public:
        logic(const logic&)            = delete;
        logic& operator=(const logic&) = delete;
    public:
        logic() noexcept: m_epoll() {}

        virtual ~logic(){}
        virtual void on_connected(const int id)    = 0;
        virtual void on_disconnected(const int id) = 0;
        virtual bool on_message_tcp(const int id)  = 0;
        virtual void on_message_udp(const int id)  = 0;
        void shutdown();
    };

    class epoll
    {
    public:
        epoll(const epoll&)            = delete;
        epoll& operator=(const epoll&) = delete;
    public:
       ~epoll();
        epoll(logic&);
        void stop();
        void run();
    private:
        void add_new_clients();
        void handle_client(const epoll_event&);
        void shutdown();
    private:
        int m_socket_tcp;
        int m_socket_udp;
        int m_stop_event;
        int m_epoll_fd;
        logic*       m_logic;
        thread_pool* m_pool;
    };

} // namespace example
