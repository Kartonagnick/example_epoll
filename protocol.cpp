
#include "protocol.hpp"
#include <sys/socket.h>  // berkley socket library
#include <arpa/inet.h>   // IP addresses conversion utilities (htons)
#include <cstring>       // memcpy
#include <cassert>
#include <iostream>

//==============================================================================
//==============================================================================

namespace example
{
    namespace tcp
    {
        static size_t write_n(const int fd, const void* buff, size_t n)
        {
            const char* buf = reinterpret_cast<const char*>(buff);
            size_t  nw = 0;
            ssize_t nx = 0;
            while (nw < n) 
            {
                nx = ::send(fd, buf + nw, n - nw, MSG_NOSIGNAL);
                if(nx < 0 && errno == EINTR)
                {
                    std::cerr << "[EINTR]" << std::endl;
                    continue;
                }
                if (nx <= 0)
                    break;
                nw += static_cast<size_t>(nx);
            }
            if(nw == 0 && nx < 0)
            {
                if(errno == EPIPE)
                    throw example::disconnect("example::tcp::write_n(failed): posix-api: send");
                throw std::runtime_error("example::tcp::write_n(failed): posix-api: send");
            }
            return nw;
        }

    } // namespace tcp

} // namespace example

//==============================================================================
//=== TCP ======================================================================

namespace example
{
    namespace tcp
    {
        void send_message(const str_t& message, const int fd)
        {
            const char* msg = message.c_str();
            const size_t len = message.size();
            const size_t n = write_n(fd, msg, len);
            assert(n == len);
            if(n != len)
                throw std::runtime_error("example::tcp::send_message(failed): internal error: write_n");
        }

        str_t read_message(const int fd)
        {
            ssize_t n = -1;
            char buf[256];
            for(;;)
            {
                n = ::recv(fd, buf, 256, MSG_NOSIGNAL);
                if(n < 0)
                {
                    if(errno == EINTR)
                        continue;
                    if(errno == EPIPE)
                        throw example::disconnect("example::tcp::read_n(failed): posix-api: recv");
                    throw std::runtime_error("example::tcp::read_n(failed): posix-api: recv");
                }
                break;
            }
            if(n == 0)
                throw example::disconnect("example::tcp::read_n(disconnect): posix-api: recv");

            buf[n] = 0;
            return buf;
        }
        
    } // namespace tcp

} // namespace example

//==============================================================================
//=== UDP ======================================================================

namespace example
{
    namespace udp
    {
        void send_message(const str_t& message, const int fd, const sockaddr_in& serv_add)
        {
            const char* buf = message.c_str();
            const size_t n = message.size();
            const sockaddr* addr = reinterpret_cast<const sockaddr*>(&serv_add);
            ::sendto(fd, buf, n, 0, addr, sizeof(sockaddr_in));
        }

        str_t read_message(const int fd, sockaddr_in& serv_add)
        {
            char buf[256];
            sockaddr* addr = reinterpret_cast<sockaddr*>(&serv_add);
            socklen_t addrlen = sizeof(sockaddr_in);
            const ssize_t n = ::recvfrom(fd, buf, 256, 0, addr, &addrlen);
            if(n < 0)
            {
                if(errno == EAGAIN)
                    throw example::disconnect("example::udp::read_message(failed): timeout");
                return "";
            }
            buf[n] = 0;
            return buf; 
        }

    } // namespace udp

} // namespace example
