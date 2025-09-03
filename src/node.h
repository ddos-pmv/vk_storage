#pragma once

#include <vk/storage.h>

#include <boost/asio.hpp>
#include <cstdint>

using namespace boost::asio;

template<typename Clock = std::chrono::steady_clock>
class Node {
public:
    Node(uint16_t port) : acceptor_(io_context_, ip::tcp::endpoint(ip::tcp::v4(), port)) {}

    void run() {
        start_accept();
        io_context_.run();
    }

private:
    void start_accept() {
        auto socket = std::make_shared<ip::tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code & ec) {
            if( !ec) {
                std::cout << "New connection from: " << socket->remote_endpoint() << std::endl;
            }
            start_accept();
        });
    }
    io_context io_context_;
    ip::tcp::acceptor acceptor_;
    vk::KVStorage<Clock> storage;
};
