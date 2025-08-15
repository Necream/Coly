#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <stdexcept>

namespace ipc {

// 操作结果类
class OperationResult {
public:
    bool success{true};
    std::string error_message;
    DWORD error_code{0};

    static OperationResult Success() {
        return OperationResult{};
    }

    static OperationResult Failure(const std::string& message, DWORD code = GetLastError()) {
        OperationResult result;
        result.success = false;
        result.error_message = message;
        result.error_code = code;
        return result;
    }

    operator bool() const { return success; }
};

// 共享内存类
class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size) : m_size(size) {
        try {
            init(name);
        } catch (const std::exception& e) {
            m_lastResult = OperationResult::Failure(e.what());
        }
    }

    ~SharedMemory() {
        try {
            if (m_pBuf) {
                UnmapViewOfFile(m_pBuf);
            }
            if (m_hMapFile) {
                CloseHandle(m_hMapFile);
            }
        } catch (...) {
            // 析构函数中最好不要抛出异常
        }
    }

    OperationResult write(const std::string& data) {
        try {
            if (!m_pBuf) {
                return OperationResult::Failure("Invalid shared memory buffer");
            }
            if (data.size() > m_size) {
                return OperationResult::Failure("Data too large for shared memory");
            }

            // 写入数据长度
            *static_cast<size_t*>(m_pBuf) = data.size();
            // 写入数据内容
            memcpy(static_cast<char*>(m_pBuf) + sizeof(size_t), 
                   data.data(), data.size());
            
            return OperationResult::Success();
        } catch (const std::exception& e) {
            return OperationResult::Failure(e.what());
        }
    }

    std::optional<std::string> read() {
        try {
            if (!m_pBuf) {
                m_lastResult = OperationResult::Failure("Invalid shared memory buffer");
                return std::nullopt;
            }

            size_t dataSize = *static_cast<size_t*>(m_pBuf);
            std::string data(static_cast<char*>(m_pBuf) + sizeof(size_t), dataSize);
            
            m_lastResult = OperationResult::Success();
            return data;
        } catch (const std::exception& e) {
            m_lastResult = OperationResult::Failure(e.what());
            return std::nullopt;
        }
    }

    const OperationResult& getLastResult() const { return m_lastResult; }
    bool isValid() const { return m_pBuf != nullptr; }

private:
    void init(const std::string& name) {
        std::wstring wname(name.begin(), name.end());
        m_hMapFile = CreateFileMappingW(
            INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
            0, m_size + sizeof(size_t), wname.c_str());
        
        if (!m_hMapFile) {
            throw std::runtime_error("Failed to create shared memory");
        }

        m_pBuf = MapViewOfFile(
            m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_size + sizeof(size_t));

        if (!m_pBuf) {
            CloseHandle(m_hMapFile);
            throw std::runtime_error("Failed to map view of file");
        }
    }

    HANDLE m_hMapFile{nullptr};
    LPVOID m_pBuf{nullptr};
    size_t m_size{0};
    OperationResult m_lastResult;
};

// 命名管道类
class NamedPipe {
public:
    static constexpr size_t BUFFER_SIZE = 4096;

    NamedPipe() = default;

    OperationResult init(const std::string& name, bool isServer) {
        try {
            std::wstring wpipeName = L"\\\\.\\pipe\\" + std::wstring(name.begin(), name.end());
            
            if (isServer) {
                m_hPipe = CreateNamedPipeW(
                    wpipeName.c_str(),
                    PIPE_ACCESS_DUPLEX,
                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                    1, BUFFER_SIZE, BUFFER_SIZE, 0, nullptr);
            } else {
                m_hPipe = CreateFileW(
                    wpipeName.c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    0, nullptr, OPEN_EXISTING, 0, nullptr);
            }

            if (m_hPipe == INVALID_HANDLE_VALUE) {
                return OperationResult::Failure("Failed to create/open named pipe");
            }

            return OperationResult::Success();
        } catch (const std::exception& e) {
            return OperationResult::Failure(e.what());
        }
    }

    ~NamedPipe() {
        try {
            if (m_hPipe != INVALID_HANDLE_VALUE) {
                CloseHandle(m_hPipe);
            }
        } catch (...) {}
    }

    OperationResult write(const std::string& data) {
        try {
            DWORD bytesWritten;
            if (!WriteFile(m_hPipe, data.c_str(), 
                static_cast<DWORD>(data.size()), &bytesWritten, nullptr)) {
                return OperationResult::Failure("Failed to write to pipe");
            }
            return OperationResult::Success();
        } catch (const std::exception& e) {
            return OperationResult::Failure(e.what());
        }
    }

    std::optional<std::string> read() {
        try {
            std::vector<char> buffer(BUFFER_SIZE);
            DWORD bytesRead;
            
            if (!ReadFile(m_hPipe, buffer.data(), Pipe, buffer.data(), 
                static_cast<DWORD>(buffer.size()), &bytesRead, nullptr)) {   static_cast<DWORD>(buffer.size()), &bytesRead, nullptr)) {
                m_lastResult = OperationResult::Failure("Failed to read from pipe");       m_lastResult = OperationResult::Failure("Failed to read from pipe");
                return std::nullopt;
            }
                   
            m_lastResult = OperationResult::Success();            m_lastResult = OperationResult::Success();
            return std::string(buffer.data(), bytesRead); std::string(buffer.data(), bytesRead);
        } catch (const std::exception& e) {
            m_lastResult = OperationResult::Failure(e.what());ionResult::Failure(e.what());
            return std::nullopt;   return std::nullopt;
        }   }
    }    }

    OperationResult waitForClient() {Client() {
        try {
            if (!ConnectNamedPipe(m_hPipe, nullptr)) {_hPipe, nullptr)) {
                DWORD error = GetLastError();or();
                if (error != ERROR_PIPE_CONNECTED) {ONNECTED) {
                    return OperationResult::Failure(      return OperationResult::Failure(
                        "Failed to connect to client", error);Failed to connect to client", error);
                }       }
            }
            return OperationResult::Success();perationResult::Success();
        } catch (const std::exception& e) {   } catch (const std::exception& e) {
            return OperationResult::Failure(e.what());            return OperationResult::Failure(e.what());
        }
    }

    const OperationResult& getLastResult() const { return m_lastResult; }t OperationResult& getLastResult() const { return m_lastResult; }

private:
    HANDLE m_hPipe{INVALID_HANDLE_VALUE};};
    OperationResult m_lastResult;
};

} // namespace ipc