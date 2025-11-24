
#include "protocol.hpp"
#include "epoll.hpp"
#include <netinet/in.h>
#include <iostream>
#include <cassert>
#include <ctime>

using example::str_t;

// trim from end of string (right)
inline str_t& rtrim(str_t& s, const char* t = " \t\n\r\f\v")
{
    assert(t);
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
inline str_t& ltrim(str_t& s, const char* t = " \t\n\r\f\v")
{
    assert(t);
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (left and right)
inline str_t& trim(str_t& s, const char* t = " \t\n\r\f\v")
{
    assert(t);
    return ltrim(rtrim(s, t), t);
}

class sample: public example::logic
{
    size_t m_users;
    size_t m_total;

    str_t make_responce(const str_t& msg)
    {
        if(msg.empty())
            return "";
        if(msg == "/time")
        {
            char buf[20];
            const std::time_t cur = std::time(0); 
            const std::tm* curtm = std::localtime(&cur);
            const char* f = "%Y-%m-%d %H:%M:%S";
            constexpr size_t sz = sizeof(buf);            
            const size_t count = std::strftime(buf, sz, f, curtm);
            assert(count == 19);
            (void) count;
            return buf;
        }
        else if(msg == "/stats")
        {
            const str_t txt_users = std::to_string(this->m_users);
            const str_t txt_total = std::to_string(this->m_total);
            const str_t result = "connected: " + txt_users + ", total: " + txt_total; 
            return result;
        }
        return msg;
    }

public:
    sample() noexcept
        : m_users()
        , m_total()
    {}

    void on_connected(const int id) override
    {
        std::cout << "[SERVER] connected: #ID-" << id << '\n'; 
        ++this->m_users;
        ++this->m_total;
    }

    void on_disconnected(const int id) override
    {
        std::cout << "[SERVER] disconnected: #ID-" << id << '\n'; 
        --this->m_users;
    }

    bool on_message_tcp(const int id) override
    {
        std::cout << "[SERVER][TCP] received from: #ID-" << id << '\n'; 
        str_t msg = example::tcp::read_message(id);
        trim(msg);
        if(msg.empty())
        {
            std::cout << "[#ID-" << id << "][TCP] empty message\n"; 
            return false;
        }
        std::cout << "[#ID-" << id << "][TCP] '" << msg << "'\n";
        if(msg == "/shutdown")
            this->shutdown();
        else
        {
            const str_t answer = make_responce(msg);
            if(answer.empty())
                return false;
            std::cout << "[SERVER][#ID-" << id << "][TCP] -> " << answer << '\n';
            example::tcp::send_message(answer, id); 
        }
        return true;
    }

    virtual void on_message_udp(const int id)
    {
        static bool increment((++this->m_total, true));
        (void) increment;

        std::cout << "[SERVER][UDP] received from: #ID-" << id << '\n'; 
        struct sockaddr_in client_add {};
        str_t msg = example::udp::read_message(id, client_add);
        trim(msg);
        if(msg.empty())
        {
            std::cout << "[#ID-" << id << "][UDP] empty message\n"; 
            return;
        }
        else
            std::cout << "[#ID-" << id << "][UDP] '" << msg << "'\n";

        if(msg == "/shutdown")
            this->shutdown();
        else
        {
            ++this->m_users;
            const str_t answer = make_responce(msg);
            --this->m_users;

            if(answer.empty())
                return;
            std::cout << "[SERVER][#ID-" << id << "][UDP] -> " << answer << '\n';
            example::udp::send_message(answer, id, client_add); 
        }
    }
};

int main()
{
    try
    {
        std::cout << "server: started\n";
        sample logic;
        example::epoll dev(logic);
        dev.run();
        std::cout << "server: stopped" << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}
