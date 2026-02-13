#include <iostream>
#include <memory>
#include <set>
#include <functional>
#include <string>
#include <cstring>
#include "GXPass.hpp"
// 添加ASIO的独立模式定义
#define ASIO_STANDALONE
#include "asio.hpp"
#include "libVarContainer.hpp" // 包含 Var 和 PorcessContainer 的定义

using asio::ip::tcp;
using namespace std;

string GetPrefix(const string& str,size_t length){
    // 如果 length 大于字符串长度，就直接返回整个字符串
    if (length>=str.size()){
        return str;
    }
    return str.substr(0,length);
}
struct Operation{
    string OperationValue;
    int id;
};


struct ServerSession;
string CommandExecutor(string command,shared_ptr<ServerSession> client);


vector<Operation> operations;
MemoryContainer memory_container; // 全局内存容器
map<string,string> proof_map; // 用于存储进程的登录凭证
map<shared_ptr<ServerSession>,string> session_map; // 用于存储会话与进程ID的映射
map<string,vector<string>> subprocess_map; // 用于存储进程与其子进程ID的映射
void OperationInit(){
    operations.push_back({"set",1});
    operations.push_back({"get",2});
    operations.push_back({"del",3});
    operations.push_back({"sync",4});
    operations.push_back({"reg",5});
    operations.push_back({"login",6});
    operations.push_back({"process",1});
    operations.push_back({"var",2});
    operations.push_back({"subprocess",3});
}

// 会话类
struct ServerSession : enable_shared_from_this<ServerSession>{
    tcp::socket socket;
    char read_buf[4096]; // 4KB 缓冲区
    set<shared_ptr<ServerSession>>& clients;

    ServerSession(tcp::socket sock, set<shared_ptr<ServerSession>>& all_clients)
        : socket(move(sock)), clients(all_clients) {}

    // 启动会话
    void start() {
        read_message();
    }

    // 读取数据
    void read_message() {
        auto self = shared_from_this();
        socket.async_read_some(asio::buffer(read_buf),
            [this, self](error_code ec, size_t length) {
                if (!ec) {
                    string msg(read_buf, length);
                    cout<<msg<<endl;

                    send_message(CommandExecutor(msg,self));
                    // cout<<memory_container.to_json().dump(4)<<endl; // 输出当前内存状态

                    // 继续读取
                    read_message();
                } else {
                    if(session_map.find(self)!=session_map.end()) cout << "Client(ProcessID:"<<session_map[self]<<") disconnected\n";
                    else cout << "Client(ClientID:"<<self<<") disconnected\n";
                    close(self);
                }
            });
    }

    // 发送数据
    void send_message(const string& msg) {
        auto self = shared_from_this();
        asio::async_write(socket, asio::buffer(msg),
            [this, self](error_code ec, size_t /*length*/) {
                if (ec) {
                    close(self);
                }
            });
    }

    // 关闭连接
    void close(shared_ptr<ServerSession> client) {
        if(session_map.find(client)!=session_map.end()){
            clients.erase(shared_from_this());
            socket.close();
            // 删除该会话对应的子进程映射
            if(subprocess_map.find(session_map[client]) != subprocess_map.end()){
                for(const string& subpid:subprocess_map[session_map[client]]){
                    proof_map.erase(subpid); // 删除子进程凭证
                }
                subprocess_map.erase(session_map[client]); // 删除子进程映射
                return;
            }
            memory_container.process_container.erase(session_map[client]); // 删除进程容器
            cout<<"Session(ProcessID:"<<session_map[client]<<") closed and resources cleaned up."<<endl;
            session_map.erase(client); // 从会话映射中删除
        }
    }
};

// 接受新连接
void start_accepting(asio::io_context& io, tcp::acceptor& acceptor, set<shared_ptr<ServerSession>>& clients) {
    acceptor.async_accept(
        [&](error_code ec, tcp::socket socket) {
            if (!ec) {
                cout << "New client connected\n";
                auto session = make_shared<ServerSession>(move(socket), clients);
                clients.insert(session);
                session->start();
            }
            start_accepting(io, acceptor, clients); // 继续等待
        });
}

string CommandExecutor(string command,shared_ptr<ServerSession> client){
    int operation_id=0;
    for(const auto op:operations){
        if(GetPrefix(command,op.OperationValue.size())==op.OperationValue){
            operation_id*=10;
            operation_id+=op.id;
            command.erase(0,min(command.size(),op.OperationValue.size()+1)); // 去掉操作前缀
        }
    }
    if(operation_id!=51&&operation_id!=63){ // 非注册或登录操作需要验证会话
        if(session_map.find(client)== session_map.end()){
            cout<<"[ERROR]Client not registered or logined, please register or login first."<<endl;
            return "[ERROR]Client not registered or logined, please register or login first.";
        }
    }
    if(operation_id==0){
        cout<<"[ERROR]Unknown command: "<<command<<endl;
        return "[ERROR]Unknown command"+command;
    }
    if(operation_id==1){ // set
        cout<<"[ERROR]Can't set memory directly, use specific commands like set process or set var."<<endl;
        return "[ERROR]Can't set memory directly, use specific commands like set process or set var.";
    }
    if(operation_id==2){ // get
        cout<<"[ERROR]Can't get memory directly, use specific commands like get process or get var."<<endl;
        return "[ERROR]Can't get memory directly, use specific commands like get process or get var.";
    }
    if(operation_id==3){ // del
        cout<<"[ERROR]Can't delete memory directly, use specific commands like del process or del var."<<endl;
        return "[ERROR]Can't delete memory directly, use specific commands like del process or del var.";
    }
    if(operation_id==4){ // sync
        cout<<"[ERROR]Can't sync memory directly, use specific commands like sync process or sync var."<<endl;
        return "[ERROR]Can't sync memory directly, use specific commands like sync process or sync var.";
    }
    if(operation_id==5){ // reg
        cout<<"[ERROR]Please use specific commands like reg process or reg subprocess."<<endl;
        return "[ERROR]Please use specific commands like reg process or reg subprocess.";
    }
    if(operation_id==6){ // login
        cout<<"[ERROR]Please use specific commands like login process or login subprocess."<<endl;
        return "[ERROR]Please use specific commands like login process or login subprocess.";
    }
    if(operation_id==11){ // set process
        string processid=session_map[client]; // 获取会话对应的进程ID
        json j;
        try{
            j = json::parse(command);
        }
        catch(const json::parse_error& e){
            cout<<"[ERROR]Failed to parse JSON: "<<e.what()<<endl;
            return "[ERROR]Failed to parse JSON: "+string(e.what());
        }
        ProcessContainer pc;
        pc.from_json(j);  // 使用 from_json 替代赋值
        memory_container.process_container[processid] = pc;
        cout<<"Process operation completed"<<endl;
        return "Process operation completed";
    }
    if(operation_id==12){ // set var
        string processid=session_map[client]; // 获取会话对应的进程ID
        json j;
        try{
            j = json::parse(command);
        }
        catch(const json::parse_error& e){
            cout<<"[ERROR]Failed to parse JSON: "<<e.what()<<endl;
            return "[ERROR]Failed to parse JSON: "+string(e.what());
        }
        string varid=GXPass::number2ABC(GXPass::compile(j["Name"]));
        VarContainer v;
        v.from_json(j);  // 使用 from_json 替代赋值
        memory_container.process_container[processid].Vars[varid] = v;
        cout<<"Var operation completed"<<endl;
        return "Var operation completed";
    }
    if(operation_id==21){ // get process
        string processid=session_map[client]; // 获取会话对应的进程ID
        if(memory_container.process_container.find(processid)==memory_container.process_container.end()){
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client); // 注销会话
            return "[ERROR]Process not found.You can register now.";
        }
        json j=memory_container.process_container[processid].to_json();
        return j.dump();
    }
    if(operation_id==22){ // get var
        string processid=session_map[client]; // 获取会话对应的进程ID
        string varid=GXPass::number2ABC(GXPass::compile(command));
        if(memory_container.process_container.find(processid)==memory_container.process_container.end()){
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client); // 注销会话
            return "[ERROR]Process not found.You can register now.";
        }
        if(memory_container.process_container[processid].Vars.find(varid)==memory_container.process_container[processid].Vars.end()){
            cout<<"[ERROR]Var not found"<<endl;
            return "[ERROR]Var not found";
        }
        json j=memory_container.process_container[processid].Vars[varid].to_json();
        return j.dump();
    }
    if(operation_id==31){ // del process
        string processid=session_map[client]; // 获取会话对应的进程ID
        if(memory_container.process_container.find(processid)==memory_container.process_container.end()){
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client); // 注销会话
            return "[ERROR]Process not found.You can register now.";
        }
        memory_container.process_container.erase(processid);
        session_map.erase(client); // 注销会话
        // 删除该会话对应的子进程映射
        if(subprocess_map.find(processid) != subprocess_map.end()){
            for(const string& subpid:subprocess_map[processid]){
                proof_map.erase(subpid); // 删除子进程凭证
            }
            subprocess_map.erase(processid); // 删除子进程映射
        }
        cout<<"Process deleted"<<endl;
        return "Process deleted";
    }
    if(operation_id==32){ // del var
        string processid=session_map[client]; // 获取会话对应的进程ID
        string varid=GXPass::number2ABC(GXPass::compile(command));
        if(memory_container.process_container.find(processid)==memory_container.process_container.end()){
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client); // 注销会话
            return "[ERROR]Process not found.You can register now.";
        }
        if(memory_container.process_container[processid].Vars.find(varid)==memory_container.process_container[processid].Vars.end()){
            cout<<"[ERROR]Var not found"<<endl;
            return "[ERROR]Var not found";
        }
        memory_container.process_container[processid].Vars.erase(varid);
        cout<<"Var deleted"<<endl;
        return "Var deleted";
    }
    if(operation_id==41){ // sync process
        string processid=session_map[client]; // 获取会话对应的进程ID
        if(memory_container.process_container.find(processid)==memory_container.process_container.end()){
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client); // 注销会话
            return "[ERROR]Process not found.You can register now.";
        }
        json j;
        try{
            j = json::parse(command);
        }
        catch(const json::parse_error& e){
            cout<<"[ERROR]Failed to parse JSON: "<<e.what()<<endl;
            return "[ERROR]Failed to parse JSON: "+string(e.what());
        }
        ProcessContainer new_pc;
        new_pc.from_json(j);  // 使用 from_json 替代赋值
        memory_container.process_container[processid].Sync(new_pc);
        cout<<"Process sync completed"<<endl;
        return "Process sync completed";
    }
    if(operation_id==42){ // sync var
        string processid=session_map[client]; // 获取会话对应的进程ID
        if(memory_container.process_container.find(processid)==memory_container.process_container.end()){
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client); // 注销会话
            return "[ERROR]Process not found.You can register now.";
        }
        json j;
        try{
            j = json::parse(command);
        }
        catch(const json::parse_error& e){
            cout<<"[ERROR]Failed to parse JSON: "<<e.what()<<endl;
            return "[ERROR]Failed to parse JSON: "+string(e.what());
        }
        string varid=GXPass::number2ABC(GXPass::compile(j["Name"]));
        if(memory_container.process_container[processid].Vars.find(varid)==memory_container.process_container[processid].Vars.end()){
            cout<<"[ERROR]Var not found"<<endl;
            return "[ERROR]Var not found";
        }
        VarContainer new_var;
        new_var.from_json(j);  // 使用 from_json 替代赋值
        memory_container.process_container[processid].Vars[varid].Sync(new_var);
        cout<<"Var sync completed"<<endl;
        return "Var sync completed";
    }
    //TODO:
    if(operation_id==51){ // reg process   This will auto login process
        string processid=GXPass::number2ABC(GXPass::compile(command));
        if(session_map.find(client) != session_map.end()){
            cout<<"[ERROR]Client already registered, please use a different command."<<endl;
            return "[ERROR]Client already registered, please use a different command.";
        }
        if( memory_container.process_container.find(processid) != memory_container.process_container.end()){
            cout<<"[ERROR]Process already exists, please use a different process ID."<<endl;
            return "[ERROR]Process already exists, please use a different process ID.";
        }
        session_map[client] = processid; // 将会话与进程ID关联
        ProcessContainer pc;
        memory_container.process_container[processid] = pc; // 初始化进程容器
        cout<<"Process "<<processid<<" registered."<<endl;
        return "Process registered";
    }
    if(operation_id==53){ // reg subprocess
        string subprocessid=GXPass::number2ABC(GXPass::compile(command));
        if(proof_map.find(subprocessid) != proof_map.end()){
            cout<<"[ERROR]Subprocess already exists, please use a different subprocess ID."<<endl;
            return "[ERROR]Subprocess already exists, please use a different subprocess ID.";
        }
        if(subprocess_map.find(session_map[client]) == subprocess_map.end()){
            proof_map[subprocessid] = session_map[client]; // 初始化子进程凭证
            subprocess_map[session_map[client]].push_back(subprocessid); // 记录子进程ID
        }else{
            cout<<"[ERROR]Subprocess("<<subprocessid<<") already exists for this parent process."<<endl;
            return "[ERROR]Subprocess(" + subprocessid + ") already exists for this parent process.";
        }
        cout<<proof_map[subprocessid]<<endl;
        cout<<"Subprocess registered."<<endl;
        return "Subprocess registered";
    }
    if(operation_id==63){ // login subprocess
        string subprocessid=GXPass::number2ABC(GXPass::compile(command));
        if(proof_map.find(subprocessid) == proof_map.end()){
            cout<<"[ERROR]Subprocess not found, please register first."<<endl;
            return "[ERROR]Subprocess not found, please register first.";
        }
        session_map[client] = proof_map[subprocessid]; // 将会话与子进程ID关联
        proof_map.erase(subprocessid); // 清除子进程凭证
        cout<<"Subprocess logged in."<<endl;
        return "Subprocess logged in";
    }
    cout<<"[ERROR]Unknown command: "<<command<<endl;
    return "[ERROR]Unknown command"+command;
}

int main(int argc, char* argv[]){
    OperationInit(); // 初始化操作列表
    try{
        asio::io_context io;
        int port = 12345; // 监听端口
        if(argc==2){
            port=atoi(argv[1]);
        }
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port));
        set<shared_ptr<ServerSession>> clients;

        cout<<"Server running on port "<<port<<"...\n";
        start_accepting(io, acceptor, clients);

        io.run();

    }catch(exception& e){
        cerr<<"Exception: "<<e.what()<<"\n";
    }
}
