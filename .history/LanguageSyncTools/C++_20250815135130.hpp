#ifndef __CPLUSPLUSSYNC_HPP__
#define __CPLUSPLUSSYNC_HPP__

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;
vector<string> getglobalvariables(vector<string> &lines){
    int layer=0;
    vector<string> globalvariables;
    for(string line:lines){
        string word="";
        for(char c:line){
            if(c==' '||c=='\t'){
                
            }
            if(c=='{') layer++;
            else if(c=='}') layer--;
        }
    }

}


#endif // __CPLUSPLUSSYNC_HPP__