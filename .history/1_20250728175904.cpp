#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U16TEXT);

    std::wcout << L"你好，Necream！中文输出正常啦~" << std::endl;

    return 0;
}
