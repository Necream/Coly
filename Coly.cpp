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
using namespace std;
using JSON = nlohmann::json;
#ifdef _WIN32
#define COLYPATH "C:\\Coly\\"
#else
#define COLYPATH "/lib/Coly/"
#endif
// Map to store the line number of defined variables, codes and positions
// This is used to check if a variable or code is defined before use
map<string, int> definedline;
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
vector<string> operationlist = {
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
vector<string> skipsynclist = {
    "Input",
    "InputLine"
};
// Structure to hold information about defined variables, codes, and positions
// It includes type, name, language, content, code information, and variable type
struct defineinfo {
    string type; // code or var
    string name; // name of the variable/code/positon
    string language; // language type, e.g., C++, Python, etc.
    string content; // content of the variable
    string codeinfo; // additional code information
    string vartype;
    defineinfo(){
        type = "";
        name = "";
        language = "";
        content = "";
        codeinfo = "";
        vartype = "string";
    }
    friend ostream& operator<<(ostream &os, const defineinfo &info) {
        os  << "Type: "     << info.type     << endl
        << "Name: "     << info.name     << endl
        << "Language: " << info.language << endl
        << "Content: "  << info.content  << endl
        << "CodeInfo: " << info.codeinfo << endl;
        return os;
    }
    string getvalue(NetworkSession& session){
        string commit_command="get var ";
        commit_command+=this->name;
        string echo;
        bool skip=0;
        for(string str:skipsynclist){
            if(str==this->name) skip=1;
        }
        if(!skip) echo=send_message(session,commit_command);
        JSON j;
        if(!skip) j=JSON::parse(echo);
        if(type == "var") {
            if(!skip) this->content=j["Value"];
            string content = this->content;
            if(name == "InputLine"){
                getline(cin, content);
            }else if(name == "Input"){
                string temp;
                cin>> content;
                cin.ignore();
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
map<string, defineinfo> definedcode;
// Map to store defined positions
map<string, int> definedposition;
// Map to store defined variables
map<string, defineinfo> definedvar;
// Map to store compiled codes
map<string, bool> compiledcode;
// Get the prefix of a string with a specified length
string prefix(string str,int len){
    if(str.length() < len) return str;
    return str.substr(0, len);
}
// Judge the type of a define operation, register the variable or code and return the defineinfo
defineinfo judgedefine(string content, NetworkSession& session){
    //0: type 1: named 2: with 3: codeinfo/varinfo 4:vartype 5:| 6: codevar
    defineinfo info;
    int infotype = 0;
    string varname="";
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
                    // cout<<varname<<endl;
                    info.content = definedvar[varname].getvalue(session);
                    varname = "";
                }else cout << "Error: Undefined variable: " << varname << endl;
            }else varname += c;
            // cout<<c;
        }
    }
    if(!varname.empty() && var){
        // cout<<varname<<endl;
        if(definedvar.find(varname) == definedvar.end()){
            cout << "Error: Undefined variable: " << varname << endl;
        }
        info.content = definedvar[varname].getvalue(session);
        // cout<<info.content<<endl;
        varname = "";
    }else if(!varname.empty() && !var){
        info.content = varname;
    }
    // cout<<info<<endl;
    return info;
}
// Get the file extension for a given language from LanguageMap.json
string getextension(const string language) {
    string LanguageJSONPath = COLYPATH;
    LanguageJSONPath+="Settings/LanguageMap.json";
    FILE *stream=fopen(LanguageJSONPath.c_str(), "r");
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
// Get the need of compile for a given language from LanguageMap.json
bool getneedcompile(const string language) {
    string LanguageJSONPath = COLYPATH;
    LanguageJSONPath+="Settings/LanguageMap.json";
    FILE *stream=fopen(LanguageJSONPath.c_str(), "r");
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
    return json[language]["needcompile"].get<bool>();
}
// Get the compiler run command for a given language from LanguageMap.json
string getcompilerun(const string language) {
    string LanguageJSONPath = COLYPATH;
    LanguageJSONPath+="Settings/LanguageMap.json";
    FILE *stream=fopen(LanguageJSONPath.c_str(), "r");
    if (!stream) {
        cout << "Error: Cannot open LanguageMap.json" << endl;
        return ".txt"; // Default run if file cannot be opened
    }
    int c=0;
    string fileinfo;
    while((c=fgetc(stream))!=-1) fileinfo += (char)c;
    fclose(stream);
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(fileinfo);
    } catch (const nlohmann::json::parse_error& e) {
        cout << "Error: JSON parse error: " << e.what() << endl;
        return ""; // Default run if parsing fails
    }
    return json[language]["compilerun"].get<string>();
}
// Get the run command for a given language from LanguageMap.json
string getrun(const string language) {
    string LanguageJSONPath = COLYPATH;
    LanguageJSONPath+="Settings/LanguageMap.json";
    FILE *stream=fopen(LanguageJSONPath.c_str(), "r");
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
// TODO: 3rd language variable sync service
// Use a defined code, compile it if necessary, and run it
void usedefine(string content){
    if(definedline.find(content) == definedline.end()){
        cout << "Error: Undefined variable or code: " << content << endl;
        return;
    }
    defineinfo info = definedcode[content];
    string extension = getextension(info.language);
    string filename = COLYPATH;
    filename += "TempCode/";
    filename += GXPass::number2ABC(GXPass::compile(info.name))+"."+extension;
    FILE *fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        cout << "Error: Cannot open file " << filename << endl;
        return;
    }
    for(char c:info.codeinfo) fputc(c, fp);
    fclose(fp);
    string command = "";
    string codename = GXPass::number2ABC(GXPass::compile(info.name));
    if((compiledcode.find(codename) != compiledcode.end() && compiledcode[codename]) || !getneedcompile(info.language)){
        command=getrun(info.language); // If the code has been compiled, just run it
    } else getcompilerun(info.language); // If the code has not been compiled, compile it first
    size_t pos = command.find('$');
    while(pos != string::npos)command.replace(pos, 1, filename),pos = command.find('$');
    pos = command.find('^');
    compiledcode[codename] = true; // Mark the code as compiled
    while(pos != string::npos)command.replace(pos, 1, codename),pos = command.find('^');
    if(command.empty()){
        cout << "Error: No run command defined for language: " << info.language << endl;
        return;
    }
    // cout<< "Running command: " << command << endl;
    system(command.c_str());
}
// Print the content to the console, handling variables and newlines
// This function replaces variables with their values and prints the content to the console
void print(const string &content, NetworkSession& session) {
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
                cout << definedvar[varname].getvalue(session);
            } else if(definedcode.find(varname) != definedcode.end()){
                cout << definedcode[varname].getvalue(session);
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
string definevarcommand(defineinfo info, NetworkSession& session){
    for(string str:skipsynclist){
        if(info.name==str) return "";
    }
    string command;
    command="set var {\"Name\":\"";
    command+=info.name;
    command+="\",\"Value\":\"";
    if(info.type=="code"){
        command+=info.codeinfo;
    }else{
        command+=info.content;
    }
    command+="\",\"TimeStamp\":\"";
    command+=GXPass::c12c2<int,string>(time(0));
    command+="\"}";
    // cout<<command<<endl;
    return command;
}
vector<string> readCly(string path);
void useCly(vector<string> lines,NetworkSession& session);
// Address a line of code, handling different operations like define, use, jump, import, print, etc.
// This function processes a line of code and performs the corresponding operation
void addressline(string line,int *lineid, NetworkSession& session){
    static bool defined = false;
    bool overdefine=0;
    static defineinfo info;
    bool definedoperation=0;
    string commit_command="";
    string content = line;
    if(defined && info.type == "code" && content[0] == '|'){
        content = content.substr(1);
        // cout<<content<<" | "<<info.codeinfo<<endl;
        info.codeinfo += content + "\n";
        return;
    }else if(defined && info.type == "code" && content[0] != '|'){
        defined = false;
        overdefine = 1;
        definedcode[info.name] = info;
    }
    for(int i=0;i<operationlist.size();i++){
        if(prefix(line, operationlist[i].length()) == operationlist[i]){
            string content = line.substr(operationlist[i].length());
            while(content[0] == ' ') content = content.substr(1);
            if(operationlist[i] == "define"){
                definedoperation=1;
                // cout << "Defining: " << content << endl;
                info=judgedefine(content, session);
                // cout<<info<<endl;
                if(info.type == "code"){
                    defined = true;
                }else if(info.type == "var"){
                    definedvar[info.name] = info;
                    overdefine = 1;
                }else if(info.type == "position") definedposition[info.name] = *lineid;
                else cout << "Error: Unknown define type: " << info.type << endl;
                // if(!command.empty()){
                //     string echo=send_message(session, command);
                //     cout<<echo<<endl;
                //     if(echo.substr(0,sizeof("[ERROR]"))=="[ERROR]"){
                //         cout<<echo<<endl;
                //     }
                // }
                definedline[info.name] = *lineid;
                break;
            } else if(operationlist[i] == "use"){
                definedoperation=1;
                // cout << "Using: " << content << endl;
                // cout<< info <<endl;
                usedefine(content);
                break;
            } else if(operationlist[i] == "jump"){
                definedoperation=1;
                // cout << "Jumping to: " << content << endl;
                // cout<< info <<endl;
                *lineid = definedline[content];
                if(definedposition.find(content) == definedposition.end()) usedefine(content);
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
                // cout << "Executing: " << content << endl;
                string command = "";
                for(int j = 0; j < content.size(); j++) {
                    if (content[j] == '$') {
                        string varname="";
                        for (j = j + 1; j < content.size(); j++) {
                            if (content[j] == ' ' || content[j] == '\n' || content[j] == '\r') {
                                break;
                            }
                            varname += content[j];
                        }
                        // cout<<varname<<endl;
                        if (definedvar.find(varname) != definedvar.end()) {
                            command+=definedvar[varname].getvalue(session);
                        } else if(definedcode.find(varname) != definedcode.end()) {
                            command+=definedcode[varname].getvalue(session);
                        } else {
                            cout << "Error: Undefined variable: " << varname << endl;
                        }
                    } else {
                        command += content[j];
                    }
                }
                // cout << "Command to execute: " << command << endl;
                int *fake_lineid = new int(-1);
                addressline(command, fake_lineid, session);
                delete fake_lineid;
                break;
            } else if (operationlist[i] == "exit") {
                definedoperation=1;
                exit(0);
            } else if(operationlist[i] == "if") {
                definedoperation=1;
                // cout << "If: " << content << endl;
                int varnum=0;
                defineinfo var1,var2;
                string varname;
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
                                cout << "Error: Undefined variable: " << varname << endl;
                            }
                        } else if (varnum == 2) {
                            if(definedcode.find(varname)!=definedcode.end()){
                                var2=definedcode[varname];
                            } else if(definedvar.find(varname)!=definedvar.end()){
                                var2=definedvar[varname];
                            } else {
                                cout << "Error: Undefined variable: " << varname << endl;
                            }
                        }
                        varname="";
                    } else varname+=c;
                    // else break;
                }
                // cout<<var1<<endl<<var2<<endl<<varname<<endl;
                if(var1.getvalue(session)==var2.getvalue(session)) usedefine(varname);
                break;
            } else if(operationlist[i] == "ifn") {
                definedoperation=1;
                // cout << "Ifn: " << content << endl;
                int varnum=0;
                defineinfo var1,var2;
                string varname;
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
                                cout << "Error: Undefined variable: " << varname << endl;
                            }
                        } else if (varnum == 2) {
                            if(definedcode.find(varname)!=definedcode.end()){
                                var2=definedcode[varname];
                            } else if(definedvar.find(varname)!=definedvar.end()){
                                var2=definedvar[varname];
                            } else {
                                cout << "Error: Undefined variable: " << varname << endl;
                            }
                        }
                        varname="";
                    } else varname+=c;
                    // else break;
                }
                // cout<<var1<<endl<<var2<<endl<<varname<<endl;
                if(var1.getvalue(session)!=var2.getvalue(session)) usedefine(varname);
                break;
            } else if(operationlist[i] == "import lib"){
                definedoperation=1;
                useCly(readCly(content), session);
                break;
            } else if(operationlist[i] == "commitvaroperation"){
                definedoperation=1;
                string echo = send_message(session, content);
                if(echo.substr(0,sizeof("[ERROR]")-1)=="[ERROR]"){
                    cout<<echo<<endl;
                }
                break;
            }
        }
    }
    if(!definedoperation) cout<<"Unknown Command:"<<line<<endl;
    if(overdefine){  // TODO:
        commit_command=definevarcommand(info, session);
        if(!commit_command.empty()){
            string echo=send_message(session, commit_command);
            // cout<<echo<<endl;
            if(echo.substr(0,sizeof("[ERROR]"))=="[ERROR]"){
                cout<<echo<<endl;
            }
        }
    }
}
// Read a Coly file and return its lines, handling imports
vector<string> readCly(string path){
    ifstream stream(path);
    if(!stream.is_open()){
        return {"Error: Cannot open file " + path};
    }
    vector<string> lines;
    string line;
    while(getline(stream, line)){
        if(prefix(line, sizeof("import lib")) == "import lib "){
            string importpath = line.substr(sizeof("import lib"));
            // cout<< "Importing library: " << importpath << endl;
            bool isabs = false;
            for(char c:importpath) if(c == ':'){ isabs = true; break; }
            if(!isabs){
                importpath = COLYPATH;
                importpath += "/lib/";
                importpath += importpath;
            }
            vector<string> importinfo=readCly(importpath);
            for(string importline : importinfo){
                lines.push_back(importline);
            }
            continue;
        }
        lines.push_back(line);
    }
    stream.close();
    // for(string line:lines) cout<<line<<endl;
    return lines;
}
// Use a Coly file, processing its lines and handling jumps
void useCly(vector<string> lines,NetworkSession& session){
    for(int lineid=0;lineid<lines.size();lineid++){
        string line = lines[lineid];
        if(line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        int beforelineid = lineid;
        addressline(line, &lineid, session);
        if(beforelineid != lineid) {
            lineid--; // Adjust lineid if it was changed by a jump
        }
    }
}
#ifdef _WIN32
// Main function to run the Coly interpreter
// It takes command line arguments to specify the Coly file to run
int main(int argc, char *argv[]){
    std::string host = "127.0.0.1";
    std::string port = "12345";
    NetworkSession session;
    if (!connect_to_server(session, host, port)) {
        std::cout << "Failed to connect to server" << std::endl;
        return 1;
    }
    string command="reg process ";
    command+=GXPass::number2ABC(GXPass::compile(GXPass::c12c2<int,string>(time(0))));
    string echo = send_message(session, command);
    // cout<<echo<<endl;
    if(argc==1){
        useCly(readCly("InteractiveColy.cly"),session);
    }
    if(argc==2){
        useCly(readCly(argv[1]),session);
    }
    close_connection(session);
    return 0;
}
#else
int main(int argc, char *argv[]){
    std::string host = "127.0.0.1";
    std::string port = "12345";
    NetworkSession session;
    if (!connect_to_server(session, host, port)) {
        std::cout << "Failed to connect to server" << std::endl;
        return 1;
    }
    string command="reg process ";
    command+=GXPass::number2ABC(GXPass::compile(GXPass::c12c2<NetworkSession,string>(session)));
    string echo = send_message(session, command);
    // cout<<echo<<endl;
    if(argc==1){
        useCly(readCly("/lib/Coly/InteractiveColy.cly"),session);
    }
    if(argc==2){
        useCly(readCly(argv[1]),session);
    }
    close_connection(session);
    return 0;
}
#endif