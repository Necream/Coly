// server.cpp
#include "asio.hpp"
#include <iostream>
#include <vector>
#include <memory>

using asio::ip::tcp;

int main() {
    try {
        asio::io_context io;

        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 12345));
        std::vector<std::shared_ptr<tcp::socket>> clients;

        std::cout << "Server running on port 12345...\n";

        while (true) {
            // 非阻塞 accept
            auto socket = std::make_shared<tcp::socket>(io);
            asio::error_code ec;
            acceptor.accept(*socket, ec);
            if (!ec) {
                std::cout << "New client connected\n";
                clients.push_back(socket);
            }

            // 轮询客户端
            for (auto it = clients.begin(); it != clients.end();) {
                auto& client = *it;
                char buf[512];
                size_t n = client->read_some(asio::buffer(buf), ec);
                if (!ec) {
                    std::string msg(buf, n);
                    std::cout << "Received: " << msg << "\n";
                    asio::write(*client, asio::buffer(msg), ec); // 回发
                    ++it;
                } else {
                    if (ec != asio::error::would_block) {
                        std::cout << "Client disconnected\n";
                        it = clients.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

            io.poll();  // 处理异步事件
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
