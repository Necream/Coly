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
        while (getline(file, line)) {
            cout << line << endl;
        }
    }
    return 0;
}