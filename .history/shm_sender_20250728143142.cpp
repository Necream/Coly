#include "ipc_helper.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // 创建两个共享内存，分别用于发送和接收
    ipc::SharedMemory shmSend("MySharedMemorySend", 1024);
    ipc::SharedMemory shmReceive("MySharedMemoryReceive", 1024);
    
    if (!ipc::ErrorManager::isSuccess()) {
        std::cerr << "错误: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
        return 1;
    }

    std::string input;
    while (true) {
        // 发送数据
        std::cout << "请输入要发送的消息 (输入'exit'退出): ";
        std::getline(std::cin, input);
        
        if (input == "exit") break;
        
        if (!shmSend.write(input.c_str(), input.size() + 1)) {
            std::cerr << "写入错误: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
            continue;
        }

        // 接收数据
        char buffer[1024] = {0};
        std::cout << "等待接收消息..." << std::endl;
        
        if (!shmReceive.read(buffer, sizeof(buffer))) {
            std::cerr << "读取错误: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
            continue;
        }
        
        std::cout << "收到回复: " << buffer << std::endl;
    }

    return 0;
}