#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
using namespace std;
string operationlist[] = {
    "define", "use", "jump", "import", "print", "printwithoutanewline"
};
void addressline(string line){
    
}
void useCly(string path){
    ifstream stream(path);
    if(!stream.is_open()){
        cout<<"Error: Cannot open file "<<path<<endl;
        return;
    }
    string line;
    while(getline(stream, line)){
        if(line.empty() || line[0] == '#') continue;
        addressline(line);
    }
}
int main(int argc, char *argv[]){
    if(argc==1){
        useCly("InteractiveColy.cly");
    }
    return 0;
}