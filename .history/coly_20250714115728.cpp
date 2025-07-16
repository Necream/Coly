#include <iostream>
#include <string>
#include <cstdio>
using namespace std;
void useCly(string path){
    FILE *stream=fopen(path.c_str(), "r");
    
}
int main(int argc, char *argv[]){
    if(argc==1){
        useCly("InteractiveColy.cly");
    }
    return 0;
}