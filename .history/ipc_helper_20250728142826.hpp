#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <memory>

namespace ipc {

// 错误码枚举
enum class ErrorCode {
    SUCCESS = 0,
    SHARED_MEMORY_CREATE_FAILED,
    SHARED_MEMORY_MAP_FAILED,
    PIPE_CREATE_FAILED,
    PIPE_CONNECT_FAILED,
    WRITE_FAILED,
    READ_FAILED
};

// 错误管理类
class ErrorManager {
private:
    static thread_local ErrorCode lastError;
    static thread_local std::string errorMessage;

public:
    static void setError(ErrorCode code, const std::string& message) {
        lastError = code;
        errorMessage = message;
    }

    static ErrorCode getLastError() {
        return lastError;
    }

    static std::string getLastErrorMessage() {
        return errorMessage;
    }

    static bool isSuccess() {
        return lastError == ErrorCode::SUCCESS;
    }
};

// 初始化线程局部存储变量
thread_local ErrorCode ErrorManager::lastError = ErrorCode::SUCCESS;
thread_local std::string ErrorManager::errorMessage = "";

// 共享内存类
class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size) {
        // Convert std::string to std::wstring for Unicode API
        std::wstring wname(name.begin(), name.end());
        m_hMapFile = CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            size,
            wname.c_str()
        );
        
        if (!m_hMapFile) {
            ErrorManager::setError(
                ErrorCode::SHARED_MEMORY_CREATE_FAILED,
                "Failed to create shared memory"
            );
            return;
        }

        m_pBuf = MapViewOfFile(
            m_hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            size
        );

        if (!m_pBuf) {
            CloseHandle(m_hMapFile);
            ErrorManager::setError(
                ErrorCode::SHARED_MEMORY_MAP_FAILED,
                "Failed to map view of file"
            );
            return;
        }

        ErrorManager::setError(ErrorCode::SUCCESS, "");
    }

    ~SharedMemory() {
        if (m_pBuf) {
            UnmapViewOfFile(m_pBuf);
        }
        if (m_hMapFile) {
            CloseHandle(m_hMapFile);
        }
    }

    bool write(const void* data, size_t size) {
        if (!m_pBuf) {
            ErrorManager::setError(
                ErrorCode::WRITE_FAILED,
                "Invalid shared memory buffer"
            );
            return false;
        }
        memcpy(m_pBuf, data, size);
        ErrorManager::setError(ErrorCode::SUCCESS, "");
        return true;
    }

    bool read(void* data, size_t size) {
        if (!m_pBuf) {
            ErrorManager::setError(
                ErrorCode::READ_FAILED,
                "Invalid shared memory buffer"
            );
            return false;
        }
        memcpy(data, m_pBuf, size);
        ErrorManager::setError(ErrorCode::SUCCESS, "");
        return true;
    }

private:
    HANDLE m_hMapFile;
    LPVOID m_pBuf;
};

// 命名管道类
class NamedPipe {
public:
    bool init(const std::string& name, bool isServer) {
        std::string pipeName = "\\\\.\\pipe\\" + name;
        
        if (isServer) {
            // Convert std::string to std::wstring for Unicode API
            std::wstring wPipeName(pipeName.begin(), pipeName.end());
            m_hPipe = CreateNamedPipeW(
                wPipeName.c_str(),
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                1,
                4096,
                4096,
                0,
                NULL
            );
        } else {
            // Convert std::string to std::wstring for Unicode API
            std::wstring wPipeName(pipeName.begin(), pipeName.end());
            m_hPipe = CreateFileW(
                wPipeName.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
            );
        }

        if (m_hPipe == INVALID_HANDLE_VALUE) {
            ErrorManager::setError(
                ErrorCode::PIPE_CREATE_FAILED,
                "Failed to create/open named pipe"
            );
            return false;
        }

        ErrorManager::setError(ErrorCode::SUCCESS, "");
        return true;
    }

    bool waitForClient() {
        if (!ConnectNamedPipe(m_hPipe, NULL)) {
            DWORD error = GetLastError();
            if (error != ERROR_PIPE_CONNECTED) {
                ErrorManager::setError(
                    ErrorCode::PIPE_CONNECT_FAILED,
                    "Failed to connect to client: " + std::to_string(error)
                );
                return false;
            }
        }
        ErrorManager::setError(ErrorCode::SUCCESS, "");
        return true;
    }

    ~NamedPipe() {
        if (m_hPipe != INVALID_HANDLE_VALUE) {
            CloseHandle(m_hPipe);
        }
    }

    bool write(const std::string& data) {
        DWORD bytesWritten;
        if (!WriteFile(m_hPipe, data.c_str(), data.size(), &bytesWritten, NULL)) {
            ErrorManager::setError(
                ErrorCode::WRITE_FAILED,
                "Failed to write to pipe"
            );
            return false;
        }
        ErrorManager::setError(ErrorCode::SUCCESS, "");
        return true;
    }

    bool read(std::string& outData) {
        char buffer[4096];
        DWORD bytesRead;
        
        if (!ReadFile(m_hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            ErrorManager::setError(
                ErrorCode::READ_FAILED,
                "Failed to read from pipe"
            );
            return false;
        }
        
        outData = std::string(buffer, bytesRead);
        ErrorManager::setError(ErrorCode::SUCCESS, "");
        return true;
    }

private:
    HANDLE m_hPipe = INVALID_HANDLE_VALUE;
};

} // namespace ipc