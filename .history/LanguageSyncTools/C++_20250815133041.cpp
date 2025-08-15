#include <iostream>
#include <fstream>
#include <string>
#include <regex>

using namespace std;

void findGlobalVariables(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return;
    }

    string line;
    regex globalVarRegex(R"((?:^|\s)(?:static\s+)?(?:const\s+)?(?:volatile\s+)?(?:inline\s+)?(?:[a-zA-Z_][a-zA-Z0-9_]*\s+)+([a-zA-Z_][a-zA-Z0-9_]*)\s*(?:\[[^\]]*\])?\s*(?:=[^;]*)?\s*;)");
    smatch match;

    while (getline(file, line)) {
        if (regex_search(line, match, globalVarRegex)) {
            cout << "找到全局变量: " << match[1].str() << endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <C++源文件>" << endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        findGlobalVariables(argv[i]);
    }

    return 0;
}