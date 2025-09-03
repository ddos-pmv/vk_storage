# pragma once
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>
#include <string>

using namespace boost::asio;

class Client {
public:
    Client() : socket_(io_context_) {}

    bool connect(const std::string& host, uint16_t port) {
        try {
            ip::tcp::resolver resolver(io_context_);
            auto endpoints = resolver.resolve(host, std::to_string(port));
            boost::asio::connect(socket_, endpoints);
            std::cout << "Connected to " << host << ":" << port << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Connection failed: " << e.what() << std::endl;
            return false;
        }
    }

    void run_cli() {
        std::string line;
        std::cout << "Enter commands (connect <ip> <port>, get <key>, set <key> <value> [ttl], remove <key>, quit):" << std::endl;

        while (std::getline(std::cin, line)) {
            std::istringstream iss(line);
            std::string command;
            iss >> command;

            if (command == "quit") {
                break;
            } else if (command == "connect") {
                std::string ip;
                uint16_t port;
                if (iss >> ip >> port) {
                    connect(ip, port);
                } else {
                    std::cout << "Usage: connect <ip> <port>" << std::endl;
                }
            } else if (command == "get") {
                std::string key;
                if (iss >> key) {
                    send_get(key);
                } else {
                    std::cout << "Usage: get <key>" << std::endl;
                }
            } else if (command == "set") {
                std::string key, value;
                uint32_t ttl = 0;
                if (iss >> key >> value) {
                    iss >> ttl;
                    send_set(key, value, ttl);
                } else {
                    std::cout << "Usage: set <key> <value> [ttl]" << std::endl;
                }
            } else if (command == "remove") {
                std::string key;
                if (iss >> key) {
                    send_remove(key);
                } else {
                    std::cout << "Usage: remove <key>" << std::endl;
                }
            } else {
                std::cout << "Unknown command: " << command << std::endl;
            }
        }
    }

private:
    void send_get(const std::string& key) {
        std::string request = "GET " + key + "\n";
        send_request(request);
    }

    void send_set(const std::string& key, const std::string& value, uint32_t ttl) {
        std::string request = "SET " + key + " " + value + " " + std::to_string(ttl) + "\n";
        send_request(request);
    }

    void send_remove(const std::string& key) {
        std::string request = "REMOVE " + key + "\n";
        send_request(request);
    }

    void send_request(const std::string& request) {
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
        } catch (const std::exception& e) {
            std::cerr << "Request failed: " << e.what() << std::endl;
        }
    }

    io_context io_context_;
    ip::tcp::socket socket_;
};

