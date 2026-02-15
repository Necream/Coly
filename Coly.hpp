#ifndef __COLY_HPP__
#define __COLY_HPP__

#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
#include <map>
#include <cstdlib>
#include <vector>
#include "json.hpp"
#include "GXPass.hpp"
#include "VariableSyncService.hpp"
using JSON = nlohmann::json;
#ifdef _WIN32
#define COLYPATH "C:\\Coly\\"
#else
#define COLYPATH "/lib/Coly/"
#endif
// Map to store the line number of defined variables, codes and positions
// This is used to check if a variable or code is defined before use
std::map<std::string, int> definedline;
// Get the prefix of a std::string with a specified length
std::string prefix(std::string str,int len){
    if(str.length() < len) return str;
    return str.substr(0, len);
}
// The following operations are supported in Coly
// define: define a variable or code or position
// use: use a  code
// jump: jump to a code / position
// import lib: import a libary
// print: print a variable or code
// printwithoutanewline: print a variable or code without a newline
// do: execute a code
// exit: exit the program
// ifn: if not, execute a code
// if: if, execute a code
std::vector<std::string> operationlist = {
    "define",
    "use",
    "jump",
    "printwithoutanewline",
    "print",
    "do",
    "exit",
    "ifn",
    "if",
    "import lib",
    "commitvaroperation"
};
std::vector<std::string> skipsynclist = {
    "Input",
    "InputLine"
};
// Structure to hold information about defined variables, codes, and positions
// It includes type, name, language, content, code information, and variable type
struct defineinfo {
    std::string type; // code or var
    std::string name; // name of the variable/code/positon
    std::string language; // language type, e.g., C++, Python, etc.
    std::string content; // content of the variable
    std::string codeinfo; // additional code information
    std::string vartype;
    defineinfo(){
        type = "";
        name = "";
        language = "";
        content = "";
        codeinfo = "";
        vartype = "std::string";
    }
    friend std::ostream& operator<<(std::ostream &os, const defineinfo &info) {
        os  << "Type: "     << info.type     << std::endl
        << "Name: "     << info.name     << std::endl
        << "Language: " << info.language << std::endl
        << "Content: "  << info.content  << std::endl
        << "CodeInfo: " << info.codeinfo << std::endl;
        return os;
    }
    std::string getvalue(NetworkSession& session){
        std::string commit_command="get var ";
        commit_command+=this->name;
        std::string echo;
        bool skip=0;
        for(std::string str:skipsynclist){
            if(str==this->name) skip=1;
        }
        if(!skip) echo=send_message(session,commit_command);
        if(prefix(echo, 7) == "[ERROR]"){
            std::cout << echo << std::endl;
            return echo;
        }
        JSON j;
        if(!skip) j=JSON::parse(echo);
        if(type == "var") {
            if(!skip) this->content=j["Value"];
            std::string content = this->content;
            if(name == "InputLine"){
                getline(std::cin, content);
            }else if(name == "Input"){
                std::string temp;
                std::cin>> content;
                std::cin.ignore();
            }
            return content;
        } else if(type == "code") {
            if(!skip) codeinfo=j["Value"];
            return codeinfo;
        }
        return "ERROR: Undefined type";
    }
};
// Map to store defined codes
std::map<std::string, defineinfo> definedcode;
// Map to store defined positions
std::map<std::string, int> definedposition;
// Map to store defined variables
std::map<std::string, defineinfo> definedvar;
// Map to store compiled codes
std::map<std::string, bool> compiledcode;
// Judge the type of a define operation, register the variable or code and return the defineinfo
defineinfo judgedefine(std::string content, NetworkSession& session){
    //0: type 1: named 2: with 3: codeinfo/varinfo 4:vartype 5:| 6: codevar
    defineinfo info;
    int infotype = 0;
    std::string varname="";
    bool var=0;
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
            if(info.type == "var"){
                info.content = "";
            } else if(info.type == "code"){
                info.codeinfo = "";
            }
        } else if(infotype == 4&&info.type == "code"){
            info.language += c;
        } else if(infotype >= 4&&info.type == "var"){
            if(varname.empty() && c == '$' && !var){
                varname = "";
                var = true;
            }else if(c == ' ' && var){
                if(!varname.empty() && definedvar.find(varname) != definedvar.end()){
                    // std::co<<varname<<std::endl;
                    info.content = definedvar[varname].getvalue(session);
                    varname = "";
                }else std::cout << "Error: Undefined variable: " << varname << std::endl;
            }else varname += c;
            // std::co<<c;
        }
    }
    if(!varname.empty() && var){
        // std::co<<varname<<std::endl;
        if(definedvar.find(varname) == definedvar.end()){
            std::cout << "Error: Undefined variable: " << varname << std::endl;
        }
        info.content = definedvar[varname].getvalue(session);
        // std::co<<info.content<<std::endl;
        varname = "";
    }else if(!varname.empty() && !var){
        info.content = varname;
    }
    // std::co<<info<<std::endl;
    return info;
}
// Get the file extension for a given language from LanguageMap.json
std::string getextension(const std::string language) {
    std::string LanguageJSONPath = COLYPATH;
    LanguageJSONPath+="Settings/LanguageMap.json";
    FILE *stream=fopen(LanguageJSONPath.c_str(), "r");
    if (!stream) {
        std::cout << "Error: Cannot open LanguageMap.json" << std::endl;
        return ".txt"; // Default extension if file cannot be opened
    }
    int c=0;
    std::string fileinfo;
    while((c=fgetc(stream))!=-1){
        fileinfo += (char)c;
    }
    fclose(stream);
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(fileinfo);
    } catch (const nlohmann::json::parse_error& e) {
        std::cout << "Error: JSON parse error: " << e.what() << std::endl;
        return ""; // Default extension if parsing fails
    }
    return json[language]["extension"].get<std::string>();
}
// Get the need of compile for a given language from LanguageMap.json
bool getneedcompile(const std::string language) {
    std::string LanguageJSONPath = COLYPATH;
    LanguageJSONPath+="Settings/LanguageMap.json";
    FILE *stream=fopen(LanguageJSONPath.c_str(), "r");
    if (!stream) {
        std::cout << "Error: Cannot open LanguageMap.json" << std::endl;
        return ".txt"; // Default extension if file cannot be opened
    }
    int c=0;
    std::string fileinfo;
    while((c=fgetc(stream))!=-1){
        fileinfo += (char)c;
    }
    fclose(stream);
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(fileinfo);
    } catch (const nlohmann::json::parse_error& e) {
        std::cout << "Error: JSON parse error: " << e.what() << std::endl;
        return ""; // Default extension if parsing fails
    }
    return json[language]["needcompile"].get<bool>();
}
// Get the compiler run command for a given language from LanguageMap.json
std::string getcompilerun(const std::string language) {
    std::string LanguageJSONPath = COLYPATH;
    LanguageJSONPath+="Settings/LanguageMap.json";
    FILE *stream=fopen(LanguageJSONPath.c_str(), "r");
    if (!stream) {
        std::cout << "Error: Cannot open LanguageMap.json" << std::endl;
        return ".txt"; // Default run if file cannot be opened
    }
    int c=0;
    std::string fileinfo;
    while((c=fgetc(stream))!=-1) fileinfo += (char)c;
    fclose(stream);
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(fileinfo);
    } catch (const nlohmann::json::parse_error& e) {
        std::cout << "Error: JSON parse error: " << e.what() << std::endl;
        return ""; // Default run if parsing fails
    }
    return json[language]["compilerun"].get<std::string>();
}
// Get the run command for a given language from LanguageMap.json
std::string getrun(const std::string language) {
    // std::co << "Getting run command for language: " << language << std::endl;
    std::string LanguageJSONPath = COLYPATH;
    LanguageJSONPath+="Settings/LanguageMap.json";
    FILE *stream=fopen(LanguageJSONPath.c_str(), "r");
    if (!stream) {
        std::cout << "Error: Cannot open LanguageMap.json" << std::endl;
        return ".txt"; // Default run if file cannot be opened
    }
    int c=0;
    std::string fileinfo;
    while((c=fgetc(stream))!=-1){
        fileinfo += (char)c;
    }
    fclose(stream);
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(fileinfo);
    } catch (const nlohmann::json::parse_error& e) {
        std::cout << "Error: JSON parse error: " << e.what() << std::endl;
        return ""; // Default run if parsing fails
    }
    return json[language]["run"].get<std::string>();
}
// TODO: 3rd language variable sync service
// Use a defined code, compile it if necessary, and run it
void usedefine(std::string content, NetworkSession& session){
    if(definedline.find(content) == definedline.end()){
        std::cout << "Error: Undefined variable or code: " << content << std::endl;
        return;
    }
    defineinfo info = definedcode[content];
    std::string extension = getextension(info.language);
    std::string filename = COLYPATH;
    filename += "TempCode/";
    filename += GXPass::number2ABC(GXPass::compile(info.name))+"."+extension;
    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        std::cout << "Error: Cannot open file " << filename << std::endl;
        return;
    }
    for(char c:info.codeinfo) fputc(c, fp);
    fclose(fp);
    std::string command = "";
    std::string codename = GXPass::number2ABC(GXPass::compile(info.name));
    if((compiledcode.find(codename) != compiledcode.end() && compiledcode[codename]) || !getneedcompile(info.language)){
        command=getrun(info.language); // If the code has been compiled, just run it
    } else command=getcompilerun(info.language); // If the code has not been compiled, compile it first
    size_t pos = command.find('$');
    while(pos != std::string::npos)command.replace(pos, 1, filename),pos = command.find('$');
    pos = command.find('^');
    std::string outputfilepath = filename.substr(0, filename.find_last_of('.'));
    compiledcode[codename] = true; // Mark the code as compiled
    while(pos != std::string::npos) command.replace(pos, 1, outputfilepath),pos = command.find('^');
    pos = command.find('*');
    while(pos != std::string::npos) command.replace(pos, 1, codename),pos = command.find('*');
    std::string regcommand = "reg subprocess ";
    regcommand += codename;
    std::string echo = send_message(session, regcommand);
    if(prefix(echo, 7) == "[ERROR]"){
        std::cout << echo << std::endl;
        return;
    }
    if(command.empty()){
        std::cout << "Error: No run command defined for language: " << info.language << std::endl;
        return;
    }
    // std::cout<< "Running command: " << command << std::endl;
    system(command.c_str());
}
// Print the content to the console, handling variables and newlines
// This function replaces variables with their values and prints the content to the console
void print(const std::string &content, NetworkSession& session) {
    for(int i=0;i<content.size();i++){
        char c = content[i];
        if(c == '$'){
            std::string varname;
            for(i=i+1;i<content.size();i++){
                if(content[i] == ' ' || content[i] == '\n' || content[i] == '\r'){
                    break;
                }
                varname += content[i];
            }
            if(definedvar.find(varname) != definedvar.end()){
                std::cout << definedvar[varname].getvalue(session);
            } else if(definedcode.find(varname) != definedcode.end()){
                std::cout << definedcode[varname].getvalue(session);
            } else {
                std::cout << "Error: Undefined variable: " << varname << std::endl;
            }
        }else if(c == '\n'){
            std::cout << std::endl;
        }else if(c == '\r'){
            // Ignore carriage return
        }else std::cout << c;
    }
}
std::string definevarcommand(defineinfo info, NetworkSession& session){
    for(std::string str:skipsynclist){
        if(info.name==str) return "";
    }
    std::string command;
    JSON j = {
        {"Name", info.name},
        {"Value", info.type == "code" ? info.codeinfo : info.content},
        {"Timestamp", time(0)}
    };
    command = "sync var " + j.dump();
    // std::co<<command<<std::endl;
    return command;
}
std::vector<std::string> readCly(std::string path);
void useCly(std::vector<std::string> lines,NetworkSession& session);
// Address a line of code, handling different operations like define, use, jump, import, print, etc.
// This function processes a line of code and performs the corresponding operation
void addressline(std::string line,int *lineid, NetworkSession& session){
    static bool defined = false;
    bool overdefine=0;
    static defineinfo info;
    bool definedoperation=0;
    std::string commit_command="";
    std::string content = line;
    if(defined && info.type == "code" && content[0] == '|'){
        content = content.substr(1);
        // std::co<<content<<" | "<<info.codeinfo<<std::endl;
        info.codeinfo += content + "\n";
        return;
    }else if(defined && info.type == "code" && content[0] != '|'){
        defined = false;
        overdefine = 1;
        definedcode[info.name] = info;
    }
    for(int i=0;i<operationlist.size();i++){
        if(prefix(line, operationlist[i].length()) == operationlist[i]){
            std::string content = line.substr(operationlist[i].length());
            while(content[0] == ' ') content = content.substr(1);
            if(operationlist[i] == "define"){
                definedoperation=1;
                // std::co << "Defining: " << content << std::endl;
                info=judgedefine(content, session);
                // std::co<<info<<std::endl;
                if(info.type == "code"){
                    defined = true;
                }else if(info.type == "var"){
                    definedvar[info.name] = info;
                    overdefine = 1;
                }else if(info.type == "position") definedposition[info.name] = *lineid;
                else std::cout << "Error: Unknown define type: " << info.type << std::endl;
                // if(!command.empty()){
                //     std::string echo=send_message(session, command);
                //     std::cout<<echo<<std::endl;
                //     if(echo.substr(0,sizeof("[ERROR]"))=="[ERROR]"){
                //         std::cout<<echo<<std::endl;
                //     }
                // }
                definedline[info.name] = *lineid;
                break;
            } else if(operationlist[i] == "use"){
                definedoperation=1;
                // std::co << "Using: " << content << std::endl;
                // std::co<< info <<std::endl;
                usedefine(content, session);
                break;
            } else if(operationlist[i] == "jump"){
                definedoperation=1;
                // std::co << "Jumping to: " << content << std::endl;
                // std::co<< info <<std::endl;
                *lineid = definedline[content];
                if(definedposition.find(content) == definedposition.end()) usedefine(content, session);
                break;
            } else if(operationlist[i] == "printwithoutanewline"){
                definedoperation=1;
                print(content, session);
                break;
            } else if(operationlist[i] == "print"){
                definedoperation=1;
                print(content + " \n", session);
                break;
            }else if (operationlist[i] == "do") {
                definedoperation=1;
                // std::co << "Executing: " << content << std::endl;
                std::string command = "";
                for(int j = 0; j < content.size(); j++) {
                    if (content[j] == '$') {
                        std::string varname="";
                        for (j = j + 1; j < content.size(); j++) {
                            if (content[j] == ' ' || content[j] == '\n' || content[j] == '\r') {
                                break;
                            }
                            varname += content[j];
                        }
                        // std::co<<varname<<std::endl;
                        if (definedvar.find(varname) != definedvar.end()) {
                            command+=definedvar[varname].getvalue(session);
                        } else if(definedcode.find(varname) != definedcode.end()) {
                            command+=definedcode[varname].getvalue(session);
                        } else {
                            std::cout << "Error: Undefined variable: " << varname << std::endl;
                        }
                    } else {
                        command += content[j];
                    }
                }
                // std::co << "Command to execute: " << command << std::endl;
                int *fake_lineid = new int(-1);
                addressline(command, fake_lineid, session);
                delete fake_lineid;
                break;
            } else if (operationlist[i] == "exit") {
                definedoperation=1;
                exit(0);
            } else if(operationlist[i] == "if") {
                definedoperation=1;
                // std::co << "If: " << content << std::endl;
                int varnum=0;
                defineinfo var1,var2;
                std::string varname;
                for(char c : content){
                    if(c=='$'){
                        varnum++;
                    } else if(c==' '){
                        if(varnum == 1) {
                            if(definedcode.find(varname)!=definedcode.end()){
                                var1=definedcode[varname];
                            } else if(definedvar.find(varname)!=definedvar.end()){
                                var1=definedvar[varname];
                            } else {
                                std::cout << "Error: Undefined variable: " << varname << std::endl;
                            }
                        } else if (varnum == 2) {
                            if(definedcode.find(varname)!=definedcode.end()){
                                var2=definedcode[varname];
                            } else if(definedvar.find(varname)!=definedvar.end()){
                                var2=definedvar[varname];
                            } else {
                                std::cout << "Error: Undefined variable: " << varname << std::endl;
                            }
                        }
                        varname="";
                    } else varname+=c;
                    // else break;
                }
                // std::co<<var1<<std::endl<<var2<<std::endl<<varname<<std::endl;
                if(var1.getvalue(session)==var2.getvalue(session)) usedefine(varname, session);
                break;
            } else if(operationlist[i] == "ifn") {
                definedoperation=1;
                // std::co << "Ifn: " << content << std::endl;
                int varnum=0;
                defineinfo var1,var2;
                std::string varname;
                for(char c : content){
                    if(c=='$'){
                        varnum++;
                    } else if(c==' '){
                        if(varnum == 1) {
                            if(definedcode.find(varname)!=definedcode.end()){
                                var1=definedcode[varname];
                            } else if(definedvar.find(varname)!=definedvar.end()){
                                var1=definedvar[varname];
                            } else {
                                std::cout << "Error: Undefined variable: " << varname << std::endl;
                            }
                        } else if (varnum == 2) {
                            if(definedcode.find(varname)!=definedcode.end()){
                                var2=definedcode[varname];
                            } else if(definedvar.find(varname)!=definedvar.end()){
                                var2=definedvar[varname];
                            } else {
                                std::cout << "Error: Undefined variable: " << varname << std::endl;
                            }
                        }
                        varname="";
                    } else varname+=c;
                    // else break;
                }
                // std::co<<var1<<std::endl<<var2<<std::endl<<varname<<std::endl;
                if(var1.getvalue(session)!=var2.getvalue(session)) usedefine(varname, session);
                break;
            } else if(operationlist[i] == "import lib"){
                definedoperation=1;
                useCly(readCly(content), session);
                break;
            } else if(operationlist[i] == "commitvaroperation"){
                definedoperation=1;
                std::string echo = send_message(session, content);
                if(prefix(echo, 7) == "[ERROR]"){
                    std::cout << echo << std::endl;
                    return;
                }
                break;
            }
        }
    }
    if(!definedoperation) std::cout<<"Unknown Command:"<<line<<std::endl;
    if(overdefine){  // TODO:
        commit_command=definevarcommand(info, session);
        if(!commit_command.empty()){
            std::string echo=send_message(session, commit_command);
            // std::co<<echo<<std::endl;
            if(prefix(echo, 7) == "[ERROR]"){
                std::cout<<echo<<std::endl;
            }
        }
    }
}
// Read a Coly file and return its lines, handling imports
std::vector<std::string> readCly(std::string path){
    std::ifstream stream(path);
    if(!stream.is_open()){
        return {"Error: Cannot open file " + path};
    }
    std::vector<std::string> lines;
    std::string line;
    while(getline(stream, line)){
        if(prefix(line, sizeof("import lib")) == "import lib "){
            std::string importpath = line.substr(sizeof("import lib"));
            // std::co<< "Importing library: " << importpath << std::endl;
            bool isabs = false;
            for(char c:importpath) if(c == ':'){ isabs = true; break; }
            if(!isabs){
                importpath = COLYPATH;
                importpath += "/lib/";
                importpath += importpath;
            }
            std::vector<std::string> importinfo=readCly(importpath);
            for(std::string importline : importinfo){
                lines.push_back(importline);
            }
            continue;
        }
        lines.push_back(line);
    }
    stream.close();
    // for(std::string line:lines) std::cout<<line<<std::endl;
    return lines;
}
// Use a Coly file, processing its lines and handling jumps
void useCly(std::vector<std::string> lines,NetworkSession& session){
    for(int lineid=0;lineid<lines.size();lineid++){
        std::string line = lines[lineid];
        if(line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        int beforelineid = lineid;
        addressline(line, &lineid, session);
        if(beforelineid != lineid) {
            lineid--; // Adjust lineid if it was changed by a jump
        }
    }
}

#endif // __COLY_HPP__