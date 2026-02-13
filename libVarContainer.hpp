#ifndef __LIBVARCONTAINER_HPP__
#define __LIBVARCONTAINER_HPP__

#include <map>
#include <string>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

// 变量结构体
struct VarContainer{
    string Name;
    string Value;
    long long Timestamp;

    // 同步方法
    void Sync(VarContainer& other) {
        if (other.Timestamp > Timestamp) {
            *this = other;
        } else {
            other = *this;
        }
    }

    // JSON 序列化
    json to_json() const {
        return json{
            {"Name", Name},
            {"Value", Value},
            {"Timestamp", Timestamp}
        };
    }
    
    // JSON 反序列化
    void from_json(const json& j) {
        Name = j.value("Name", "");
        Value = j.value("Value", "");
        Timestamp = j.value("Timestamp", 0);
    }
};

// 进程容器结构体
struct ProcessContainer {
    map<string, VarContainer> Vars;
    long long Timestamp;

    // 同步方法
    void Sync(ProcessContainer& other) {
        if (other.Timestamp > Timestamp) {
            *this = other;
        } else {
            other = *this;
        }
    }

    // JSON 序列化
    json to_json() const {
        json vars_json;
        for (auto iter = Vars.begin(); iter != Vars.end(); ++iter) {
            vars_json[iter->first] = iter->second.to_json();
        }
        return json{
            {"vars", vars_json},
            {"Timestamp", Timestamp}
        };
    }
    
    // JSON 反序列化
    void from_json(const json& j) {
        Vars.clear();
        if (j.contains("vars") && j["vars"].is_object()) {
            const json& vars_json = j["vars"];
            for (auto iter = vars_json.begin(); iter != vars_json.end(); ++iter) {
                VarContainer var;
                var.from_json(iter.value());
                Vars[iter.key()] = var;
            }
        }
        Timestamp = j.value("Timestamp", 0);
    }
};

// 内存容器结构体
struct MemoryContainer {
    map<string, ProcessContainer> process_container;
    long long Timestamp;

    // 同步方法
    void Sync(MemoryContainer& other) {
        if (other.Timestamp > Timestamp) {
            *this = other;
        } else {
            other = *this;
        }
    }

    // JSON 序列化
    json to_json() const {
        json processes_json;
        for (auto iter = process_container.begin(); iter != process_container.end(); ++iter) {
            processes_json[iter->first] = iter->second.to_json();
        }
        return json{
            {"process_container", processes_json},
            {"Timestamp", Timestamp}
        };
    }

    // JSON 反序列化
    void from_json(const json& j) {
        process_container.clear();
        if (j.contains("process_container") && j["process_container"].is_object()) {
            const json& proc_json = j["process_container"];
            for (auto iter = proc_json.begin(); iter != proc_json.end(); ++iter) {
                ProcessContainer pc;
                pc.from_json(iter.value());
                process_container[iter.key()] = pc;
            }
        }
        Timestamp = j.value("Timestamp", 0);
    }

    // 清除所有数据
    void clear() {
        process_container.clear();
        Timestamp = 0;
    }
};

#endif // __LIBVARCONTAINER_HPP__