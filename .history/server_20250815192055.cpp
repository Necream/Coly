#include "asio.hpp"
#include <iostream>
#include <memory>
#include <set>

using asio::ip::tcp;

struct ClientSession : std::enable_shared_from_this<ClientSession> {
    tcp::socket socket;
    char read_buf[512];
    std::set<std::shared_ptr<ClientSession>>& clients;

    ClientSession(tcp::socket sock, std::set<std::shared_ptr<ClientSession>>& all_clients)
        : socket(std::move(sock)), clients(all_clients) {}

    void start() {
        do_read();
    }

    void do_read() {
        auto self = shared_from_this();
        socket.async_read_some(asio::buffer(read_buf),
            [this, self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string msg(read_buf, length);
                    std::cout << "Received: " << msg << "\n";
                    do_write("111");
                    do_read(); // 继续读
                } else {
                    std::cout << "Client disconnected\n";
                    clients.erase(self);
                }
            });
    }

    void do_write(const std::string& msg) {
        auto self = shared_from_this();
        asio::async_write(socket, asio::buffer(msg),
            [this, self](std::error_code ec, std::size_t /*length*/) {
                if (ec) {
                    clients.erase(self);
                }
            });
    }
};

int main() {
    try {
        asio::io_context io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 12345));
        std::set<std::shared_ptr<ClientSession>> clients;

        std::function<void()> do_accept;
        do_accept = [&]() {
            acceptor.async_accept(
                [&](std::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        std::cout << "New client connected\n";
                        auto session = std::make_shared<ClientSession>(std::move(socket), clients);
                        clients.insert(session);
                        session->start();
                    }
                    do_accept(); // 继续等下一个客户端
                });
        };

        do_accept();

        std::cout << "Server running on port 12345...\n";
        io.run();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
