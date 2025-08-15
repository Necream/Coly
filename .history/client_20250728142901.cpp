#include "ipc_helper.hpp"
#include <iostream>

int main() {
    ipc::NamedPipe pipe;
    if (!pipe.init("MyPipe", false)) {
        std::cerr << "Error: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
        return 1;
    }

    std::string receivedMessage;
    if (!pipe.read(receivedMessage)) {
        std::cerr << "Read Error: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
        return 1;
    }

    std::cout << "收到消息: " << receivedMessage << std::endl;
    return 0;
}