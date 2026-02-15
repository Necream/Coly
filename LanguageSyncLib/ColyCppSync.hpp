// TODO:Unfinished
#ifndef COLY_CPP_SYNC_HPP
#define COLY_CPP_SYNC_HPP

#include <string>
#include "./VariableSyncService.hpp"
#include "./json.hpp"
#include "./GXPass.hpp"

using JSON = nlohmann::json;

static NetworkSession *session = new NetworkSession();
// 连接服务器
static bool is_connected;

static std::string ColyProcessID="";

static std::string RegEcho;

struct ColySyncString;
static std::string sync_variable(const ColySyncString* var);

struct ColySyncString{
    std::string data;
    std::string varname;
    ColySyncString(const std::string& varname, const std::string& value=""){
        this->varname = varname;
        this->data = value;
        sync_variable(this);
    };
    ColySyncString(const std::string& varname, const char* str){
        this->varname = varname;
        this->data = str;
        sync_variable(this);
    }
    ColySyncString& operator=(const std::string& str) {
        data = str;
        sync_variable(this);
        return *this;
    }
    ColySyncString& operator=(const char* str) {
        data = str;
        sync_variable(this);
        return *this;
    }
    ColySyncString operator+(const ColySyncString& other) const {
        return ColySyncString(data + other.data);
    }
    ColySyncString& operator+=(const ColySyncString& other) {
        data += other.data;
        sync_variable(this);
        return *this;
    }
    bool operator==(const ColySyncString& other) const {
        return data == other.data;
    }
    bool operator!=(const ColySyncString& other) const {
        return data != other.data;
    }
    bool operator<(const ColySyncString& other) const {
        return data < other.data;
    }
    bool operator>(const ColySyncString& other) const {
        return data > other.data;
    }
    bool operator<=(const ColySyncString& other) const {
        return data <= other.data;
    }
    bool operator>=(const ColySyncString& other) const {
        return data >= other.data;
    }
    char& operator[](size_t pos) {
        sync_variable(this);
        return data[pos];
    }
    const char& operator[](size_t pos) const {
        sync_variable(this);
        return data[pos];
    }
    operator std::string() const {
        sync_variable(this);
        return data;
    }
    size_t length() const {
        sync_variable(this);
        return data.length();
    }
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    void clear() { data.clear(); }
    const char* c_str() const { return data.c_str(); }
    std::string substr(size_t pos = 0, size_t len = std::string::npos) const {
        return data.substr(pos, len);
    }
    size_t find(const std::string& str, size_t pos = 0) const {
        return data.find(str, pos);
    }
};
std::string sync_variable(const ColySyncString* var) {
    if (!is_connected) {
        return "[ERROR] Not connected to server.";
    }
    std::string var_name = var->varname;
    std::string var_value = var->data;
    JSON j = {
        {"Name", var_name},
        {"Value", var_value},
        {"Timestamp", time(0)}
    };
    std::string command = "sync var " + j.dump();
    std::string echo = send_message(*session, command);
    return echo;
}
#define RegColyVar(varname) ColySyncString varname(#varname, "")
#define InitColySyncService() do{if(argc<2){return 0;}else{ColyProcessID=argv[1];is_connected = connect_to_server(*session, "localhost", "12345");RegEcho = send_message(*session, "login subprocess " + ColyProcessID);}}while(0)
#endif // COLY_CPP_SYNC_HPP