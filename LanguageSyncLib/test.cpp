#include <iostream>
#include "ColyCppSync.hpp"
int main() {
    RegColyVar(testVar);
    testVar = "Hello, Coly!";
    std::cout << "testVar: " << testVar.data << std::endl;
    return 0;
}