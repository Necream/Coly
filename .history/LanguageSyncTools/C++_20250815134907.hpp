#ifndef __CPLUSPLUSSYNC_HPP__
#define __CPLUSPLUSSYNC_HPP__

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;
vector<string> getglobalvariables() {
    // This function returns a vector of global variables in C++
    // For simplicity, we return a hardcoded list of common global variables
    return {
        "int globalInt = 42;",
        "const double PI = 3.14159;",
        "string globalString = \"Hello, World!\";",
        "static int staticGlobal = 100;",
        "extern float externalVar;",
        "int* globalPtr = &globalInt;",
        "int& globalRef = globalInt;",
        "constexpr int ARRAY_SIZE = 10;",
        "int globalArray[ARRAY_SIZE];",
        "struct GlobalStruct { int x; double y; } globalStructInstance;"
    };
}


#endif // __CPLUSPLUSSYNC_HPP__