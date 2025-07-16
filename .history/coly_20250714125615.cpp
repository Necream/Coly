#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
using namespace std;
struct defineinfo {
    string type; // code or var
    string name; // name of the variable/code
    string language; // language type, e.g., C++, Python, etc.
    string content; // content of the variable
    string codeinfo; // additional code information
    friend ostream& operator<<(ostream &os, const defineinfo &info) {
        os  << "Type: " << info.type << ", "
            << "Name: "     << info.name << ", "
            << "Language: " << info.language << ", "
            << "Content: " << info.content << ", "
            << "CodeInfo: " << info.codeinfo;
    }
};
string operationlist[] = {
    "define", "use", "jump", "import", "print", "printwithoutanewline"
};
string prefix(string str,int len){
    if(str.length() < len) return str;
    return str.substr(0, len);
}
defineinfo judgedefine(string content){
    //0: type 1: named 2: with 3: codeinfo/varinfo 4:vartype 5:| 6: codevar
    defineinfo info;
    int infotype = 0;
    for(char c : content){
        if(c == ' '){
            infotype++;
            continue;
        }
        if(infotype == 0){
            info.type += c;
        } else if(infotype == 1){
        } else if(infotype == 2){
            info.name += c;
        } else if(infotype == 3){
        } else if(infotype == 4){
            info.language += c;
        }
        if(infotype >= 4&&info.type == "var"){
            info.codeinfo += c;
        }
    }
    return info;
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