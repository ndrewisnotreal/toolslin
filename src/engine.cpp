#include "engine.h"
#include "memory.h"
#include "classes.h"
#include <iostream>
#include <unordered_map>
#include <mutex>

static std::unordered_map<uint32_t, std::string> g_NameCache;
static std::mutex g_NameCacheMutex;

std::string GetNameFromId(uint32_t index) {
    {
        std::lock_guard<std::mutex> lock(g_NameCacheMutex);
        auto it = g_NameCache.find(index);
        if (it != g_NameCache.end()) return it->second;
    }

    uint32_t chunkOffset = index >> 16;
    uint16_t nameOffset = index & 0xFFFF;

    uintptr_t FNamePool = Memory::GetBaseAddress() + GOffsets.GNameOffset;

    // FNamePool -> Blocks (offset 0x10)
    uintptr_t namePoolChunk = Memory::Read<uintptr_t>(FNamePool + 0x10 + (chunkOffset * 8));
    if (!namePoolChunk) return "";

    // Decompiled logic: (nameOffset * 4) + chunkPtr
    uintptr_t entryOffset = namePoolChunk + (nameOffset * 4);

    // Header is at +4
    uint16_t header = Memory::Read<uint16_t>(entryOffset + 4);
    int nameLength = header >> 1;
    bool isWide = header & 1;

    std::string result;
    if (nameLength > 0 && nameLength < 512) {
        if (isWide) {
            // Wide string resolution (UTF-16) starts at +6
            for (int i = 0; i < nameLength; i++) {
                uint16_t ch = Memory::Read<uint16_t>(entryOffset + 6 + (i * 2));
                result += (ch < 256) ? (char)ch : '?';
            }
        } else {
            // Ansi string resolution starts at +6
            result = Memory::ReadString(entryOffset + 6, nameLength);
        }
    }

    if (!result.empty()) {
        std::lock_guard<std::mutex> lock(g_NameCacheMutex);
        g_NameCache[index] = result;
    }
    return result;
}
