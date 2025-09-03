#pragma once

#include <vk/storage.h>

#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>

using namespace boost::asio;

template<typename Clock = std::chrono::steady_clock>
class Node {
public:
    Node(uint16_t port) : acceptor_(io_context_, ip::tcp::endpoint(ip::tcp::v4(), port)) {}

    void run() {
        std::cout << "Node started on port " << acceptor_.local_endpoint().port() << std::endl;
        start_accept();
        io_context_.run();
    }

private:
    void start_accept() {
        auto socket = std::make_shared<ip::tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& ec) {
            if (!ec) {
                std::cout << "New connection from: " << socket->remote_endpoint() << std::endl;
                start_read(socket);
            }
            start_accept();
        });
    }

    void start_read(std::shared_ptr<ip::tcp::socket> socket) {
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read_until(*socket, *buffer, "\n",
            [this, socket, buffer](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    std::istream stream(buffer.get());
                    std::string line;
                    std::getline(stream, line);

                    std::string response = handle_request(line);

                    auto response_buffer = std::make_shared<std::string>(response + "\n");
                    boost::asio::async_write(*socket, boost::asio::buffer(*response_buffer),
                        [this, socket, response_buffer](const boost::system::error_code&, std::size_t) {
                            start_read(socket);
                        });
                } else {
                    std::cout << "Client disconnected" << std::endl;
                }
            });
    }

    std::string handle_request(const std::string& request) {
        std::istringstream iss(request);
        std::string command;
        iss >> command;

        if (command == "GET") {
            std::string key;
            if (iss >> key) {
                if (auto value = storage_.get(key)) {
                    return "OK " + *value;
                } else {
                    return "NOT_FOUND";
                }
            }
            return "ERROR Invalid GET format";
        }
        else if (command == "SET") {
            std::string key, value;
            uint32_t ttl = 0;
            if (iss >> key >> value) {
                iss >> ttl;
                storage_.set(key, value, ttl);
                return "OK";
            }
            return "ERROR Invalid SET format";
        }
        else if (command == "REMOVE") {
            std::string key;
            if (iss >> key) {
                if (storage_.remove(key)) {
                    return "OK";
                } else {
                    return "NOT_FOUND";
                }
            }
            return "ERROR Invalid REMOVE format";
        }

        return "ERROR Unknown command";
    }

    io_context io_context_;
    ip::tcp::acceptor acceptor_;
    vk::KVStorage<Clock> storage_;
};