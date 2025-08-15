#include "asio.hpp"
#include <iostream>
#include <memory>
#include <set>
#include <functional>

using asio::ip::tcp;

// 会话类
struct ServerSession : std::enable_shared_from_this<ServerSession> {
    tcp::socket socket;
    char read_buf[512];
    std::set<std::shared_ptr<ServerSession>>& clients;

    ServerSession(tcp::socket sock, std::set<std::shared_ptr<ServerSession>>& all_clients)
        : socket(std::move(sock)), clients(all_clients) {}

    // 启动会话
    void start() {
        read_message();
    }

    // 读取数据
    void read_message() {
        auto self = shared_from_this();
        socket.async_read_some(asio::buffer(read_buf),
            [this, self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string msg(read_buf, length);
                    std::cout << "Received: " << msg << "\n";

                    // 回显固定消息
                    send_message("111");

                    // 继续读取
                    read_message();
                } else {
                    std::cout << "Client disconnected\n";
                    close();
                }
            });
    }

    // 发送数据
    void send_message(const std::string& msg) {
        auto self = shared_from_this();
        asio::async_write(socket, asio::buffer(msg),
            [this, self](std::error_code ec, std::size_t /*length*/) {
                if (ec) {
                    close();
                }
            });
    }

    // 关闭连接
    void close() {
        clients.erase(shared_from_this());
        socket.close();
    }
};

// 接受新连接
void start_accepting(asio::io_context& io, tcp::acceptor& acceptor, std::set<std::shared_ptr<ServerSession>>& clients) {
    acceptor.async_accept(
        [&](std::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::cout << "New client connected\n";
                auto session = std::make_shared<ServerSession>(std::move(socket), clients);
                clients.insert(session);
                session->start();
            }
            start_accepting(io, acceptor, clients); // 继续等待
        });
}

int main() {
    try {
        asio::io_context io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 12345));
        std::set<std::shared_ptr<ServerSession>> clients;

        std::cout << "Server running on port 12345...\n";
        start_accepting(io, acceptor, clients);

        io.run();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
