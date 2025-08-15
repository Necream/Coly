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
vector<var> getglobalvariables(vector<string> lines){
    lines=deletenamespace(lines);
    lines=deletecomments(lines);
    for(string line:lines) cout<<line<<endl;
    int layer=0;
    vector<var> globalvariables;
    var v;
    bool isvar=0;
    for(string line:lines){
        string word="";
        bool skip=0;
        for(char c:line){
            skip=0;
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
                        if(word=="const"||word=="static"||word=="extern"||word=="volatile"){
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


#endif // __CPLUSPLUSSYNC_HPP__