#include <iostream>
#include <fstream>
#include "C++.hpp"
using namespace std;
int main(int argc, char* argv[]){
    if(argc<2){
        cout << "Usage: " << argv[0] << " <file_path>" << endl;
        return 1;
    }else if(argc==2){
        string fiepath=argv[1];
        ifstream file(fiepath);
        if (!file) {
            cerr << "Error opening file: " << fiepath << endl;
            return 1;
        }
        string line;
        vector<string> lines;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        vector<var> globalvariables=getglobalvariables(lines);
        cout << "Global variables found:" << endl;
        for (const auto& v : globalvariables) {
            cout << "Type: " << v.type << ", Name: " << v.name << endl;
        }
    }
    return 0;
}