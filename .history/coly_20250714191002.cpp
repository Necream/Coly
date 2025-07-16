#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
#include <map>
#include <cstdlib>
#include <vector>
#include "json.hpp"
#include "GXPass.hpp"
using namespace std;
map<string, int> defineline;
struct defineinfo {
    string type; // code or var
    string name; // name of the variable/code
    string language; // language type, e.g., C++, Python, etc.
    string content; // content of the variable
    string codeinfo; // additional code information
    friend ostream& operator<<(ostream &os, const defineinfo &info) {
        os  << "Type: "     << info.type     << endl
        << "Name: "     << info.name     << endl
        << "Language: " << info.language << endl
        << "Content: "  << info.content  << endl
        << "CodeInfo: " << info.codeinfo << endl;
        return os;
    }
};
map<string, defineinfo> definedcode;
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
        } else if(infotype == 4&&info.type == "code"){
            info.language += c;
        }
        if(infotype >= 4&&info.type == "var"){
            info.content += c;
        }
    }
    return info;
}
string getextension(const string language) {
    FILE *stream=fopen("./Settings/LanguageMap.json", "r");
    if (!stream) {
        cout << "Error: Cannot open LanguageMap.json" << endl;
        return ".txt"; // Default extension if file cannot be opened
    }
    int c=0;
    string fileinfo;
    while((c=fgetc(stream))!=-1){
        fileinfo += (char)c;
    }
    fclose(stream);
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(fileinfo);
    } catch (const nlohmann::json::parse_error& e) {
        cout << "Error: JSON parse error: " << e.what() << endl;
        fclose(stream);
        return ""; // Default extension if parsing fails
    }
    return json[language]["extension"].get<string>();
}
string getrun(const string language) {
    FILE *stream=fopen("./Settings/LanguageMap.json", "r");
    if (!stream) {
        cout << "Error: Cannot open LanguageMap.json" << endl;
        return ".txt"; // Default run if file cannot be opened
    }
    int c=0;
    string fileinfo;
    while((c=fgetc(stream))!=-1){
        fileinfo += (char)c;
    }
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(fileinfo);
    } catch (const nlohmann::json::parse_error& e) {
        cout << "Error: JSON parse error: " << e.what() << endl;
        fclose(stream);
        return ""; // Default run if parsing fails
    }
    return json[language]["run"].get<string>();
}
void usedefine(string content){
    if(defineline.find(content) == defineline.end()){
        cout << "Error: Undefined variable or code: " << content << endl;
        return;
    }
    defineinfo info = definedcode[content];
    string extension = getextension(info.language);
    string filename = GXPass::number2ABC(GXPass::compile(info.name))+"."+extension;
    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        cout << "Error: Cannot open file " << filename << endl;
        return;
    }
    for(char c:info.codeinfo) fputc(c, fp);
    fclose(fp);
    string command=getrun(info.language);
    cout<<
    if(command.empty()){
        cout << "Error: No run command defined for language: " << info.language << endl;
        return;
    }
    system(command.c_str());
}
void addressline(string line,int &lineid){
    static bool defined = false;
    static defineinfo info;
    for(int i=0;i<operationlist->size();i++){
        string content = line;
        if(defined && info.type == "code" && content[0] == '|'){
            content = content.substr(1);
            // cout<<content<<" | "<<info.codeinfo<<endl;
            info.codeinfo += content + "\n";
            return;
        }else if(defined && info.type == "code" && content[0] != '|'){
            defined = false;
            defineline[info.name] = lineid;
            definedcode[info.name] = info;
        }
        if(prefix(line, operationlist[i].length()) == operationlist[i]){
            string content = line.substr(operationlist[i].length());
            if(content[0] == ' ') content = content.substr(1);
            if(operationlist[i] == "define"){
                cout << "Defining: " << content << endl;
                info=judgedefine(content);
                // cout<<info<<endl;
                defined = true;
            } else if(operationlist[i] == "use"){
                cout << "Using: " << content << endl;
                cout<< info <<endl;
                usedefine(content);
            } else if(operationlist[i] == "jump"){
                cout << "Jumping to: " << content << endl;
                cout<< info <<endl;
                lineid= defineline[content];
                usedefine(content);
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
        return;
    }
    vector<string> lines;
    string line;
    while(getline(stream, line)){
        lines.push_back(line);
    }
    for(int lineid=0;lineid<lines.size();lineid++){
        line = lines[lineid];
        if(line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        int beforelineid = lineid;
        addressline(line, lineid);
        if(beforelineid != lineid) {
            lineid--; // Adjust lineid if it was changed by a jump
        }
    }
}
int main(int argc, char *argv[]){
    if(argc==1){
        useCly("InteractiveColy.cly");
    }
    return 0;
}