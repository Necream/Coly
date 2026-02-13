#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif
#include <iostream>
#include <string>
#include "Coly.hpp"
using namespace std;
#ifdef _WIN32
// Main function to run the Coly interpreter
// It takes command line arguments to specify the Coly file to run
int main(int argc, char *argv[]){
    string host = "127.0.0.1";
    string port = "12345";
    NetworkSession session;
    if (!connect_to_server(session, host, port)) {
        cout << "Failed to connect to server" << endl;
        return 1;
    }
    if(argc == 3){
        string command = "login subprocess ";
        command += argv[2];
        string echo = send_message(session, command);
        useCly(readCly(argv[1]),session);
        close_connection(session);
        return 0;
    }
    string command="reg process ";
    command+=GXPass::number2ABC(GXPass::compile(GXPass::c12c2<int,string>(time(0))));
    string echo = send_message(session, command);
    // cout<<echo<<endl;
    if(argc==1){
        useCly(readCly("C:\\Coly\\InteractiveColy.cly"),session);
    }
    if(argc==2){
        // useCly(readCly(argv[1]),session);
        useCly(readCly("test.cly"),session);
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
    if(argc == 3){
        string command = "login subprocess ";
        command += argv[2];
        string echo = send_message(session, command);
        useCly(readCly(argv[1]),session);
        close_connection(session);
        return 0;
    }
    string command="reg process ";
    command+=GXPass::number2ABC(GXPass::compile(GXPass::c12c2<int,string>(time(0))));
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