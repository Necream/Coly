#ifndef __CPLUSPLUSSYNC_HPP__
#define __CPLUSPLUSSYNC_HPP__

#include <string>
#include <vector>
#include <fstream>
#include <cctype>
#include <stdexcept>

struct VarDecl {
    std::string type;
    std::string name;
};

namespace detail {

// 简单去掉注释与字符串/字符字面量，避免干扰
inline std::string strip_comments_and_literals(const std::string& src) {
    std::string out;
    out.reserve(src.size());
    enum State { CODE, SLASH, LINE_COMMENT, BLOCK_COMMENT, STR, CHR, ESC_IN_STR, ESC_IN_CHR } st = CODE;

    for (size_t i = 0; i < src.size(); ++i) {
        char c = src[i];
        switch (st) {
            case CODE:
                if (c == '/') st = SLASH, out += ' ';
                else if (c == '"') st = STR, out += '"';
                else if (c == '\'') st = CHR, out += '\'';
                else out += c;
                break;
            case SLASH:
                if (c == '/') { st = LINE_COMMENT; out.back() = ' '; }
                else if (c == '*') { st = BLOCK_COMMENT; out.back() = ' '; }
                else { st = CODE; out.back() = '/'; out += c; }
                break;
            case LINE_COMMENT:
                if (c == '\n') { st = CODE; out += '\n'; }
                break;
            case BLOCK_COMMENT:
                if (c == '*' && i + 1 < src.size() && src[i+1] == '/') { ++i; st = CODE; out += ' '; }
                break;
            case STR:
                if (c == '\\') { st = ESC_IN_STR; out += ' '; }
                else if (c == '"') { st = CODE; out += '"'; }
                else out += ' ';
                break;
            case CHR:
                if (c == '\\') { st = ESC_IN_CHR; out += ' '; }
                else if (c == '\'') { st = CODE; out += '\''; }
                else out += ' ';
                break;
            case ESC_IN_STR: st = STR; out += ' '; break;
            case ESC_IN_CHR: st = CHR; out += ' '; break;
        }
    }
    return out;
}

// 简单词法：标识符、数字、符号分离
struct Tok { std::string s; };
inline bool is_ident_start(char c){ return std::isalpha((unsigned char)c) || c=='_'; }
inline bool is_ident(char c){ return std::isalnum((unsigned char)c) || c=='_'; }

inline std::vector<Tok> tokenize(const std::string& src) {
    std::vector<Tok> t;
    for (size_t i = 0; i < src.size();) {
        char c = src[i];
        if (std::isspace((unsigned char)c)) { ++i; continue; }
        if (is_ident_start(c)) {
            size_t j = i+1;
            while (j < src.size() && is_ident(src[j])) ++j;
            t.push_back({src.substr(i, j-i)});
            i = j;
        } else if (std::isdigit((unsigned char)c)) {
            size_t j = i+1;
            while (j < src.size() && (std::isalnum((unsigned char)src[j]) || src[j]=='.' || src[j]=='\'')) ++j;
            t.push_back({src.substr(i, j-i)});
            i = j;
        } else {
            // 处理双字符操作符
            if (i+1 < src.size()) {
                std::string two = src.substr(i,2);
                if (two=="::" || two=="->" || two=="++" || two=="--" || two=="&&" || two=="||" ||
                    two=="<=" || two==">=" || two=="==" || two=="!=" || two=="<<" || two==">>" ) {
                    t.push_back({two}); i+=2; continue;
                }
            }
            t.push_back({std::string(1,c)});
            ++i;
        }
    }
    return t;
}

enum ScopeKind { S_GLOBAL, S_NAMESPACE, S_TYPE, S_FUNCTION, S_OTHER };

inline bool is_storage_or_cv(const std::string& s) {
    return s=="extern" || s=="static" || s=="const" || s=="constexpr" || s=="volatile" || s=="register" || s=="thread_local";
}

inline bool is_type_word(const std::string& s) {
    // 常见内建 + 关键词（不严格）
    return s=="void"||s=="bool"||s=="char"||s=="wchar_t"||s=="char8_t"||s=="char16_t"||s=="char32_t"||
           s=="short"||s=="int"||s=="long"||s=="signed"||s=="unsigned"||s=="float"||s=="double"||
           s=="auto";
}

// 仅由 namespace 打开的作用域被视为“全局层”（可计入全局变量）
inline bool scope_is_globalish(const std::vector<ScopeKind>& st) {
    for (auto k : st) {
        if (k==S_TYPE || k==S_FUNCTION || k==S_OTHER) return false;
    }
    return true; // 只含 GLOBAL/NAMESPACE
}

// 简单判断：当前位置是否开始了函数定义（… ) { ）
inline bool looks_like_function(const std::vector<Tok>& T, size_t l, size_t r) {
    // 寻找形如 <maybe type> <ident> '(' ... ')' '{'
    // 从右到左找 “){”
    size_t i = l;
    while (i < r) {
        if (T[i].s == "(") {
            // 找到匹配 ')'
            int depth = 1; size_t j = i+1;
            for (; j<r; ++j) {
                if (T[j].s=="(") ++depth;
                else if (T[j].s==")") { --depth; if (!depth) break; }
            }
            if (j<r) {
                // 括号后看看是否紧跟 '{'
                size_t k = j+1;
                while (k<r && (T[k].s==";" || T[k].s==":" || T[k].s== "noexcept")) ++k; // 超简单
                // 最关键：出现 '{' 则判断为函数/构造等
                for (; k<r; ++k) {
                    if (T[k].s=="{") return true;
                    if (T[k].s==";") break;
                }
            }
        }
        ++i;
    }
    return false;
}

} // namespace detail

// 读取文件全部内容
inline std::string read_file_all(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("无法打开文件: " + path);
    std::string s((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return s;
}

/**
 * 核心函数：获取“正常的全局变量”
 * 规则：
 *  - 仅在“全局/命名空间”层（不在函数/类/结构体/枚举内部）
 *  - 排除 extern/static/const/constexpr
 *  - 支持多声明、指针/引用/数组/初始化
 */
inline std::vector<VarDecl> get_global_normal_variables_from_tokens(const std::vector<detail::Tok>& T) {
    using namespace detail;
    std::vector<VarDecl> out;
    std::vector<ScopeKind> scope; // 空 == global
    scope.push_back(S_GLOBAL);

    auto push = [&](ScopeKind k){ scope.push_back(k); };
    auto pop  = [&]{ if (scope.size()>1) scope.pop_back(); };

    // 简易状态：在“候选声明行”上收集类型前缀与声明符
    for (size_t i=0; i<T.size(); ++i) {
        const std::string& s = T[i].s;

        // 作用域推进/回退（依据前文关键字粗略分类）
        if (s == "namespace") {
            // 跳过名字与可选的 '::'
            size_t j = i+1;
            while (j<T.size() && (is_ident_start(T[j].s[0]) || T[j].s=="::")) ++j;
            // 期望 '{'
            while (j<T.size() && T[j].s!="{") ++j;
            if (j<T.size() && T[j].s=="{") { push(S_NAMESPACE); i = j; continue; }
        }
        if (s=="class"||s=="struct"||s=="union"||s=="enum") {
            // 跳到 '{' 或 ';'
            size_t j = i+1;
            while (j<T.size() && T[j].s!="{" && T[j].s!=";") ++j;
            if (j<T.size() && T[j].s=="{") { push(S_TYPE); i=j; continue; }
        }
        if (s=="{") { push(S_OTHER); continue; } // 兜底（如 try/catch、初始化器等）
        if (s=="}") { pop(); continue; }

        // 只在“全局/命名空间层”尝试识别变量声明
        if (!scope_is_globalish(scope)) continue;

        // 从此处尝试解析一条“声明/定义语句”，直到 ';' 或 '{'
        // 如果遇到 '{' 且看起来像函数定义，则忽略这一段
        size_t stmt_l = i, stmt_r = i;
        bool saw_semicolon = false, saw_brace = false;
        while (stmt_r < T.size()) {
            if (T[stmt_r].s == ";") { saw_semicolon = true; break; }
            if (T[stmt_r].s == "{") { saw_brace = true; break; }
            ++stmt_r;
        }
        if (stmt_l == stmt_r) continue;

        if (saw_brace && looks_like_function(T, stmt_l, stmt_r+1)) {
            // 这是函数/构造/聚合初始化起手式，跳过整个块（交给外层 '{' 处理）
            i = stmt_r; 
            continue;
        }

        if (!saw_semicolon) { // 没有分号，不是变量声明（可能是 using、template等）
            i = stmt_r;
            continue;
        }

        // 现在 [stmt_l, stmt_r] 是以 ';' 结束的一条语句
        // 试图按 “<前缀> <类型> <声明符[,声明符...]> ;” 解析
        // 先粗略收集前缀词
        bool has_forbidden = false; // extern/static/const/constexpr 等
        size_t p = stmt_l;
        while (p < stmt_r && (is_storage_or_cv(T[p].s) || is_type_word(T[p].s) || T[p].s=="::" || T[p].s=="*" || T[p].s=="&" || is_ident_start(T[p].s[0]) || T[p].s=="<" || T[p].s==">" || T[p].s==",")) {
            if (is_storage_or_cv(T[p].s)) has_forbidden = true;
            // 粗略越过模板尖括号内容  Foo<...>
            if (T[p].s=="<") {
                int depth=1; ++p;
                for (; p<stmt_r && depth; ++p) {
                    if (T[p].s=="<") ++depth;
                    else if (T[p].s==">") --depth;
                }
                continue; // p 已经在 '>' 后
            }
            // 遇到第一个标识符后，可能进入声明符区（遇到逗号或 '=' 才能确认）
            ++p;
            // 一旦遇到标识符 + 下一个是标识符/括号，有可能是函数声明，后面会被过滤
        }

        if (has_forbidden) { i = stmt_r; continue; } // 过滤 extern/static/const/constexpr

        // 从头扫描，找到第一个“声明符”的起点：它应当包含一个标识符（变量名）
        // 我们抽取：逗号分隔的若干声明符
        // 声明符 grammar（简化）：  (*|&)* <IDENT> ( [ ... ] )* ( = ... )?
        // 允许形如：Type a, *b, &c[10] = {...};
        size_t k = stmt_l;
        // 跳过前缀/类型碎片直到出现一个“看起来像名字的标识符”
        // 这里的策略：遇到第一个标识符，把它作为潜在名字，然后回看前是否存在 '*' 或 '&'，继续解析其后的数组/初始化
        auto consume_initializer = [&](size_t& u){
            // 跳过 = 表达式 或 { ... } 初始化器，直到逗号或分号
            if (u<stmt_r && T[u].s == "=") {
                ++u;
                int par=0, br=0, brc=0;
                while (u<stmt_r) {
                    if      (T[u].s=="(") ++par;
                    else if (T[u].s==")") --par;
                    else if (T[u].s=="[") ++br;
                    else if (T[u].s=="]") --br;
                    else if (T[u].s=="{") ++brc;
                    else if (T[u].s=="}") --brc;
                    else if (par==0 && br==0 && brc==0 && (T[u].s=="," || T[u].s==";")) break;
                    ++u;
                }
            } else if (u<stmt_r && T[u].s=="{") {
                int brc=1; ++u;
                while (u<stmt_r && brc) {
                    if (T[u].s=="{") ++brc;
                    else if (T[u].s=="}") --brc;
                    ++u;
                }
            }
        };

        // 为了输出类型字符串，粗略截到第一个名字前的片段
        auto collect_type_until = [&](size_t name_pos)->std::string{
            std::string ty;
            for (size_t x = stmt_l; x < name_pos; ++x) {
                const std::string& w = T[x].s;
                if (w=="," || w==";") break;
                // 排除存储/修饰关键字；保留命名空间限定、模板、内建类型
                if (is_storage_or_cv(w)) continue;
                if (w=="*" || w=="&") { ty += w; continue; }
                if (w=="::" || is_ident_start(w[0]) || is_type_word(w) || w=="<" || w==">" || w=="(" || w==")") {
                    ty += (ty.empty() ? "" : " ");
                    ty += w;
                }
            }
            return ty;
        };

        // 按逗号分声明符提取
        size_t u = stmt_l;
        // 找第一个名字
        while (u < stmt_r) {
            // 跳过修饰符与符号
            while (u<stmt_r && (is_storage_or_cv(T[u].s) || T[u].s=="*" || T[u].s=="&" || T[u].s=="::" || T[u].s=="<" || T[u].s==">"))
                ++u;

            // 找到名字（标识符且下一个不是 '(' —— 排除函数声明如  int f(...); ）
            if (u<stmt_r && is_ident_start(T[u].s[0])) {
                size_t name_pos = u;
                // 如果后面紧跟 '('，大概率是函数声明，整条语句跳过
                if (name_pos+1 < stmt_r && T[name_pos+1].s == "(") { u = stmt_r; break; }

                // 记录类型
                std::string ty = collect_type_until(name_pos);

                // 变量名
                std::string name = T[name_pos].s;

                // 消耗名字后的数组/初始化器
                u = name_pos + 1;
                // 数组维度
                while (u<stmt_r && T[u].s=="[") {
                    int br=1; ++u;
                    while (u<stmt_r && br) {
                        if (T[u].s=="[") ++br;
                        else if (T[u].s=="]") --br;
                        ++u;
                    }
                }
                // 初始化
                consume_initializer(u);

                // 允许多个声明符：逗号继续，下一个声明符
                out.push_back({ty, name});
                if (u<stmt_r && T[u].s==",") { ++u; continue; }
                break;
            } else {
                ++u;
            }
        }

        i = stmt_r; // 跳到分号位置，继续下一条语句
    }

    return out;
}

inline std::vector<VarDecl> getglobalvariables(const std::vector<std::string>& lines) {
    // 1) 读入并预处理
    std::string src;
    for (auto& ln : lines) { src += ln; src += '\n'; }
    auto cleaned = detail::strip_comments_and_literals(src);
    auto toks = detail::tokenize(cleaned);
    // 2) 提取“正常全局变量”
    return get_global_normal_variables_from_tokens(toks);
}

#endif // __CPLUSPLUSSYNC_HPP__
