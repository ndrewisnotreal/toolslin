#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Memory {
public:
    static bool Attach(const std::string& processName);
    static void Detach();
    static bool IsAttached();
    static pid_t GetProcessId();
    static uintptr_t GetBaseAddress();

    static bool Read(uintptr_t address, void* buffer, size_t size);
    static bool Write(uintptr_t address, const void* buffer, size_t size);

    static uintptr_t FindPattern(const std::string& moduleName, const std::string& pattern);

    template <typename T>
    static T Read(uintptr_t address) {
        T value{};
        Read(address, &value, sizeof(T));
        return value;
    }

    template <typename T>
    static bool Write(uintptr_t address, T value) {
        return Write(address, &value, sizeof(T));
    }

    static std::string ReadString(uintptr_t address, size_t maxLength = 256);

private:
    static pid_t m_pid;
    static uintptr_t m_baseAddress;
};
