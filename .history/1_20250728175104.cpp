#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>

int main() {
    // 切换控制台到 UTF-8
    SetConsoleOutputCP(CP_UTF8);
    // 把 stdout 设置为宽字符模式
    _setmode(_fileno(stdout), _O_U16TEXT);

    // 用宽字符串输出
    std::wcout << L"你好，Necream！中文输出正常啦～" << std::endl;

    return 0;
}
