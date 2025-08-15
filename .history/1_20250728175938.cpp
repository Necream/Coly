#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

int main() {
    // 设置控制台输出为UTF-8编码
    SetConsoleOutputCP(CP_UTF8); 
    // 将标准输出流设置为以UTF-16方式写入，配合宽字符使用
    _setmode(_fileno(stdout), _O_U16TEXT); 

    // 使用宽字符流输出中文
    std::wcout << L"你好，世界！" << std::endl; 

    return 0;
}