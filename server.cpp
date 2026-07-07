#include <iostream>
#include <memory>
#include <set>
#include <functional>
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "GXPass.hpp"
#define ASIO_STANDALONE
#include "asio.hpp"
#include "libVarContainer.hpp"

using asio::ip::tcp;
using namespace std;

string GetPrefix(const string& str,size_t length){
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

struct WaitState{
    mutex m;
    condition_variable cv;
    atomic<unsigned long long> version{0};
};

vector<Operation> operations;
mutex map_mutex;
mutex process_mutex;
mutex wait_mutex;
MemoryContainer memory_container;
map<string,string> proof_map;
map<shared_ptr<ServerSession>,string> session_map;
map<string,map<string,string>> subprocess_map;
map<string,shared_ptr<WaitState>> process_wait_map;
map<string,shared_ptr<WaitState>> var_wait_map;

void OperationInit(){
    operations.push_back({"set",1});
    operations.push_back({"get",2});
    operations.push_back({"del",3});
    operations.push_back({"sync",4});
    operations.push_back({"reg",5});
    operations.push_back({"login",6});
    operations.push_back({"wait",7});
    operations.push_back({"process",1});
    operations.push_back({"var",2});
    operations.push_back({"subprocess",3});
}

struct ServerSession{
    tcp::socket socket;
    string subprocess_id;

    ServerSession(tcp::socket sock) : socket(move(sock)) {}
};

shared_ptr<WaitState> get_wait_state(map<string,shared_ptr<WaitState>>& states, const string& key){
    lock_guard<mutex> lock(wait_mutex);
    auto& state = states[key];
    if(!state){
        state = make_shared<WaitState>();
    }
    return state;
}

void signal_wait_state(map<string,shared_ptr<WaitState>>& states, const string& key){
    shared_ptr<WaitState> state;
    {
        lock_guard<mutex> lock(wait_mutex);
        auto it = states.find(key);
        if(it == states.end()){
            state = make_shared<WaitState>();
            states[key] = state;
        }else{
            state = it->second;
        }
    }
    state->version.fetch_add(1, memory_order_release);
    state->cv.notify_all();
}

set<string> collect_var_ids(const ProcessContainer& pc){
    set<string> ids;
    for(const auto& [varid, _] : pc.Vars){
        ids.insert(varid);
    }
    return ids;
}

string wait_for_process_exit(const string& processid){
    auto state = get_wait_state(process_wait_map, processid);
    unsigned long long snapshot;
    {
        lock_guard<mutex> process_lock(process_mutex);
        if(memory_container.process_container.find(processid) == memory_container.process_container.end()){
            return "Process already exited";
        }
        snapshot = state->version.load(memory_order_acquire);
    }
    unique_lock<mutex> lock(state->m);
    state->cv.wait(lock, [&]{
        return state->version.load(memory_order_acquire) != snapshot;
    });
    return "Process exited";
}

string wait_for_var_change(const string& processid, const string& varid){
    auto state = get_wait_state(var_wait_map, processid + "\x1f" + varid);
    unsigned long long snapshot;
    long long baseline_timestamp = -1;
    {
        lock_guard<mutex> process_lock(process_mutex);
        auto pit = memory_container.process_container.find(processid);
        if(pit == memory_container.process_container.end()){
            return "Var changed";
        }
        auto vit = pit->second.Vars.find(varid);
        if(vit == pit->second.Vars.end()){
            return "Var changed";
        }
        baseline_timestamp = vit->second.Timestamp;
        snapshot = state->version.load(memory_order_acquire);
    }
    unique_lock<mutex> lock(state->m);
    state->cv.wait(lock, [&]{
        return state->version.load(memory_order_acquire) != snapshot;
    });
    return "Var changed";
}

void close_session(shared_ptr<ServerSession> client){
    lock_guard<mutex> lock(map_mutex);

    if(session_map.find(client) == session_map.end()){
        cout << "Unregistered client disconnected\n";
        return;
    }

    string related_process_id = session_map[client];

    if(!client->subprocess_id.empty()){
        string sub_id = client->subprocess_id;
        proof_map.erase(sub_id);
        if(subprocess_map.find(related_process_id) != subprocess_map.end()){
            subprocess_map[related_process_id].erase(sub_id);
            if(subprocess_map[related_process_id].empty()){
                subprocess_map.erase(related_process_id);
                // 子进程全部退出了，检查父进程是否还在
                bool parent_gone = true;
                for(const auto& [sess, pid] : session_map){
                    if(pid == related_process_id){
                        parent_gone = false;
                        break;
                    }
                }
                if(parent_gone){
                    lock_guard<mutex> p_lock(process_mutex);
                    memory_container.process_container.erase(related_process_id);
                    cout << "Parent clean up completed after last subprocess disconnected\n";
                }
            }
        }
        session_map.erase(client);
        cout << "Subprocess(" << sub_id << ") disconnected\n";
        return;
    }

    // 检查是否有活跃的子进程会话
    bool has_active_subprocesses = false;
    for(const auto& [sess, pid] : session_map){
        if(pid == related_process_id && sess != client){
            has_active_subprocesses = true;
            break;
        }
    }

    if(has_active_subprocesses){
        // 有子进程还在运行，保留 process_container 和子进程映射
        session_map.erase(client);
        cout << "Parent process(" << related_process_id << ") disconnected, subprocesses still active, keeping process container\n";
        return;
    }

    // 没有活跃子进程，清理全部
    if(subprocess_map.find(related_process_id) != subprocess_map.end()){
        for(const auto& [sub_id, val] : subprocess_map[related_process_id]){
            proof_map.erase(sub_id);
        }
        subprocess_map.erase(related_process_id);
    }
    {
        lock_guard<mutex> p_lock(process_mutex);
        memory_container.process_container.erase(related_process_id);
    }
    session_map.erase(client);
    cout << "Parent process(" << related_process_id << ") disconnected, all resources cleaned up\n";
}

void handle_client(tcp::socket sock){
    char buf[4096];
    auto client = make_shared<ServerSession>(move(sock));

    while(true){
        error_code ec;
        size_t n = client->socket.read_some(asio::buffer(buf), ec);
        if(ec) break;

        string msg(buf, n);
        cout << "Received message: " << msg << endl;

        string response = CommandExecutor(msg, client);

        asio::write(client->socket, asio::buffer(response), ec);
        if(ec) break;
    }

    close_session(client);
}

string CommandExecutor(string command,shared_ptr<ServerSession> client){
    int operation_id=0;
    for(const auto op:operations){
        if(GetPrefix(command,op.OperationValue.size())==op.OperationValue){
            operation_id*=10;
            operation_id+=op.id;
            command.erase(0,min(command.size(),op.OperationValue.size()+1));
        }
    }
    if(operation_id!=51&&operation_id!=63){
        lock_guard<mutex> lock(map_mutex);
        if(session_map.find(client)== session_map.end()){
            cout<<"[ERROR]Client not registered or logined, please register or login first."<<endl;
            return "[ERROR]Client not registered or logined, please register or login first.";
        }
    }
    if(operation_id==0){
        cout<<"[ERROR]Unknown command: "<<command<<endl;
        return "[ERROR]Unknown command"+command;
    }
    if(operation_id==1){
        cout<<"[ERROR]Can't set memory directly, use specific commands like set process or set var."<<endl;
        return "[ERROR]Can't set memory directly, use specific commands like set process or set var.";
    }
    if(operation_id==2){
        cout<<"[ERROR]Can't get memory directly, use specific commands like get process or get var."<<endl;
        return "[ERROR]Can't get memory directly, use specific commands like get process or get var.";
    }
    if(operation_id==3){
        cout<<"[ERROR]Can't delete memory directly, use specific commands like del process or del var."<<endl;
        return "[ERROR]Can't delete memory directly, use specific commands like del process or del var.";
    }
    if(operation_id==4){
        cout<<"[ERROR]Can't sync memory directly, use specific commands like sync process or sync var."<<endl;
        return "[ERROR]Can't sync memory directly, use specific commands like sync process or sync var.";
    }
    if(operation_id==5){
        cout<<"[ERROR]Please use specific commands like reg process or reg subprocess."<<endl;
        return "[ERROR]Please use specific commands like reg process or reg subprocess.";
    }
    if(operation_id==6){
        cout<<"[ERROR]Please use specific commands like login process or login subprocess."<<endl;
        return "[ERROR]Please use specific commands like login process or login subprocess.";
    }
    if(operation_id==11){
        string processid;
        {
            lock_guard<mutex> lock(map_mutex);
            processid = session_map[client];
        }
        set<string> old_var_ids;
        {
            lock_guard<mutex> lock(process_mutex);
            auto it = memory_container.process_container.find(processid);
            if(it != memory_container.process_container.end()){
                old_var_ids = collect_var_ids(it->second);
            }
        }
        json j;
        try{
            j = json::parse(command);
        }
        catch(const json::parse_error& e){
            cout<<"[ERROR]Failed to parse JSON: "<<e.what()<<endl;
            return "[ERROR]Failed to parse JSON: "+string(e.what());
        }
        ProcessContainer pc;
        pc.from_json(j);
        {
            lock_guard<mutex> lock(process_mutex);
            memory_container.process_container[processid] = pc;
        }
        set<string> affected_var_ids = old_var_ids;
        for(const auto& [varid, _] : pc.Vars){
            affected_var_ids.insert(varid);
        }
        for(const auto& varid : affected_var_ids){
            signal_wait_state(var_wait_map, processid + "\x1f" + varid);
        }
        cout<<"Process operation completed"<<endl;
        return "Process operation completed";
    }
    if(operation_id==12){
        string processid;
        {
            lock_guard<mutex> lock(map_mutex);
            processid = session_map[client];
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
        VarContainer v;
        v.from_json(j);
        {
            lock_guard<mutex> lock(process_mutex);
            memory_container.process_container[processid].Vars[varid] = v;
        }
        signal_wait_state(var_wait_map, processid + "\x1f" + varid);
        cout<<"Var operation completed"<<endl;
        return "Var operation completed";
    }
    if(operation_id==21){
        string processid;
        {
            lock_guard<mutex> lock(map_mutex);
            processid = session_map[client];
        }
        bool found;
        json j;
        {
            lock_guard<mutex> lock(process_mutex);
            found = memory_container.process_container.find(processid) != memory_container.process_container.end();
            if(found){
                j = memory_container.process_container[processid].to_json();
            }
        }
        if(!found){
            lock_guard<mutex> lock(map_mutex);
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client);
            return "[ERROR]Process not found.You can register now.";
        }
        return j.dump();
    }
    if(operation_id==22){
        string processid;
        {
            lock_guard<mutex> lock(map_mutex);
            processid = session_map[client];
        }
        string varid=GXPass::number2ABC(GXPass::compile(command));
        bool process_found;
        bool var_found;
        json j;
        {
            lock_guard<mutex> lock(process_mutex);
            process_found = memory_container.process_container.find(processid) != memory_container.process_container.end();
            if(process_found){
                var_found = memory_container.process_container[processid].Vars.find(varid) != memory_container.process_container[processid].Vars.end();
                if(var_found){
                    j = memory_container.process_container[processid].Vars[varid].to_json();
                }
            }else{
                var_found = false;
            }
        }
        if(!process_found){
            lock_guard<mutex> lock(map_mutex);
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client);
            return "[ERROR]Process not found.You can register now.";
        }
        if(!var_found){
            cout<<"[ERROR]Var not found"<<endl;
            return "[ERROR]Var not found";
        }
        return j.dump();
    }
    if(operation_id==31){
        string processid;
        {
            lock_guard<mutex> lock(map_mutex);
            processid = session_map[client];
        }
        bool found;
        {
            lock_guard<mutex> lock(process_mutex);
            found = memory_container.process_container.find(processid) != memory_container.process_container.end();
        }
        if(!found){
            lock_guard<mutex> lock(map_mutex);
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client);
            return "[ERROR]Process not found.You can register now.";
        }
        {
            lock_guard<mutex> lock(process_mutex);
            auto it = memory_container.process_container.find(processid);
            if(it != memory_container.process_container.end()){
                for(const auto& [varid, _] : it->second.Vars){
                    signal_wait_state(var_wait_map, processid + "\x1f" + varid);
                }
                memory_container.process_container.erase(it);
            }
        }
        {
            lock_guard<mutex> lock(map_mutex);
            if(subprocess_map.find(processid) != subprocess_map.end()){
                for(const auto& subpid:subprocess_map[processid]){
                    proof_map.erase(subpid.first);
                }
                subprocess_map.erase(processid);
            }
            session_map.erase(client);
        }
        signal_wait_state(process_wait_map, processid);
        cout<<"Process deleted"<<endl;
        return "Process deleted";
    }
    if(operation_id==32){
        string processid;
        {
            lock_guard<mutex> lock(map_mutex);
            processid = session_map[client];
        }
        string varid=GXPass::number2ABC(GXPass::compile(command));
        bool process_found;
        bool var_found;
        {
            lock_guard<mutex> lock(process_mutex);
            process_found = memory_container.process_container.find(processid) != memory_container.process_container.end();
            if(process_found){
                var_found = memory_container.process_container[processid].Vars.find(varid) != memory_container.process_container[processid].Vars.end();
                if(var_found){
                    memory_container.process_container[processid].Vars.erase(varid);
                }
            }else{
                var_found = false;
            }
        }
        if(process_found){
            signal_wait_state(var_wait_map, processid + "\x1f" + varid);
        }
        if(!process_found){
            lock_guard<mutex> lock(map_mutex);
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client);
            return "[ERROR]Process not found.You can register now.";
        }
        if(!var_found){
            cout<<"[ERROR]Var not found"<<endl;
            return "[ERROR]Var not found";
        }
        cout<<"Var deleted"<<endl;
        return "Var deleted";
    }
    if(operation_id==41){
        string processid;
        {
            lock_guard<mutex> lock(map_mutex);
            processid = session_map[client];
        }
        bool found;
        {
            lock_guard<mutex> lock(process_mutex);
            found = memory_container.process_container.find(processid) != memory_container.process_container.end();
        }
        if(!found){
            lock_guard<mutex> lock(map_mutex);
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client);
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
        new_pc.from_json(j);
        set<string> old_var_ids;
        {
            lock_guard<mutex> lock(process_mutex);
            auto it = memory_container.process_container.find(processid);
            if(it != memory_container.process_container.end()){
                old_var_ids = collect_var_ids(it->second);
            }
        }
        {
            lock_guard<mutex> lock(process_mutex);
            memory_container.process_container[processid].Sync(new_pc);
        }
        set<string> affected_var_ids = old_var_ids;
        for(const auto& [varid, _] : new_pc.Vars){
            affected_var_ids.insert(varid);
        }
        for(const auto& varid : affected_var_ids){
            signal_wait_state(var_wait_map, processid + "\x1f" + varid);
        }
        cout<<"Process sync completed"<<endl;
        return "Process sync completed";
    }
    if(operation_id==42){
        string processid;
        {
            lock_guard<mutex> lock(map_mutex);
            processid = session_map[client];
        }
        bool found;
        {
            lock_guard<mutex> lock(process_mutex);
            found = memory_container.process_container.find(processid) != memory_container.process_container.end();
        }
        if(!found){
            lock_guard<mutex> lock(map_mutex);
            cout<<"[ERROR]Process not found.You can register now."<<endl;
            session_map.erase(client);
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
        bool var_exists;
        {
            lock_guard<mutex> lock(process_mutex);
            if(memory_container.process_container[processid].Vars.find(varid)==memory_container.process_container[processid].Vars.end()){
                var_exists = false;
            }else{
                var_exists = true;
                VarContainer new_var;
                new_var.from_json(j);
                memory_container.process_container[processid].Vars[varid].Sync(new_var);
            }
        }
        if(!var_exists){
            json set_j = j;
            string set_command = "set var " + set_j.dump();
            return CommandExecutor(set_command, client);
        }
        cout<<"Var sync completed"<<endl;
        return "Var sync completed";
    }
    if(operation_id==51){
        string processid=GXPass::number2ABC(GXPass::compile(command));
        {
            lock_guard<mutex> lock(map_mutex);
            if(session_map.find(client) != session_map.end()){
                cout<<"[ERROR]Client already registered, please use a different command."<<endl;
                return "[ERROR]Client already registered, please use a different command.";
            }
        }
        {
            lock_guard<mutex> lock(process_mutex);
            if(memory_container.process_container.find(processid) != memory_container.process_container.end()){
                cout<<"[ERROR]Process already exists, please use a different process ID."<<endl;
                return "[ERROR]Process already exists, please use a different process ID.";
            }
        }
        {
            lock_guard<mutex> lock(map_mutex);
            session_map[client] = processid;
        }
        {
            lock_guard<mutex> lock(process_mutex);
            ProcessContainer pc;
            memory_container.process_container[processid] = pc;
        }
        cout<<"Process "<<processid<<" registered."<<endl;
        return "Process registered";
    }
    if(operation_id==53){
        string subprocessid=GXPass::number2ABC(GXPass::compile(command));
        lock_guard<mutex> lock(map_mutex);
        if(proof_map.find(subprocessid) != proof_map.end()){
            cout<<"[ERROR]Subprocess already exists, please use a different subprocess ID."<<endl;
            return "[ERROR]Subprocess already exists, please use a different subprocess ID.";
        }
        if(subprocess_map[session_map[client]].find(subprocessid) == subprocess_map[session_map[client]].end()){
            proof_map[subprocessid] = session_map[client];
            subprocess_map[session_map[client]][subprocessid] = "1";
        }else{
            cout<<"[ERROR]Subprocess("<<subprocessid<<") already exists for this parent process."<<endl;
            return "[ERROR]Subprocess(" + subprocessid + ") already exists for this parent process.";
        }
        cout<<proof_map[subprocessid]<<endl;
        cout<<"Subprocess registered."<<endl;
        return "Subprocess registered";
    }
    if(operation_id==63){
        string subprocessid=GXPass::number2ABC(GXPass::compile(command));
        string parent_id;
        {
            lock_guard<mutex> lock(map_mutex);
            if(proof_map.find(subprocessid) == proof_map.end()){
                cout<<"[ERROR]Subprocess not found, please register first."<<endl;
                return "[ERROR]Subprocess not found, please register first.";
            }
            parent_id = proof_map[subprocessid];
            session_map[client] = parent_id;
            proof_map.erase(subprocessid);
        }
        client->subprocess_id = subprocessid;
        cout<<"Subprocess logged in."<<endl;
        return "Subprocess logged in";
    }
    if(operation_id==71){
        string processid=GXPass::number2ABC(GXPass::compile(command));
        // 等待退出，即memory_container.process_container[processid]被删除
        return wait_for_process_exit(processid);
    }
    if(operation_id==72){
        string varid=GXPass::number2ABC(GXPass::compile(command));
        // 等待var改变
        string processid;
        {
            lock_guard<mutex> lock(map_mutex);
            if(session_map.find(client) == session_map.end()){
                cout<<"[ERROR]Client not registered or logined, please register or login first."<<endl;
                return "[ERROR]Client not registered or logined, please register or login first.";
            }
            processid = session_map[client];
        }
        return wait_for_var_change(processid, varid);
    }
    cout<<"[ERROR]Unknown command: "<<command<<endl;
    return "[ERROR]Unknown command: "+command;
}

int main(int argc, char* argv[]){
    OperationInit();
    try{
        asio::io_context io;
        int port = 12345;
        if(argc==2){
            port=atoi(argv[1]);
        }
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port));

        cout<<"Server running on port "<<port<<"...\n";

        while(true){
            error_code ec;
            tcp::socket sock = acceptor.accept(ec);
            if(ec) continue;

            cout << "New client connected\n";
            thread(handle_client, move(sock)).detach();
        }

    }catch(exception& e){
        cerr<<"Exception: "<<e.what()<<"\n";
    }
}
