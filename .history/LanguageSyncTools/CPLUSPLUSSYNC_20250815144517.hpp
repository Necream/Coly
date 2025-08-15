#ifndef __CPLUSPLUSSYNC_HPP__
#define __CPLUSPLUSSYNC_HPP__

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;
struct var{
    string type;
    string name;
    void clear(){
        type="";
        name="";
    }
};
vector<string> deletenamespace(vector<string> lines){
    vector<string> newlines;
    for(string line:lines){
        string target = "std::";
        size_t pos = line.find(target);
        if (pos != std::string::npos) {
            line.erase(pos, target.length());
        }
        newlines.push_back(line);
    }
    return newlines;
}
vector<string> deletecomments(vector<string> lines){
    vector<string> newlines;
    for(string line:lines){
        size_t pos = line.find("//");
        if (pos != std::string::npos) {
            line.erase(pos);
        }
        newlines.push_back(line);
    }
    return newlines;
}
vector<string> deleteblockcomments(vector<string> lines){
    vector<string> newlines;
    bool inblockcomment = false;
    for(string line:lines){
        size_t startpos = line.find("/*");
        size_t endpos = line.find("*/");
        if (inblockcomment) {
            if (endpos != std::string::npos) {
                line.erase(0, endpos + 2);
                inblockcomment = false;
            } else {
                continue; // Skip this line entirely
            }
        }
        while (startpos != std::string::npos) {
            if (endpos != std::string::npos && endpos > startpos) {
                line.erase(startpos, endpos - startpos + 2);
                startpos = line.find("/*", startpos);
                endpos = line.find("*/", startpos);
            } else {
                line.erase(startpos);
                inblockcomment = true;
                break; // Exit the loop to handle the rest of the line in the next iteration
            }
        }
        if (!inblockcomment) {
            newlines.push_back(line);
        }
    }
    return newlines;
}
vector<string> deletepreprocessor(vector<string> lines){
    vector<string> newlines;
    for(string line:lines){
        if(line.empty()) continue;
        if(line[0]=='#') continue;
        newlines.push_back(line);
    }
    return newlines;
}
vector<string> deleteliterals(vector<string> lines){
    vector<string> newlines;
    for(string line:lines){
        string new_line="";
        bool in_string_literal = false;
        for(char c:line){
            if(c=='"'){
                in_string_literal = !in_string_literal; // Toggle the state
            }
            if(!in_string_literal && (c=='\'' || c=='`')) continue; // Skip character literals
            new_line += c;
        }
        newlines.push_back(new_line);
    }
    return newlines;
}
vector<var> getglobalvariables(vector<string> lines){
    lines=deletenamespace(lines);
    lines=deletecomments(lines);
    lines=deleteblockcomments(lines);
    lines=deletepreprocessor(lines);
    lines=deleteliterals(lines);
    int layer=0;
    vector<var> globalvariables;
    var v;
    bool isvar=0;
    for(string line:lines){
        string word="";
        bool skip=0;
        for(char c:line){
            if(skip){
                if(c==';') skip=0;
                continue;
            }
            if(c==' '||c=='\t'||c==','||c==';'){
                if(!word.empty() && layer==0){
                    if(isvar==1){
                        bool notvar=0;
                        for(char c:word) if(c=='('||c==')'||c=='{'||c=='}'||c=='['||c==']'||c=='"'||c=='\''||c=='.'||c==';') notvar=1;
                        if(notvar) v.clear();
                        if(notvar) isvar=0;
                        if(isvar==0) continue;
                        v.name=word;
                        word="";
                        globalvariables.push_back(v);
                        if(c!=',') isvar=0;
                    }else if(isvar==0){
                        if(word=="int"||word=="float"||word=="double"||word=="char"||word=="string"){
                            v.type=word;
                            word="";
                            isvar=1;
                            continue;
                        }
                        if(word=="const"||word=="static"||word=="extern"||word=="volatile"||word=="constexpr"){
                            skip=1;
                            continue;
                        }
                    }
                }
                word="";
                continue;
            }
            if(c=='{') layer++;
            else if(c=='}') layer--;
            if(layer==0){
                word+=c;
            }
        }
    }
    return globalvariables;
}
vector<string> insertsynccode(vector<string> lines, const vector<var> syncvariables) {
    vector<string> newlines;
    new
}


#endif // __CPLUSPLUSSYNC_HPP__