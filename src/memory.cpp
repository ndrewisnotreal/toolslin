#include "memory.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <cstring>

pid_t Memory::m_pid = -1;
uintptr_t Memory::m_baseAddress = 0;

bool Memory::Attach(const std::string& processName) {
    DIR* dir = opendir("/proc");
    if (!dir) {
        std::cerr << "[DEBUG] Failed to open /proc\n";
        return false;
    }

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (!isdigit(*ent->d_name)) continue;

        pid_t pid = std::stoi(ent->d_name);
        std::string cmdPath = std::string("/proc/") + ent->d_name + "/cmdline";
        std::ifstream cmdFile(cmdPath, std::ios::binary);
        std::string cmdLine;
        if (cmdFile) {
            std::getline(cmdFile, cmdLine, '\0');
            if (cmdLine.find(processName) != std::string::npos) {
                m_pid = pid;
                std::cerr << "[DEBUG] Found: " << processName << " PID: " << pid << " cmd: " << cmdLine.substr(0, 60) << "\n";
                break;
            }
        }
    }
    closedir(dir);

    if (m_pid == -1) {
        std::cerr << "[DEBUG] Process '" << processName << "' not found\n";
        return false;
    }

    std::string mapsPath = std::string("/proc/") + std::to_string(m_pid) + "/maps";
    std::ifstream mapsFile(mapsPath);
    std::string line;
    uintptr_t exeBase = 0;
    while (std::getline(mapsFile, line)) {
        size_t dashPos = line.find('-');
        if (dashPos != std::string::npos) {
            uintptr_t addr = std::stoull(line.substr(0, dashPos), nullptr, 16);

            if (line.find("DeadByDaylight-Win64-Shipping.exe") != std::string::npos) {
                m_baseAddress = addr;
                exeBase = addr;
                std::cerr << "[DEBUG] Found exe base: 0x" << std::hex << addr << "\n";
                break;
            }
        }
    }

    if (m_baseAddress == 0) {
        m_baseAddress = exeBase;
    }

    std::cerr << "[DEBUG] Attached: PID=" << m_pid << " Base=0x" << std::hex << m_baseAddress << std::dec << "\n";
    return m_pid != -1 && m_baseAddress != 0;

    // Suppress unused warnings
    (void)processName;
}

void Memory::Detach() {
    m_pid = -1;
    m_baseAddress = 0;
}

bool Memory::IsAttached() {
    return m_pid != -1;
}

pid_t Memory::GetProcessId() {
    return m_pid;
}

uintptr_t Memory::GetBaseAddress() {
    return m_baseAddress;
}

bool Memory::Read(uintptr_t address, void* buffer, size_t size) {
    if (m_pid == -1 || address < 0x10000) return false;
    
    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", m_pid);
    
    int fd = open(memPath, O_RDONLY);
    if (fd == -1) {
        static bool s_loggedOpen = false;
        if (!s_loggedOpen) {
            printf("[DEBUG] Memory::Read open /proc/pid/mem failed, errno=%d\n", errno);
            s_loggedOpen = true;
        }
        return false;
    }
    
    ssize_t bytesRead = pread64(fd, buffer, size, address);
    if (bytesRead < 0) {
        static bool s_loggedRead = false;
        if (!s_loggedRead) {
            printf("[DEBUG] Memory::Read pread64 failed, address=0x%lx, size=%zu, fd=%d, errno=%d\n", address, size, fd, errno);
            s_loggedRead = true;
        }
    }
    close(fd);
    
    return bytesRead == static_cast<ssize_t>(size);
}

bool Memory::Write(uintptr_t address, const void* buffer, size_t size) {
    if (m_pid == -1) return false;

    char memPath[64];
    snprintf(memPath, sizeof(memPath), "/proc/%d/mem", m_pid);

    int fd = open(memPath, O_WRONLY);
    if (fd == -1) {
        fd = open(memPath, O_RDWR);
    }
    
    if (fd == -1) return false;

    ssize_t bytesWritten = pwrite64(fd, buffer, size, address);
    close(fd);

    return bytesWritten == static_cast<ssize_t>(size);
}

std::string Memory::ReadString(uintptr_t address, size_t maxLength) {
    std::string result;
    result.resize(maxLength);
    if (Read(address, &result[0], maxLength)) {
        size_t nullPos = result.find('\0');
        if (nullPos != std::string::npos) {
            result.resize(nullPos);
        }
    } else {
        result.clear();
    }
    return result;
}

uintptr_t Memory::FindPattern(const std::string& moduleName, const std::string& pattern) {
    if (m_pid == -1 || m_baseAddress == 0) return 0;

    std::vector<std::pair<uint8_t, bool>> patternData;
    std::istringstream iss(pattern);
    std::string byteStr;
    while (iss >> byteStr) {
        if (byteStr == "?" || byteStr == "??") {
            patternData.push_back({0, false});
        } else {
            patternData.push_back({static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16)), true});
        }
    }

    uintptr_t start = 0;
    uintptr_t end = 0;
    
    static pid_t s_lastMapsPid = -1;
    bool shouldLogMaps = (m_pid != s_lastMapsPid);

    std::string mapsPath = "/proc/" + std::to_string(m_pid) + "/maps";
    FILE* maps = fopen(mapsPath.c_str(), "r");
    if (!maps) {
        if (shouldLogMaps) printf("[DEBUG] FindPattern: Failed to open %s, errno=%d\n", mapsPath.c_str(), errno);
    } else {
        if (shouldLogMaps) printf("[DEBUG] FindPattern: Successfully opened %s\n", mapsPath.c_str());
        char line[512];
        
        struct MappingInfo {
            uintptr_t start;
            uintptr_t end;
            std::string perms;
        };
        std::vector<MappingInfo> largeExecMappings;
        
        while (fgets(line, sizeof(line), maps)) {
            // Log any line mentioning the process name to see what Wine is doing
            if (shouldLogMaps && strstr(line, "DeadByDaylight")) {
                printf("[MAPS-DBG] %s", line);
            }
            
            char perms[5] = {0};
            uintptr_t tmpStart = 0, tmpEnd = 0;
            if (sscanf(line, "%lx-%lx %4s", &tmpStart, &tmpEnd, perms) >= 3) {
                // If it's the exact target
                if (strstr(line, moduleName.c_str()) && strstr(perms, "r-xp")) {
                    start = tmpStart;
                    end = tmpEnd;
                    break;
                }
                
                // Keep track of any r-xp mappings larger than 10MB as fallbacks
                if (strstr(perms, "r-xp") && (tmpEnd - tmpStart) > 10 * 1024 * 1024) {
                    largeExecMappings.push_back({tmpStart, tmpEnd, perms});
                }
            }
        }
        fclose(maps);
        
        if (start == 0 && end == 0 && !largeExecMappings.empty()) {
            if (shouldLogMaps) printf("[WARNING] Failed to find exact module %s. Falling back to largest r-xp mapping.\n", moduleName.c_str());
            // Find the largest one
            size_t maxSize = 0;
            for (const auto& m : largeExecMappings) {
                size_t sz = m.end - m.start;
                if (shouldLogMaps) printf("[MAPS-DBG] Candidate fallback: 0x%lx-0x%lx (%.2f MB)\n", m.start, m.end, (float)sz / (1024.0f*1024.0f));
                if (sz > maxSize) {
                    maxSize = sz;
                    start = m.start;
                    end = m.end;
                }
            }
        }
    }
    
    if (shouldLogMaps && start != 0 && end != 0) {
        s_lastMapsPid = m_pid;
    }

    if (start == 0 || end == 0) {
        printf("[WARNING] Could not find any suitable executable section for %s\n", moduleName.c_str());
        return 0;
    }

    // Only log the found range once per scan
    static bool s_loggedMaps = false;
    if (!s_loggedMaps) {
        printf("[INFO] %s .text section: 0x%lx - 0x%lx (size: %.2f MB)\n", 
               moduleName.c_str(), start, end, (float)(end - start) / (1024.0f * 1024.0f));
        s_loggedMaps = true;
    }

    const size_t chunkSize = 0x1000;
    size_t totalBytesRead = 0;
    size_t totalBytesSkipped = 0;

    std::vector<uintptr_t> matches;
    std::vector<uint8_t> buf(chunkSize);
    for (uintptr_t addr = start; addr < end; addr += chunkSize) {
        size_t toRead = std::min(chunkSize, (size_t)(end - addr));
        if (!Memory::Read(addr, buf.data(), toRead)) {
            totalBytesSkipped += toRead;
            continue;
        }
        totalBytesRead += toRead;

        for (size_t i = 0; i + patternData.size() <= toRead; i++) {
            bool match = true;
            for (size_t j = 0; j < patternData.size(); j++) {
                if (patternData[j].second && buf[i+j] != patternData[j].first) {
                    match = false;
                    break;
                }
            }
            if (match) {
                matches.push_back(addr + i);
                printf("[INFO] Pattern match #%zu at 0x%lx\n", matches.size(), addr + i);
            }
        }
    }

    if (matches.size() == 1) {
        return matches[0];
    }
    
    printf("[INFO] Total matches: %zu\n", matches.size());
    if (matches.empty()) {
        printf("[DEBUG] FindPattern '%s' failed. Scanned 0x%lx-0x%lx. Read: %zu bytes, Skipped: %zu bytes.\n",
               pattern.substr(0, std::min((size_t)10, pattern.length())).c_str(), start, end, totalBytesRead, totalBytesSkipped);

        static bool s_loggedSample = false;
        if (!s_loggedSample) {
            uint8_t sample[16];
            if (Memory::Read(start, sample, 16)) {
                printf("[DEBUG] Sample 16 bytes at 0x%lx: ", start);
                for (int i = 0; i < 16; i++) {
                    printf("%02X ", sample[i]);
                }
                printf("\n");
            }
            s_loggedSample = true;
        }
    } else {
        printf("[WARNING] FindPattern '%s' returned %zu matches! Skipping to avoid bad patches.\n", 
               pattern.substr(0, std::min((size_t)10, pattern.length())).c_str(), matches.size());
    }

    return 0;
}