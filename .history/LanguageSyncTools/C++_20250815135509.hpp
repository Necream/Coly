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
};
vector<var> getglobalvariables(vector<string> &lines){
    int layer=0;
    vector<string> globalvariables;
    var v;
    bool isvar=0;
    for(string line:lines){
        string word="";
        for(char c:line){
            if(c==' '||c=='\t'||){
                if(!word.empty() && layer==0){
                    if(isvar==1){
                        v.name=word;
                        isvar=0;
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

}


#endif // __CPLUSPLUSSYNC_HPP__