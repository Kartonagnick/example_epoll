
#include "protocol.hpp"
#include <unistd.h>      // unix standard library (sleep)
#include <sys/socket.h>  // berkley socket library
#include <arpa/inet.h>   // IP addresses conversion utilities (htons)
#include <stdexcept>
#include <iostream>
#include <random>
#include <chrono>

namespace example
{
    class client
    {
        int m_client_fd;
        sockaddr_in m_serv_add;
        using str_t = example::str_t;
    public:
        client()
            : m_client_fd(-1)
            , m_serv_add()
        {
            this->m_client_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
            if(this->m_client_fd == -1)
                throw std::runtime_error("client(failed): posix-api: socket");
    
            this->m_serv_add.sin_family = AF_INET;      // type of address: IPV4
            this->m_serv_add.sin_port = ::htons(1234);  // port (network byte order) 
            const char* argv = "127.0.0.1";   
            ::inet_pton(AF_INET, argv, &this->m_serv_add.sin_addr);

            struct timeval tv;
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            const char* ctv = reinterpret_cast<const char*>(&tv);
            ::setsockopt(this->m_client_fd, SOL_SOCKET, SO_RCVTIMEO, ctv, sizeof(tv));
        }
    
        void send_command(const str_t& msg)
        {
            std::cout << "-> " << msg << '\n';
            example::udp::send_message(msg, this->m_client_fd, this->m_serv_add);
            if(msg == "/shutdown")
                return;

            const str_t responce = example::udp::read_message(this->m_client_fd, this->m_serv_add);
            if(responce.empty())
                std::cout << "<- (empty message)\n";
            else 
                std::cout << "<- " << responce << '\n';
        }
    };

    std::default_random_engine& rnd_() noexcept
    {
        using clock_type = std::chrono::steady_clock;
        static std::default_random_engine engine(
            static_cast<unsigned>(
                clock_type::now().time_since_epoch().count()
            )
        );
        return engine;
    }

    unsigned rand_value(const unsigned a, const unsigned b)
    {
        using rand_t = std::uniform_int_distribution<unsigned>;
        rand_t gen(a, b);
        return gen(rnd_());
    }

    void sleep()
    {
        ::sleep(example::rand_value(1, 3));
    }

} // namespace example

int main()
{
    try
    {
        std::cout << "[UPD] client: started\n";
        example::client client;

        // 1. mirror
        client.send_command("hello");
        example::sleep();

        // 2. time
        client.send_command("/time");
        example::sleep();

        // 3. stats
        client.send_command("/stats");
        example::sleep();

        // random commands
        std::cout << "---- rand ---\n";
        while(1)
        {
            const unsigned choice = example::rand_value(1, 4);
            if(choice == 1)
                client.send_command("hello");
            else if(choice == 2)
                client.send_command("/time");
            else if(choice == 3)
                client.send_command("/stats");
            else
            {
                client.send_command("/shutdown");
                break;
            }
            example::sleep();
        }
        std::cout << "client: stopped\n";
    }
    catch(const example::disconnect&)
    {
        std::cerr << "client: disconnected\n";
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "client: done\n";
}
