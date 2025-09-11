#pragma once

#include <boost/asio.hpp>

#include <iostream>
#include <sstream>
#include <string>



using namespace boost::asio;

class Client {
public:
    Client() : socket_(io_context_) {}

    bool connect(const std::string &host, uint16_t port);

    void run_cli() {
        std::string line;
        std::cout << "Enter commands:\n"
                     "connect <ip> <port>,\n"
                     "link <ip> <port>\n"
                     "get <key>,\n"
                     "set <key> <value> [ttl],\n"
                     "remove <key>,\n"
                     "quit\n"
                  << std::endl;

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

            } else if (command == "link") {
                std::string ip1;
                std::string port1;

                if ( iss >> ip1 >> port1) {
                    link(ip1, port1);
                }
                else {
                    std::cout << "Usage: link <ip1> <port1> <ip2> <port2>" << std::endl;
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
    void link(const std::string &ip, const std::string& port);
    void send_get(const std::string &key);

    void send_set(const std::string &key, const std::string &value, uint32_t ttl);

    void send_remove(const std::string &key);

    void send_request(const std::string &request);

    io_context io_context_;
    ip::tcp::socket socket_;
};
