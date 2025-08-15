#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>

std::vector<std::string> findGlobalVariables(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("无法打开文件: " + filename);
    }

    std::string code, line;
    while (std::getline(file, line)) {
        // 去掉单行注释
        size_t pos = line.find("//");
        if (pos != std::string::npos) {
            line = line.substr(0, pos);
        }
        code += line + "\n";
    }

    // 去掉多行注释
    std::regex block_comment(R"(/\*[\s\S]*?\*/)");
    code = std::regex_replace(code, block_comment, "");

    std::vector<std::string> globals;
    std::regex var_pattern(
        R"(^\s*(?:static\s+|extern\s+)?(?:const\s+)?[a-zA-Z_]\w*(?:\s*::\s*[a-zA-Z_]\w*)*\s+\**\s*([a-zA-Z_]\w*)\s*(?:=\s*[^;]+)?;)"
    );

    int brace_level = 0;
    std::string buf;
    for (size_t i = 0; i < code.size(); ++i) {
        char c = code[i];
        buf += c;
        if (c == '{') brace_level++;
        else if (c == '}') brace_level--;

        if (c == '\n') {
            if (brace_level == 0) {
                std::smatch match;
                if (std::regex_match(buf, match, var_pattern)) {
                    globals.push_back(match[1]);
                }
            }
            buf.clear();
        }
    }

    return globals;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <cpp_source_file>\n";
        return 1;
    }

    try {
        auto globals = findGlobalVariables(argv[1]);
        if (globals.empty()) {
            std::cout << "未检测到全局变量。\n";
        } else {
            std::cout << "检测到的全局变量：\n";
            for (auto& var : globals) {
                std::cout << var << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
