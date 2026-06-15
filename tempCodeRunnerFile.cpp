#include <iostream>
#include "C:\\Coly\\LanguageSyncLib\\ColyCppSync.hpp"
using namespace std;
int main(int argc, char* argv[]){
    InitColySyncService();
    ReadColyVar(varname);
    cout << varname.data << endl;
    return 0;
}