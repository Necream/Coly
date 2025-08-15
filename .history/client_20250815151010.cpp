// client.cpp
#include "asio.hpp"
#include <iostream>
#include <string>

using asio::ip::tcp;

int main() {
    try {
        asio::io_context io;
        tcp::socket socket(io);

        tcp::resolver resolver(io);
        auto endpoints = resolver.resolve("127.0.0.1", "12345");

        asio::error_code ec;
        asio::connect(socket, endpoints, ec);
        if (ec) {
            std::cerr << "Connect failed: " << ec.message() << "\n";
            return 1;
        }

        std::cout << "Connected to server\n";

        while (true) {
            std::string input;
            std::cout << "> ";
            std::getline(std::cin, input);
            if (input == "quit") break;

            asio::write(socket, asio::buffer(input), ec);

            char buf[512];
            size_t n = socket.read_some(asio::buffer(buf), ec);
            if (!ec) {
                std::string msg(buf, n);
                std::cout << "Server echo: " << msg << "\n";
            }
        }

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
