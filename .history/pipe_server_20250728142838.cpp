#include "ipc_helper.hpp"
#include <iostream>

int main() {
    // 共享内存示例
    ipc::SharedMemory shm("MySharedMemory", 1024);
    if (!ipc::ErrorManager::isSuccess()) {
        std::cerr << "Error: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
        return 1;
    }

    std::string message = "Hello from process 1!";
    if (!shm.write(message.c_str(), message.size())) {
        std::cerr << "Write Error: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
        return 1;
    }

    // 命名管道示例
    ipc::NamedPipe pipe;
    if (!pipe.init("MyPipe", true)) {
        std::cerr << "Error: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
        return 1;
    }

    std::cout << "等待客户端连接..." << std::endl;
    if (!pipe.waitForClient()) {
        std::cerr << "Error: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
        return 1;
    }
    std::cout << "客户端已连接" << std::endl;

    std::string pipeMessage = "Hello through pipe!";
    if (!pipe.write(pipeMessage)) {
        std::cerr << "Write Error: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
        return 1;
    }

    return 0;
}