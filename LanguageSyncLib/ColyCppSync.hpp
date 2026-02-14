// TODO:Unfinished
#ifndef COLY_CPP_SYNC_HPP
#define COLY_CPP_SYNC_HPP

#include "VariableSyncService.hpp"

NetworkSession *session = new NetworkSession();
// 连接服务器
bool is_connected = connect_to_server(*session, "localhost", "12345");
struct Syncint {
    int value;

    Syncint(int initial_value = 0) : value(initial_value) {}

    void set(int new_value) {
        value = new_value;
        std::string msg = "SET " + std::to_string(new_value);
        send_message(*session, msg);
    }

    int get() {
        std::string response = send_message(*session, "GET");
        return std::stoi(response);
    }
};

#endif // COLY_CPP_SYNC_HPP