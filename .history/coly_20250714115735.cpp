#include <iostream>
#include <string>
#include <cstdio>
using namespace std;
void useCly(string path){
    FILE *stream=fopen(path.c_str(), "r");
    if(stream==NULL){
        cout<<"Error: Cannot open file "<<path<<endl;
        return;
    }
    string line;
    while(getline(stream, line)){
        if(line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        cout << line << endl; // Simulate processing the line
    }
}
int main(int argc, char *argv[]){
    if(argc==1){
        useCly("InteractiveColy.cly");
    }
    return 0;
}