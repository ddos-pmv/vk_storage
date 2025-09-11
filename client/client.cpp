#include "client.h"

#include <boost/asio.hpp>

#include <format>


bool Client::connect(const std::string &host, uint16_t port) {
    try {
        ip::tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host, std::to_string(port));
        boost::asio::connect(socket_, endpoints);
        std::cout << "Connected to " << host << ":" << port << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return false;
    }
}

void Client::link(const std::string &ip, const std::string &port) {
    std::string request = std::format("LINK {} {}\n",ip, port);
    send_request(request);
}

void Client::send_get(const std::string &key) {
    std::string request = std::format( "GET {}\n", key );
    send_request(request);
}


void Client::send_set(const std::string &key, const std::string &value, uint32_t ttl) {
    std::string request = std::format("SET {} {} {}\n",key, value, std::to_string(ttl));
    send_request(request);
}


void Client::send_remove(const std::string &key) {
    std::string request = std::format("REMOVE {}\n", key);
    send_request(request);
}


void Client::send_request(const std::string &request) {
    if (!socket_.is_open()) {
        std::cout << "Not connected to server" << std::endl;
        return;
    }

    try {
        boost::asio::write(socket_, boost::asio::buffer(request));

        boost::asio::streambuf response_buf;
        boost::asio::read_until(socket_, response_buf, "\n");

        std::istream response_stream(&response_buf);
        std::string response;
        std::getline(response_stream, response);
        std::cout << "Response: " << response << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Request failed: " << e.what() << std::endl;
    }
}
