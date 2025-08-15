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
vector<string> getglobalvariables(vector<string> &lines){
    int layer=0;
    vector<string> globalvariables;
    for(string line:lines){
        string word="";
        for(char c:line){
            if(c==' '||c=='\t'){
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