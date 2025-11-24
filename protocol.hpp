
#pragma once
#include <stdexcept>
#include <string>

struct sockaddr_in;

namespace example
{
    using str_t = std::string;
    struct disconnect: public std::runtime_error 
    {
        disconnect(const char* msg)
            : std::runtime_error (msg)
        {}
    };

    namespace tcp
    {
        void send_message(const str_t& message, const int fd);
        str_t read_message(const int fd);

    } // namespace tcp

    namespace udp
    {
        void send_message(const str_t& message, const int fd, const sockaddr_in&);
        str_t read_message(const int fd, sockaddr_in&);

    } // namespace udp

} // namespace example

