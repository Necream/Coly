#ifndef __CPLUSPLUSSYNC_HPP__
#define __CPLUSPLUSSYNC_HPP__

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;
vector<string> getglobalvariables(vector<string> &lines){
    vector<string> globalVariables;
    for (const string &line : lines) {
        if (line.find("int ") != string::npos || line.find("float ") != string::npos ||
            line.find("double ") != string::npos || line.find("char ") != string::npos ||
            line.find("std::string ") != string::npos) {
            size_t pos = line.find_first_of(" \t");
            if (pos != string::npos) {
                globalVariables.push_back(line.substr(0, pos));
            }
        }
    }
    return globalVariables;
    
}


#endif // __CPLUSPLUSSYNC_HPP__