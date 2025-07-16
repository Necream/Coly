#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
using namespace std;
string operationlist[] = {
    "define", "use", "jump", "import", "print", "printwithoutanewline"
};
string prefix(string str,int len){
    if(str.length() < len) return str;
    return str.substr(0, len);
}
bool judgedefine(string content){
    for(char c : content){
        if(c == ' ' || c == '\t') continue;
        if(c == '|' || c == ',') return true; // 允许逗号和竖线
        if(c < '0' || c > '9') return false; // 只允许数字
    }
}
void addressline(string line){
    static bool defined = false;
    for(int i=0;i<operationlist->size();i++){
        if(prefix(line, operationlist[i].length()) == operationlist[i]){
            string content = line.substr(operationlist[i].length());
            if(content[0] == ' ') content = content.substr(1);
            if(operationlist[i] == "define"){
                cout << "Defining: " << content << endl;
                defined = true;
            } else if(operationlist[i] == "use"){
                cout << "Using: " << content << endl;
            } else if(operationlist[i] == "jump"){
                cout << "Jumping to: " << content << endl;
            } else if(operationlist[i] == "import"){
                cout << "Importing: " << content << endl;
            } else if(operationlist[i] == "print"){
                cout << content << endl;
            } else if(operationlist[i] == "printwithoutanewline"){
                cout << content;
            }
            return;
        }
    }
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