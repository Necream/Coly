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
vector<var> getglobalvariables(vector<string> lines){
    int layer=0;
    vector<var> globalvariables;
    var v;
    bool isvar=0;
    for(string line:lines){
        string word="";
        for(char c:line){
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