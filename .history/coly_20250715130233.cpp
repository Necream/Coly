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
    string getvalue(){
        if(type == "var") {
            if(name == "InputValue"){
                string content;
                cin>> content;
                return content;
            }
            return content;
        } else if(type == "code") {
            return codeinfo;
        }
        return "ERROR: Undefined type";
    }
};
map<string, defineinfo> definedcode;
map<string, defineinfo> definedvar;
vector<string> operationlist = {
    "$","define", "use", "jump", "printwithoutanewline", "print"
};
string prefix(string str,int len){
    if(str.length() < len) return str;
    return str.substr(0, len);
}
defineinfo judgedefine(string content){
    //0: type 1: named 2: with 3: codeinfo/varinfo 4:vartype 5:| 6: codevar
    defineinfo info;
    int infotype = 0;
    string varname="";
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
            if(varname.empty() && c == '$'){
                varname = "";
            }else if(c == ' ' || c == '\n' || c == '\r'){
                if(!varname.empty()){
                    info.content += varname;
                    varname = "";
                    info.content += definedvar[varname].getvalue();
                }else
            }
        }
    }
    // cout<<info<<endl;
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
    fclose(stream);
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(fileinfo);
    } catch (const nlohmann::json::parse_error& e) {
        cout << "Error: JSON parse error: " << e.what() << endl;
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
    size_t pos = command.find('$');
    if (pos != string::npos) {
        command.replace(pos, 1, filename);
    }
    if(command.empty()){
        cout << "Error: No run command defined for language: " << info.language << endl;
        return;
    }
    // cout<< "Running command: " << command << endl;
    system(command.c_str());
}
void print(const string &content) {
    for(int i=0;i<content.size();i++){
        char c = content[i];
        if(c == '$'){
            string varname;
            for(i=i+1;i<content.size();i++){
                if(content[i] == ' ' || content[i] == '\n' || content[i] == '\r'){
                    break;
                }
                varname += content[i];
            }
            if(definedvar.find(varname) != definedvar.end()){
                cout << definedvar[varname].getvalue();
            } else {
                cout << "Error: Undefined variable: " << varname << endl;
            }
        }else if(c == '\n'){
            cout << endl;
        }else if(c == '\r'){
            // Ignore carriage return
        }else cout << c;
    }
}
void addressline(string line,int *lineid){
    static bool defined = false;
    static defineinfo info;
    for(int i=0;i<operationlist.size();i++){
        string content = line;
        if(defined && info.type == "code" && content[0] == '|'){
            content = content.substr(1);
            // cout<<content<<" | "<<info.codeinfo<<endl;
            info.codeinfo += content + "\n";
            return;
        }else if(defined && info.type == "code" && content[0] != '|'){
            defined = false;
            definedcode[info.name] = info;
        }
        if(prefix(line, operationlist[i].length()) == operationlist[i]){
            string content = line.substr(operationlist[i].length());
            while(content[0] == ' ') content = content.substr(1);
            if(operationlist[i] == "define"){
                // cout << "Defining: " << content << endl;
                info=judgedefine(content);
                // cout<<info<<endl;
                if(info.type == "code") defined = true;
                else definedvar[info.name] = info;
                defineline[info.name] = *lineid;
            } else if(operationlist[i] == "use"){
                // cout << "Using: " << content << endl;
                // cout<< info <<endl;
                usedefine(content);
            } else if(operationlist[i] == "jump"){
                // cout << "Jumping to: " << content << endl;
                // cout<< info <<endl;
                *lineid = defineline[content];
                usedefine(content);
            } else if(operationlist[i] == "printwithoutanewline"){
                print(content);
            } else if(operationlist[i] == "print"){
                print(content + " \n");
            }
            return;
        }
    }
}
vector<string> readCly(string path){  //TODO:
    ifstream stream(path);
    if(!stream.is_open()){
        return {"Error: Cannot open file " + path};
    }
    vector<string> lines;
    string line;
    while(getline(stream, line)){
        if(prefix(line, sizeof("import lib")) == "import lib"){
            string importpath = line.substr(sizeof("import lib "));
            vector<string> importinfo=readCly(importpath);
            for(string importline : importinfo){
                lines.push_back(importline);
            }
            continue;
        }
        lines.push_back(line);
    }
    stream.close();
    return lines;
}
void useCly(vector<string> lines){
    for(int lineid=0;lineid<lines.size();lineid++){
        string line = lines[lineid];
        if(line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        int beforelineid = lineid;
        addressline(line, &lineid);
        if(beforelineid != lineid) {
            lineid--; // Adjust lineid if it was changed by a jump
        }
    }
}
int main(int argc, char *argv[]){
    if(argc==1){
        useCly(readCly("InteractiveColy.cly"));
    }
    if(argc==2){
        useCly(readCly(argv[1]));
    }
    return 0;
}