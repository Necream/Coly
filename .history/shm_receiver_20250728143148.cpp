#include "ipc_helper.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // 创建两个共享内存，与发送端相反
    ipc::SharedMemory shmReceive("MySharedMemorySend", 1024);
    ipc::SharedMemory shmSend("MySharedMemoryReceive", 1024);
    
    if (!ipc::ErrorManager::isSuccess()) {
        std::cerr << "错误: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
        return 1;
    }

    std::cout << "接收端启动，等待消息..." << std::endl;
    
    while (true) {
        // 接收数据
        char buffer[1024] = {0};
        if (!shmReceive.read(buffer, sizeof(buffer))) {
            std::cerr << "读取错误: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
            continue;
        }

        std::string received(buffer);
        if (received.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::cout << "收到消息: " << received << std::endl;
        
        // 发送回复
        std::string reply = "已收到消息: " + received;
        if (!shmSend.write(reply.c_str(), reply.size() + 1)) {
            std::cerr << "写入错误: " << ipc::ErrorManager::getLastErrorMessage() << std::endl;
            continue;
        }
    }

    return 0;
}