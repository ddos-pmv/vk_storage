#pragma once
#include <boost/asio.hpp>


using namespace boost::asio;

class Client {
public:
    Client() : socket_(io_context){}

    bool connect(const std::string & host, uint16_t port ) {
        try{ip::tcp::resolver resolver(io_context);}
    }
private:
    ip::tcp::socket socket_;
    io_context context_;
};