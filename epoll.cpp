
#include "epoll.hpp"
#include <sys/epoll.h>   // epoll (epoll_create1)
#include <sys/eventfd.h> // eventfd
#include <arpa/inet.h>   // IP addresses conversion utilities (htons)
#include <unistd.h>      // unix standard library (close)
#include <fcntl.h>       // used in: set_non_blocking
#include "tpool.hpp"     // example::thread_pool
#include "protocol.hpp"  // example::disconnect
#include <stdexcept>
#include <iostream>
#include <cassert>

//==============================================================================
//=== staff ====================================================================

namespace example
{
    static int make_socket_tcp_()
    {
        const int socket_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socket_fd == -1)
            throw std::runtime_error("example::make_socket_tcp_(failed): posix-api: socket");
        return socket_fd;
    }

    static int make_socket_upd_()
    {
        const int socket_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if(socket_fd == -1)
            throw std::runtime_error("example::make_socket_upd_(failed): posix-api: socket");

        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        const char* ctv = reinterpret_cast<const char*>(&tv);
        ::setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, ctv, sizeof(tv));
        return socket_fd;
    }

    static int make_stop_event_()
    {
        const int event_fd = ::eventfd(0, EFD_NONBLOCK);
        if (event_fd == -1)
            throw std::runtime_error("example::make_stop_event_(failed): posix-api: eventfd");
        return event_fd;
    }

    static void set_reuseopt_(const int socket_fd)
    {
        int val = 1;
        const int ret_opt1 = ::setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
        if(ret_opt1 == -1)
            throw std::runtime_error("example::set_reuseopt_(failed): posix-api: setsockopt -1-");

        const int ret_opt2 = ::setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));
        if(ret_opt2 == -1)
            throw std::runtime_error("example::set_reuseopt_(failed): posix-api: setsockopt -2-");
    }

    static void set_non_blocking_(const int sock_fd, bool on = true)
    {
        int flags = ::fcntl(sock_fd, F_GETFL, 0);
        if(flags == -1)
            throw std::runtime_error("example::set_non_blocking_(failed): posix-api: fcntl -1-");

        flags = on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
        const int ret = ::fcntl(sock_fd, F_SETFL, flags);
        if(ret == -1)
            throw std::runtime_error("example::set_non_blocking_(failed): posix-api: fcntl -2-");
    }

    static void bind_(const int socket_fd)
    {
        struct sockaddr_in serv_add {}; 
        serv_add.sin_family = AF_INET;                   // type of address: IPV4
        serv_add.sin_port = ::htons(1234);               // port (network byte order) 
        serv_add.sin_addr.s_addr = ::htonl(INADDR_ANY);  // connect from anywhere
        sockaddr* addr = reinterpret_cast<sockaddr*>(&serv_add);
        const int ret_bind = ::bind(socket_fd, addr, sizeof(sockaddr_in));
        if(ret_bind == -1)
            throw std::runtime_error("example::bind_(failed): posix-api: bind");
    }

    static void listen_(const int socket_fd)
    {
        constexpr size_t backlog = 10;
        const int ret_listen = ::listen(socket_fd, backlog);
        if(ret_listen == -1)
            throw std::runtime_error("example::listen_(failed): posix-api: listen");
    }

    static int make_epoll_(const int socket_tcp, const int socket_udp, const int stop_event)
    {
        const int epoll_fd = epoll_create1(0);
        if(epoll_fd == -1)
            throw std::runtime_error("example::make_epoll_(failed): posix-api: epoll_create1");

        struct epoll_event ev {};
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = socket_tcp;
        const int ret_ctl_1 = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_tcp, &ev);
        if(ret_ctl_1 == -1)
            throw std::runtime_error("example::make_epoll_(failed): posix-api: epoll_ctl -1-");

        ev.data.fd = socket_udp;
        const int ret_ctl_2 = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_udp, &ev);
        if(ret_ctl_2 == -1)
            throw std::runtime_error("example::make_epoll_(failed): posix-api: epoll_ctl -2-");

        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = stop_event;
        const int ret_ctl = ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, stop_event, &ev);
        if(ret_ctl == -1)
            throw std::runtime_error("example::make_epoll_(failed): posix-api: epoll_ctl -3-");

        return epoll_fd;
    }

} // namespace example

//==============================================================================
//=== logic ====================================================================

namespace example
{
    void logic::shutdown() 
    {
        this->m_epoll->stop();
    }

} // namespace example

//==============================================================================
//=== epoll ====================================================================

namespace example
{
    epoll::~epoll()
    {
        assert(this->m_pool);
        delete this->m_pool;
    }

    epoll::epoll(logic& agent)
        : m_socket_tcp(example::make_socket_tcp_())
        , m_socket_udp(example::make_socket_upd_())
        , m_stop_event(example::make_stop_event_())
        , m_epoll_fd(-1)
        , m_logic(&agent)
        , m_pool(new thread_pool)
    {
        example::set_reuseopt_(this->m_socket_tcp);
        example::bind_(this->m_socket_tcp);
        example::listen_(this->m_socket_tcp);
        example::set_non_blocking_(this->m_socket_tcp);

        example::set_reuseopt_(this->m_socket_udp);
        example::bind_(this->m_socket_udp);
        example::set_non_blocking_(this->m_socket_udp);

        this->m_epoll_fd = example::make_epoll_(
            this->m_socket_tcp, 
            this->m_socket_udp,
            this->m_stop_event
        );
    
        agent.m_epoll = this;
    }

    void epoll::shutdown()
    {
        ::shutdown(this->m_socket_tcp, SHUT_RDWR);
        ::shutdown(this->m_socket_udp, SHUT_RDWR);
        ::close(this->m_socket_tcp); 
        ::close(this->m_socket_udp); 
        ::close(this->m_epoll_fd); 
        this->m_socket_tcp = -1; 
        this->m_socket_udp = -1; 
        this->m_epoll_fd   = -1; 
    }

    void epoll::stop()
    {
        const int ret = ::eventfd_write(this->m_stop_event, 1);
        if(ret == -1)
            throw std::runtime_error("epoll.stop(failed): posix-api: eventfd_write");
    }

    void epoll::run()
    {
        std::cout << "epoll: runned\n";

        constexpr size_t max_events = 10;
        struct epoll_event events[max_events];

        bool stop = false;
        while (!stop) 
        { 
            if(this->m_epoll_fd == -1)
                break;
            std::cout << "epoll: wait...\n";
            const int nfds = ::epoll_wait(this->m_epoll_fd, events, max_events, -1);
            if(nfds == -1)
                throw std::runtime_error("epoll.run(failed): posix-api: epoll_wait");

            for (int i = 0; i < nfds; ++i) 
            {
                const epoll_event& cur_event = events[i];
                const int fd = cur_event.data.fd;
                if (fd == this->m_stop_event)
                {
                    std::cout << "-------------------((stopped))" << std::endl;
                    stop = true;
                    break;
                }
                else if (fd == this->m_socket_tcp)
                    this->add_new_clients();
                else if (fd == this->m_socket_udp)
                {
                    const auto lambda = [this, fd]
                    {
                        try
                        {
                            assert(this->m_logic);
                            this->m_logic->on_message_udp(fd);
                        }
                        catch(const example::disconnect& e)
                        {
                            std::cerr << "udp-client: disconnected: " << e.what() << '\n';
                        }
                        catch(const std::exception& e)
                        {
                            std::cerr << "[ERROR] epoll::run(std::exception): " 
                                      << e.what() << '\n';
                        } 
                    };
                    assert(this->m_pool);
                    std::cout << "epoll.run: add user-task\n";
                    this->m_pool->add(std::move(lambda));
                }
                else
                    this->handle_client(cur_event);
            }
        } 
        std::cout << "epoll: done\n";
        this->shutdown();
    }

    void epoll::add_new_clients()
    {
        while(1)
        {
            struct sockaddr_in address {};
            socklen_t addrlen = sizeof(address);
            sockaddr* addr = reinterpret_cast<sockaddr*>(&address);
            const int client_fd = accept(this->m_socket_tcp, addr, &addrlen);
            if(client_fd == -1)
            {
                if(errno == EAGAIN)
                    break;
                throw std::runtime_error("epoll.add_new_clients(failed): posix-api: accept4");
            }

            assert(this->m_logic);
            this->m_logic->on_connected(client_fd); 

            struct epoll_event ev {};
            ev.data.fd = client_fd;
            ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLET;
            const int code = epoll_ctl(this->m_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
            if(code == -1)
                throw std::runtime_error("epoll.add_new_clients(failed): posix-api: epoll_ctl");
        }
    }

    void epoll::handle_client(const epoll_event& ev)
    {
        assert(this->m_pool);
        const int fd = ev.data.fd;

        const auto close_task = [this, fd]
        {
            try
            {
                ::close(fd);
                assert(this->m_logic);
                this->m_logic->on_disconnected(fd);
            }
            catch(const std::exception& e)
            {
                std::cerr << "[ERROR] epoll::handle_client(std::exception): -1- " 
                          << e.what() << '\n';
            } 
        };

        if (ev.events & EPOLLHUP)
        {
            std::cout << "epoll.handle_client(1): add close-task\n";
            this->m_pool->add(std::move(close_task));
        }
        else if (ev.events & EPOLLRDHUP)
        {
            std::cout << "epoll.handle_client(2): add close-task\n";
            this->m_pool->add(std::move(close_task));
        }
        else if (ev.events & EPOLLIN)
        {
            const auto lambda = [this, fd]
            {
                try
                {
                    if(!this->m_logic->on_message_tcp(fd))
                        ::close(fd);
                }
                catch(const example::disconnect& e)
                {
                    std::cerr << "client: disconnected: " << e.what() << '\n';
                }
                catch(const std::exception& e)
                {
                    std::cerr << "[ERROR] epoll::handle_client(std::exception): " 
                              << e.what() << '\n';
                } 
            };
            std::cout << "epoll.handle_client: add user-task\n";
            this->m_pool->add(std::move(lambda));
        }
        else 
        {
            assert(false && "epoll::handle_client");
        }
    }

} // namespace example
