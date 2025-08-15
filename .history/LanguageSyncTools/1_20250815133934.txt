#include <iostream>
#include <string>

// 普通全局变量
int globalInt = 42;
const double PI = 3.14159;
std::string globalString = "Hello, World!";

// 静态全局变量
static int staticGlobal = 100;

// 外部声明
extern float externalVar;

// 带指针和引用的全局变量
int* globalPtr = &globalInt;
int& globalRef = globalInt;

// 常量表达式
constexpr int ARRAY_SIZE = 10;

// 全局数组
int globalArray[ARRAY_SIZE];

// 全局结构体
struct GlobalStruct {
    int x;
    double y;
} globalStructInstance;

// 命名空间中的全局变量
namespace MyNamespace {
    int namespaceVar = 99;
}

// 函数声明
void someFunction();

int main() {
    // 局部变量
    int localInt = 10;
    double localDouble = 2.71828;
    char localChar = 'A';
    
    // 局部静态变量
    static int staticLocal = 50;
    
    // 局部数组
    int localArray[5] = {1, 2, 3, 4, 5};
    
    // 局部结构体
    struct {
        int a;
        float b;
    } localStruct;
    
    // 局部指针
    int* localPtr = &localInt;
    
    // 局部引用
    int& localRef = localInt;
    
    // 块作用域变量
    {
        int blockScopedVar = 1000;
        std::cout << blockScopedVar << std::endl;
    }
    
    someFunction();
    return 0;
}

void someFunction() {
    // 函数局部变量
    int functionLocal = 999;
    static int functionStatic = 888;
    
    // 函数参数也是局部变量
    auto lambda = [](int param) {
        // lambda中的局部变量
        int lambdaLocal = param * 2;
        return lambdaLocal;
    };
    
    lambda(5);
}