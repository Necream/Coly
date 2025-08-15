#include <iostream>
#include <vector>
#include <clang-c/Index.h>

// 回调函数：遍历 AST
CXChildVisitResult visitor(CXCursor cursor, CXCursor parent, CXClientData clientData) {
    std::vector<std::string>* globals = static_cast<std::vector<std::string>*>(clientData);

    if (clang_getCursorKind(cursor) == CXCursor_VarDecl) {
        if (clang_Cursor_isFileVarDecl(cursor) && !clang_Cursor_isStaticLocal(cursor)) {
            CXString name = clang_getCursorSpelling(cursor);
            globals->push_back(clang_getCString(name));
            clang_disposeString(name);
        }
    }

    clang_visitChildren(cursor, visitor, clientData);
    return CXChildVisit_Continue;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <cpp_source_file>\n";
        return 1;
    }

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        argv[1],
        nullptr, 0,
        nullptr, 0,
        CXTranslationUnit_None
    );

    if (!unit) {
        std::cerr << "无法解析文件: " << argv[1] << "\n";
        clang_disposeIndex(index);
        return 1;
    }

    CXCursor rootCursor = clang_getTranslationUnitCursor(unit);
    std::vector<std::string> globals;
    clang_visitChildren(rootCursor, visitor, &globals);

    if (globals.empty()) {
        std::cout << "未检测到全局变量。\n";
    } else {
        std::cout << "检测到的全局变量：\n";
        for (auto& var : globals) {
            std::cout << var << "\n";
        }
    }

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
    return 0;
}
