#pragma once

#include <vk/storage.h>

#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

using namespace boost::asio;

template<typename Clock = std::chrono::steady_clock>
class Node {
public:
    Node(uint16_t port, size_t thread_count = std::thread::hardware_concurrency()) :
        acceptor_(io_context_, ip::tcp::endpoint(ip::tcp::v4(), port)), thread_count_(thread_count) {}

    void run() {
        std::cout << "Node started on port " << acceptor_.local_endpoint().port() << " with " << thread_count_
                  << " threads" << std::endl;

        start_accept();

        // Запускаем пул потоков для io_context
        std::vector<std::thread> threads;
        threads.reserve(thread_count_);

        for (size_t i = 0; i < thread_count_; ++i) {
            threads.emplace_back([this] { io_context_.run(); });
        }

        // Ждем завершения всех потоков
        for (auto &thread: threads) {
            thread.join();
        }
    }

private:
    void start_accept() {
        auto socket = std::make_shared<ip::tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code &ec) {
            if (!ec) {
                std::cout << "New connection from: " << socket->remote_endpoint()
                          << " [thread: " << std::this_thread::get_id() << "]" << std::endl;
                handle_client(socket);
            }
            start_accept();
        });
    }

    void handle_client(std::shared_ptr<ip::tcp::socket> socket) {
        // Каждый клиент обрабатывается в отдельном потоке из пула
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read_until(
                *socket, *buffer, "\n", [this, socket, buffer](const boost::system::error_code &ec, std::size_t) {
                    if (!ec) {
                        std::istream stream(buffer.get());
                        std::string line;
                        std::getline(stream, line);

                        // Обработка запроса может происходить в любом потоке
                        std::string response = handle_request(line);

                        auto response_buffer = std::make_shared<std::string>(response + "\n");
                        boost::asio::async_write(*socket, boost::asio::buffer(*response_buffer),
                                                 [this, socket, response_buffer](
                                                         const boost::system::error_code &write_ec, std::size_t) {
                                                     if (!write_ec) {
                                                         handle_client(socket); // Продолжаем читать от этого клиента
                                                     } else {
                                                         std::cout << "Client disconnected" << std::endl;
                                                     }
                                                 });
                    } else {
                        std::cout << "Client disconnected" << std::endl;
                    }
                });
    }

    std::string handle_request(const std::string &request) {
        std::istringstream iss(request);
        std::string command;
        iss >> command;
        if (command == "LINK") {
            std::string ip;
            uint16_t port;
            if (iss >> ip >> port) {
                if (link(ip, port)) {
                    return std::format("OK LINKED {} {}", ip, port);
                } else {
                    return std::format("NOT LINKED TO {} {}", ip, port);
                }
            } else {
                return "ERROR Invalid LINK format";
            }
        } else if (command == "GET") {
            std::string key;
            if (iss >> key) {
                if (auto value = storage_.get(key)) {
                    return "OK " + *value;
                } else {
                    return "NOT_FOUND";
                }
            }
            return "ERROR Invalid GET format";
        } else if (command == "SET") {
            std::string key, value;
            uint32_t ttl = 0;
            if (iss >> key >> value) {
                iss >> ttl;
                storage_.set(key, value, ttl);
                return "OK";
            }
            return "ERROR Invalid SET format";
        } else if (command == "REMOVE") {
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

    bool link(const std::string &ip, uint16_t port) {
        std::unique_lock lock(peers_mtx_);
        auto endpoint = ip::tcp::endpoint(ip::address_v4::from_string(ip), port);

        if (std::any_of(peers_.begin(), peers_.end(), [&](const auto &p) { return p.ep == endpoint; }))
            return false;

        peers_.emplace_back(endpoint, nullptr);
        return true;
    }

    io_context io_context_;
    ip::tcp::acceptor acceptor_;
    vk::KVStorage<Clock> storage_;
    size_t thread_count_;

    struct Peer {
        ip::tcp::endpoint ep;
        std::shared_ptr<ip::tcp::socket> socket;
    };

    std::mutex peers_mtx_;
    std::vector<Peer> peers_;
};