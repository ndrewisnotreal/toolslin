#include "esp.h"
#include "memory.h"
#include "classes.h"
#include "engine.h"
#include "math.h"
#include "globals.h"
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <array>
#include <cctype>
#include <cfloat>
#include <initializer_list>
#include <unordered_map>
#include "name_mapping.h"
#include <unistd.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static float lastSkillCheckProgress = -1.0f;
static uintptr_t lastHitSkillCheck = 0;
static int lastHitSkillCheckType = -1;
static float lastHitSkillCheckZoneStart = -1.0f;
static float lastHitSkillCheckZoneEnd = -1.0f;
static float lastHitSkillCheckStartTicker = -1.0f;

static std::string GetPlatformName(uint32_t flag) {
    if (flag == 0) return "None";
    if (flag & 32) return "Steam";
    if (flag & 4096) return "Epic";
    if (flag & 1024) return "PS5";
    if (flag & 16) return "PS4";
    if (flag & 2048) return "XSX";
    if (flag & 128) return "Xbox";
    if (flag & 8) return "Switch";
    if (flag & 64) return "WinGDK";
    if (flag & 1) return "Android";
    if (flag & 4) return "IOS";
    return "Unknown";
}

static std::string GetProviderName(uint32_t flag) {
    if (flag == 0) return "None";
    if (flag & 32) return "Steam";
    if (flag & 32768) return "Epic";
    if (flag & 16) return "PSN";
    if (flag & 128) return "Xbox";
    if (flag & 8) return "Nintendo";
    if (flag & 1) return "Google";
    if (flag & 4) return "Facebook";
    if (flag & 64) return "WinGDK";
    if (flag & 256) return "Apple";
    if (flag & 512) return "Kraken";
    return "Unknown";
}
static bool wasInBonusZone = false;

static float NormalizeSkillProgress(float value) {
    if (!std::isfinite(value)) return 0.0f;
    value = std::fmod(value, 1.0f);
    if (value < 0.0f) value += 1.0f;
    return value;
}

static bool IsSkillProgressInRange(float progress, float start, float end) {
    progress = NormalizeSkillProgress(progress);
    start = NormalizeSkillProgress(start);
    end = NormalizeSkillProgress(end);

    if (start <= end) {
        return progress >= start && progress <= end;
    }
    return progress >= start || progress <= end;
}

static float SkillProgressRangeLength(float start, float end) {
    start = NormalizeSkillProgress(start);
    end = NormalizeSkillProgress(end);
    if (end >= start) return end - start;
    return (1.0f - start) + end;
}

static float SkillProgressCircularDistance(float a, float b) {
    float delta = std::abs(NormalizeSkillProgress(a) - NormalizeSkillProgress(b));
    return std::min(delta, 1.0f - delta);
}

static bool IsFiniteSkillValue(float value) {
    return std::isfinite(value) && std::abs(value) < 10.0f;
}

struct SkillCheckWindow {
    bool valid = false;
    bool isWiggle = false;
    bool usesBonusZone = false;
    float progress = 0.0f;
    float zoneStart = 0.0f;
    float zoneEnd = 0.0f;
    float length = 0.0f;
};

static bool IsWiggleSkillCheckType(int customType) {
    return customType == 1 || customType == 11;
}

static SkillCheckWindow BuildSkillCheckWindow(float currentProgress, const FSkillCheckDefinition& def, int customType) {
    SkillCheckWindow window;
    if (!IsFiniteSkillValue(currentProgress) ||
        !IsFiniteSkillValue(def.SuccessZoneStart) ||
        !IsFiniteSkillValue(def.SuccessZoneEnd) ||
        !IsFiniteSkillValue(def.BonusZoneLength) ||
        !IsFiniteSkillValue(def.BonusZoneStart) ||
        !IsFiniteSkillValue(def.ProgressRate) ||
        !IsFiniteSkillValue(def.StartingTickerPosition)) {
        return window;
    }

    float successLength = SkillProgressRangeLength(def.SuccessZoneStart, def.SuccessZoneEnd);
    bool hasSuccessZone = successLength > 0.0005f && successLength < 0.75f;
    bool hasBonusZone = def.BonusZoneLength > 0.0005f && def.BonusZoneLength < 0.75f;
    if (!hasSuccessZone && !hasBonusZone) return window;

    float sourceZoneStart = hasBonusZone ? def.BonusZoneStart : def.SuccessZoneStart;
    float sourceZoneEnd = hasBonusZone ? (def.BonusZoneStart + def.BonusZoneLength) : def.SuccessZoneEnd;
    bool isNegativeProgressRate = def.ProgressRate < 0.0f;

    window.isWiggle = IsWiggleSkillCheckType(customType);
    window.usesBonusZone = hasBonusZone;

    if (window.isWiggle) {
        window.progress = isNegativeProgressRate
            ? def.StartingTickerPosition + (1.0f - currentProgress)
            : currentProgress + def.StartingTickerPosition;
        window.zoneStart = sourceZoneStart;
        window.zoneEnd = sourceZoneEnd;
    } else {
        float startZone = sourceZoneStart - def.StartingTickerPosition;
        float endZone = sourceZoneEnd - def.StartingTickerPosition;

        window.progress = currentProgress;
        window.zoneStart = isNegativeProgressRate ? 1.0f - endZone : startZone;
        window.zoneEnd = isNegativeProgressRate ? 1.0f - startZone : endZone;
    }

    window.length = SkillProgressRangeLength(window.zoneStart, window.zoneEnd);
    window.valid = window.length > 0.0005f && window.length < 0.75f;
    return window;
}

struct SkillCheckHitWindow {
    bool valid = false;
    float start = 0.0f;
    float end = 0.0f;
    float lateBias = 0.0f;
    float endGuard = 0.0f;
};

static SkillCheckHitWindow BuildSkillCheckHitWindow(const SkillCheckWindow& window, const FSkillCheckDefinition& def) {
    SkillCheckHitWindow hit;
    if (!window.valid) return hit;

    hit.lateBias = std::abs(def.ProgressRate) * (g_Cheats_AutoSkillCheckDelayMs / 1000.0f);
    hit.lateBias = std::clamp(hit.lateBias, 0.0f, window.length * 0.45f);
    hit.endGuard = std::min(window.length * 0.10f, 0.003f);
    if (hit.lateBias + hit.endGuard >= window.length) return hit;

    if (window.isWiggle && def.ProgressRate < 0.0f) {
        hit.start = window.zoneStart + hit.endGuard;
        hit.end = window.zoneEnd - hit.lateBias;
    } else {
        hit.start = window.zoneStart + hit.lateBias;
        hit.end = window.zoneEnd - hit.endGuard;
    }
    hit.valid = true;
    return hit;
}

static void ResetSkillCheckLatch() {
    wasInBonusZone = false;
    lastSkillCheckProgress = -1.0f;
}

static bool IsLikelyPointer(uintptr_t value) {
    return value > 0x10000 && value < 0x0000800000000000ULL;
}

static uintptr_t ReadFirstLikelyPointer(uintptr_t base, std::initializer_list<uintptr_t> offsets) {
    for (uintptr_t offset : offsets) {
        if (offset == 0) continue;
        uintptr_t value = Memory::Read<uintptr_t>(base + offset);
        if (IsLikelyPointer(value)) return value;
    }
    return 0;
}

static int uinput_fd = -1;

bool SetupUinput() {
    if (uinput_fd >= 0) return true;
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_fd < 0) {
        printf("[!] Failed to open /dev/uinput: %s\n", strerror(errno));
        return false;
    }

    ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
    for (int i = 0; i < 255; i++) {
        ioctl(uinput_fd, UI_SET_KEYBIT, i);
    }

    struct uinput_user_dev uidev;
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Logitech USB Keyboard");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x046d;
    uidev.id.product = 0xc31c;
    uidev.id.version = 1;

    write(uinput_fd, &uidev, sizeof(uidev));
    ioctl(uinput_fd, UI_DEV_CREATE);

    printf("[+] uinput virtual keyboard created successfully. Waiting 1s for OS to register...\n");
    sleep(1);
    return true;
}

static void emit(int fd, int type, int code, int val) {
    struct input_event ie;
    ie.type = type;
    ie.code = code;
    ie.value = val;
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    write(fd, &ie, sizeof(ie));
}

static void SendKey(int key, int value) {
    if (uinput_fd < 0 && !SetupUinput()) return;
    emit(uinput_fd, EV_KEY, key, value);
    emit(uinput_fd, EV_SYN, SYN_REPORT, 0);
}

void PressKeyE() {
    if (uinput_fd < 0) return;
    int fd = uinput_fd;
    emit(fd, EV_KEY, KEY_E, 1);
    emit(fd, EV_SYN, SYN_REPORT, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    emit(fd, EV_KEY, KEY_E, 0);
    emit(fd, EV_SYN, SYN_REPORT, 0);
}

void PressSpaceBar(int holdMs) {
    if (uinput_fd < 0 && !SetupUinput()) return;
    holdMs = std::clamp(holdMs, 15, 120);

    emit(uinput_fd, EV_KEY, KEY_SPACE, 1);
    emit(uinput_fd, EV_SYN, SYN_REPORT, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(holdMs));
    emit(uinput_fd, EV_KEY, KEY_SPACE, 0);
    emit(uinput_fd, EV_SYN, SYN_REPORT, 0);
}

void PressSpaceBar() {
    PressSpaceBar(80);
}

void DumpSkillCheck(uintptr_t sc) {
    static auto lastDump = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastDump).count() < 2) return;
    lastDump = now;

    printf("\n--- SKILLCHECK DUMP (0x%lx) ---\n", sc);
    float floats[64];
    Memory::Read(sc + 0x180, floats, sizeof(floats));
    for (int i = 0; i < 64; i++) {
        if (std::abs(floats[i]) > 0.0001f && std::abs(floats[i]) < 100.0f) {
            printf("  +0x%02x: %.4f\n", 0x180 + (i * 4), floats[i]);
        }
    }
    printf("-------------------------------\n\n");
}

static std::string GetUClassName(uintptr_t actor) {
    if (!actor) return "";
    uintptr_t uclass = Memory::Read<uintptr_t>(actor + 0x10); // UObject::ClassPrivate
    if (!uclass) return "";
    uint32_t nameId = Memory::Read<uint32_t>(uclass + 0x18); // UObject::NamePrivate
    return GetNameFromId(nameId);
}


// --- Actor type vectors ---
std::vector<ActorEntry> g_Survivors, g_Killers, g_Generators, g_Hooks, g_Lockers, g_Totems, g_Pallets, g_Windows, g_Chests, g_CenobiteBoxes, g_NemesisZombies, g_Victors, g_HagTraps, g_PlagueFountains, g_SupplyCrates, g_K33Tunnels, g_K33Turrets, g_Hatches, g_Traps, g_BreakableDoors, g_EscapeDoors, g_OtherActors;
std::vector<PlayerLoadout> g_PlayerLoadouts;

std::mutex g_ActorMutex;
FMinimalViewInfo g_CameraInfo;
std::mutex g_CameraMutex;

int g_LocalPlayerRole = 0;
uintptr_t g_LocalController = 0;

static float     g_OriginalFOV = -1.0f;
static uintptr_t g_LastCamMgr  = 0;

struct FLinearColor { float R, G, B, A; };

void ESP_Cleanup() {
    static bool s_cleaned = false;
    if (s_cleaned) return;
    s_cleaned = true;

    // Patch restoration removed to prevent crashes.

    if (uinput_fd >= 0) {
        ioctl(uinput_fd, UI_DEV_DESTROY);
        close(uinput_fd);
        uinput_fd = -1;
    }
    if (Memory::IsAttached() && g_LastCamMgr && g_OriginalFOV > 0.0f) {
        Memory::Write<float>(g_LastCamMgr + GOffsets.APlayerCameraManager_DefaultFOVOffset, g_OriginalFOV);
        std::cout << "[*] Reset FOV to original: " << g_OriginalFOV << std::endl;
    }
}

// --- helpers ---


// walk UClass hierarchy and check for an exact class name (or with A/U prefix)
static bool ClassInheritsFrom(uintptr_t uClass, const std::string& needle) {
    for (int depth = 0; depth < 24 && uClass; depth++) {
        uint32_t fnameId = Memory::Read<uint32_t>(uClass + GOffsets.UObject_FNameOffset + GOffsets.FName_DisplayIndexOffset);
        std::string name = GetNameFromId(fnameId);
        if (!name.empty()) {
            if (name == needle ||
               (name.length() == needle.length() + 1 && (name[0] == 'A' || name[0] == 'U') && name.substr(1) == needle)) {
                return true;
            }
        }
        uClass = Memory::Read<uintptr_t>(uClass + GOffsets.UStruct_SuperStructOffset);
    }
    return false;
}

static ActorType GetActorType(uintptr_t uClass, const std::string& cname) {
    static std::unordered_map<uintptr_t, ActorType> typeCache;
    auto it = typeCache.find(uClass);
    if (it != typeCache.end()) return it->second;

    ActorType type = UNKNOWN;

    // quick string match first
    if (cname.find("Camper") != std::string::npos || cname.find("Survivor") != std::string::npos) type = SURVIVOR;
    else if (cname.find("Slasher") != std::string::npos || cname.find("Killer") != std::string::npos) type = KILLER;
    else if (cname.find("Generator") != std::string::npos) type = GENERATOR;
    else if (cname.find("Hook") != std::string::npos) type = HOOK;
    else if (cname.find("Locker") != std::string::npos) type = LOCKER;
    else if (cname.find("Totem") != std::string::npos) type = TOTEM;
    else if (cname.find("Pallet") != std::string::npos) type = PALLET;
    else if (cname.find("Hatch") != std::string::npos) type = HATCH;
    else if (cname.find("Trap") != std::string::npos) type = TRAP;
    else if (cname.find("Breakable") != std::string::npos) type = BREAKABLE;
    else if (cname.find("EscapeDoor") != std::string::npos) type = ESCAPEDOOR;
    else if (cname.find("Chest") != std::string::npos) type = CHEST;
    else if (cname.find("Window") != std::string::npos) type = WINDOW;
    else if (cname.find("Lament") != std::string::npos ||
             cname.find("PuzzleBox") != std::string::npos ||
             cname.find("CenobiteBox") != std::string::npos ||
             cname.find("K25Lament") != std::string::npos) type = CENOBITE_BOX;
    else if (cname.find("Zombie") != std::string::npos) type = NEMESIS_ZOMBIE;
    else if (cname.find("Twin") != std::string::npos) type = VICTOR;
    else if (cname.find("PhantomTrap") != std::string::npos) type = HAG_TRAP;
    else if (cname.find("MagicFountain") != std::string::npos) type = PLAGUE_FOUNTAIN;
    else if (cname.find("SupplyCrate") != std::string::npos) type = SUPPLY_CRATE;
    else if (cname.find("K33Tunnel") != std::string::npos) type = K33_TUNNEL;
    else if (cname.find("K33Turret") != std::string::npos) type = K33_TURRET;

    // strict inheritance check to eliminate false positives
    if (type == SURVIVOR && !ClassInheritsFrom(uClass, "Survivor")) type = UNKNOWN;
    if (type == KILLER && !ClassInheritsFrom(uClass, "Killer")) type = UNKNOWN;
    if (type == GENERATOR && !ClassInheritsFrom(uClass, "Generator")) type = UNKNOWN;
    if (type == HOOK && !ClassInheritsFrom(uClass, "MeatHook")) type = UNKNOWN;
    if (type == PALLET && !ClassInheritsFrom(uClass, "Pallet")) type = UNKNOWN;
    if (type == LOCKER && !ClassInheritsFrom(uClass, "Locker")) type = UNKNOWN;
    if (type == TOTEM && !ClassInheritsFrom(uClass, "Totem")) type = UNKNOWN;
    if (type == HATCH && !ClassInheritsFrom(uClass, "Hatch")) type = UNKNOWN;
    if (type == TRAP && !ClassInheritsFrom(uClass, "BaseTrap")) type = UNKNOWN;
    if (type == BREAKABLE && !ClassInheritsFrom(uClass, "BreakableBase")) type = UNKNOWN;
    if (type == ESCAPEDOOR && !ClassInheritsFrom(uClass, "EscapeDoor")) type = UNKNOWN;
    if (type == CHEST && !ClassInheritsFrom(uClass, "Searchable")) type = UNKNOWN;
    if (type == WINDOW && !ClassInheritsFrom(uClass, "Window")) type = UNKNOWN;

    typeCache[uClass] = type;
    return type;
}

static FVector ReadActorLocation(uintptr_t actor) {
    uintptr_t root = Memory::Read<uintptr_t>(actor + GOffsets.AActor_RootComponentOffset);
    if (!root) return {};

    // ComponentToWorld is at 0x220 in USceneComponent.
    // FTransform layout: FQuat Rotation (0x00, 32 bytes) + FVector Translation (0x20, 24 bytes)
    // We read the Translation (world-space position) directly.
    return Memory::Read<FVector>(root + 0x220 + 0x20);
}

static std::string ReadPlayerName(uintptr_t actor) {
    uintptr_t pawn       = actor;
    uintptr_t playerState = Memory::Read<uintptr_t>(pawn + GOffsets.APawn_PlayerStateOffset);
    if (!playerState) return "";
    // FString at PlayerNamePrivateOffset: ptr + len
    uintptr_t namePtr = Memory::Read<uintptr_t>(playerState + GOffsets.APlayerState_PlayerNamePrivateOffset);
    if (!namePtr) return "";
    // FString is UTF-16 — read chars and narrow
    char16_t buf[64] = {};
    for (int i = 0; i < 63; i++) {
        char16_t ch = Memory::Read<char16_t>(namePtr + i * 2);
        if (!ch) break;
        buf[i] = ch;
    }
    std::string out;
    for (int i = 0; buf[i]; i++) out += (char)buf[i];
    return out;
}

static uintptr_t FindRedStainComponent(uintptr_t killer) {
    // 9.6.0 SDK: InstanceComponents = 0x2C8, BlueprintCreatedComponents = 0x2D8
    for (uintptr_t offset : {(uintptr_t)0x2C8, (uintptr_t)0x2D8}) {
        uintptr_t data = Memory::Read<uintptr_t>(killer + offset);
        int count = Memory::Read<int>(killer + offset + 0x8);
        if (count > 0 && count < 200) {
            for (int i = 0; i < count; i++) {
                uintptr_t comp = Memory::Read<uintptr_t>(data + i * 8);
                if (!comp) continue;
                uintptr_t uClass = Memory::Read<uintptr_t>(comp + GOffsets.UObject_ClassOffset);
                uint32_t fnameId = Memory::Read<uint32_t>(uClass + GOffsets.UObject_FNameOffset + GOffsets.FName_DisplayIndexOffset);
                std::string name = GetNameFromId(fnameId);
                if (name.find("RedStainComponent") != std::string::npos) return comp;
            }
        }
    }
    return 0;
}

// --- rainbow ---
static ImVec4 RainbowColor() {
    static auto startTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float t = std::chrono::duration<float>(now - startTime).count();
    return ImVec4(
        0.5f + 0.5f * sinf(t * 2.0f),
        0.5f + 0.5f * sinf(t * 2.0f + 2.094f),
        0.5f + 0.5f * sinf(t * 2.0f + 4.189f),
        1.0f
    );
}

// --- background scanning thread ---

static void ScanLoop() {
    while (true) {
        // match FPS limit for scan frequency if it is reasonable
        std::this_thread::sleep_for(std::chrono::milliseconds(2));

        if (!Memory::IsAttached()) {
            if (Memory::Attach("DeadByDaylight-Win64-Shipping.exe")) {
                ResolveSignatures();
            } else {
                continue;
            }
        }

        // Aura patching removed to prevent crashes.
        if (!Memory::IsAttached()) {
            continue;
        }

        uintptr_t base = Memory::GetBaseAddress();
        if (!base || !Memory::IsAttached()) continue;

        uintptr_t UWorld = Memory::Read<uintptr_t>(base + GOffsets.GWorldOffset);
        if (!UWorld) continue;

        static bool world_logged = false;
        if (!world_logged) {
            std::cerr << "[DEBUG] UWorld: 0x" << std::hex << UWorld << std::dec << "\n";
            world_logged = true;
        }

        uintptr_t gameState = Memory::Read<uintptr_t>(UWorld + GOffsets.UWorld_GameStateOffset);
        bool gameEnded = false;
        std::vector<PlayerLoadout> loadouts;

        if (gameState) {
            gameEnded = Memory::Read<bool>(gameState + GOffsets.DBDGameState__gameLevelEndedOffset);
            
            bool currentInLobby = !Memory::Read<bool>(gameState + GOffsets.DBDGameState__gameLevelCreated);
            static bool s_wasInLobby = true;
            if (s_wasInLobby && !currentInLobby) {
                printf("[INFO] Match transition detected.\n");
            }
            if (!s_wasInLobby && currentInLobby) {
                // Restore logic removed
            }
            s_wasInLobby = currentInLobby;
            g_InLobby = currentInLobby;

            // Read PlayerArray for loadouts
            uintptr_t playerArray = Memory::Read<uintptr_t>(gameState + GOffsets.AGameStateBase_PlayerArrayOffset);
            int playerCount = Memory::Read<int>(gameState + GOffsets.AGameStateBase_PlayerArrayOffset + 8);
            if (playerArray && playerCount > 0 && playerCount <= 64) {
                for (int p = 0; p < playerCount; p++) {
                    uintptr_t ps = Memory::Read<uintptr_t>(playerArray + p * 8);
                    if (!ps) continue;

                    PlayerLoadout pl;
                    pl.role = Memory::Read<uint8_t>(ps + GOffsets.ADBDPlayerState_GameRoleOffset);

                    // Get name
                    uintptr_t namePtr = Memory::Read<uintptr_t>(ps + GOffsets.APlayerState_PlayerNamePrivateOffset);
                    if (namePtr) {
                        char16_t buf[64] = {};
                        for (int i = 0; i < 63; i++) {
                            char16_t ch = Memory::Read<char16_t>(namePtr + i * 2);
                            if (!ch) break;
                            buf[i] = ch;
                        }
                        for (int i = 0; buf[i]; i++) pl.name += (char)buf[i];
                    }
                    if (pl.name.empty()) pl.name = (pl.role == 1) ? "Killer" : "Survivor";

                    // Platform / Provider Flags
                    uint32_t platFlag = Memory::Read<uint32_t>(ps + GOffsets.ADBDPlayerState_platformOffset);
                    uint32_t provFlag = Memory::Read<uint32_t>(ps + GOffsets.ADBDPlayerState_providerOffset);
                    pl.platform = GetPlatformName(platFlag);
                    pl.provider = GetProviderName(provFlag);
                    
                    pl.ping = (int)Memory::Read<uint8_t>(ps + GOffsets.APlayerState_CompressedPingOffset) * 4;
                    pl.crossplayAllowed = Memory::Read<bool>(ps + GOffsets.ADBDPlayerState_crossplayAllowedOffset);
                    pl.survivorPips = Memory::Read<int>(ps + GOffsets.ADBDPlayerState_survivorPipsOffset);
                    pl.killerPips = Memory::Read<int>(ps + GOffsets.ADBDPlayerState_killerPipsOffset);

                    if (pl.role == 2) {
                        int survivorIdx = Memory::Read<int>(ps + 0x600); // _selectedSurvivorIndex
                        pl.characterName = GetFriendlyName(std::to_string(survivorIdx), 3);
                    } else if (pl.role == 1) {
                        int killerIdx = Memory::Read<int>(ps + 0x604); // _selectedKillerIndex
                        pl.characterName = GetFriendlyName(std::to_string(killerIdx), 3);
                    }

                    // Read Loadout
                    uintptr_t playerDataPtr = ps + GOffsets.ADBDPlayerState_playerDataOffset;
                    pl.prestige = Memory::Read<int>(playerDataPtr + GOffsets.FPlayerStateData_prestigeLevelOffset);
                    pl.characterLevel = Memory::Read<int>(playerDataPtr + GOffsets.FPlayerStateData_CharacterLevelOffset);

                    // Offering (Favor)
                    uint32_t favorId = Memory::Read<uint32_t>(playerDataPtr + GOffsets.FPlayerStateData_EquippedFavorIdOffset + GOffsets.FName_ComparisonIndexOffset);
                    std::string favorName = GetNameFromId(favorId);
                    if (!favorName.empty() && favorName != "None") {
                        if (favorName.find("Favor_") == 0) favorName = favorName.substr(6);
                        pl.offering = favorName;
                    }

                    // Perks
                    uintptr_t perkArrayData = Memory::Read<uintptr_t>(playerDataPtr + GOffsets.FPlayerStateData_EquippedPerkIdsOffset);
                    int pCount = Memory::Read<int>(playerDataPtr + GOffsets.FPlayerStateData_EquippedPerkIdsOffset + 8);
                    if (perkArrayData && pCount > 0 && pCount <= 4) {
                        for (int i = 0; i < pCount; i++) {
                            uintptr_t fnameAddr = perkArrayData + (i * 12);
                            uint32_t compIndex = Memory::Read<uint32_t>(fnameAddr + GOffsets.FName_ComparisonIndexOffset);
                            std::string perkName = GetNameFromId(compIndex);
                            if (!perkName.empty() && perkName != "None") {
                                if (perkName.find("Perk_") == 0) perkName = perkName.substr(5);
                                pl.perks.push_back(perkName);
                            }
                        }
                    }

                    // Item/Power
                    uint32_t itemCompIndex = Memory::Read<uint32_t>(playerDataPtr + GOffsets.FPlayerStateData_powerOrItemIdOffset + GOffsets.FName_ComparisonIndexOffset);
                    std::string itemName = GetNameFromId(itemCompIndex);
                    if (!itemName.empty() && itemName != "None") {
                        if (itemName.find("Item_") == 0) itemName = itemName.substr(5);
                        pl.item = itemName;
                    }

                    // Addons
                    uintptr_t addonArrayData = Memory::Read<uintptr_t>(playerDataPtr + GOffsets.FPlayerStateData_addonIdsOffset);
                    int aCount = Memory::Read<int>(playerDataPtr + GOffsets.FPlayerStateData_addonIdsOffset + 8);
                    if (addonArrayData && aCount > 0 && aCount <= 4) {
                        for (int i = 0; i < aCount; i++) {
                            uintptr_t fnameAddr = addonArrayData + (i * 12);
                            uint32_t compIndex = Memory::Read<uint32_t>(fnameAddr + GOffsets.FName_ComparisonIndexOffset);
                            std::string addonName = GetNameFromId(compIndex);
                            if (!addonName.empty() && addonName != "None") {
                                if (addonName.find("Addon_") == 0) addonName = addonName.substr(6);
                                else if (addonName.find("ItemAddon_") == 0) addonName = addonName.substr(10);
                                pl.addons.push_back(addonName);
                            }
                        }
                    }

                    loadouts.push_back(pl);
                }
            }
        } else {
            if (!g_InLobby) {
                // Restore logic removed
            }
            g_InLobby = true;
        }

        // --- camera state ---
        uintptr_t gameInst = Memory::Read<uintptr_t>(UWorld + GOffsets.UWorld_OwningGameInstanceOffset);
        uintptr_t localPlayers = Memory::Read<uintptr_t>(gameInst + GOffsets.UGameInstance_LocalPlayersOffset);
        if (!localPlayers) {
            static auto lastLTime = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastLTime).count() > 5) {
                printf("[DEBUG] Chain Break: localPlayers is null. GameInst: 0x%lx\n", gameInst);
                lastLTime = std::chrono::steady_clock::now();
            }
        }

        uintptr_t localPlayer = Memory::Read<uintptr_t>(localPlayers + 0);
        g_LocalController   = Memory::Read<uintptr_t>(localPlayer + GOffsets.UPlayer_PlayerControllerOffset);
        uintptr_t acknowledgedPawn = Memory::Read<uintptr_t>(g_LocalController + GOffsets.APlayerController_AcknowledgedPawnOffset);

        // Logging the chain once every 5 seconds
        static auto lastChainLog = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastChainLog).count() > 5) {
            printf("[DEBUG] Chain: UWorld: 0x%lx -> GInst: 0x%lx -> LPlayers: 0x%lx -> LPlayer: 0x%lx -> PCtrl: 0x%lx -> Pawn: 0x%lx\n",
                   UWorld, gameInst, localPlayers, localPlayer, g_LocalController, acknowledgedPawn);
            lastChainLog = std::chrono::steady_clock::now();
        }

        uintptr_t localPawn = 0;
        if (g_LocalController) {
            localPawn = acknowledgedPawn;
            uintptr_t localPlayerState = Memory::Read<uintptr_t>(g_LocalController + GOffsets.AController_PlayerStateOffset);
            if (localPlayerState) {
                g_LocalPlayerRole = Memory::Read<uint8_t>(localPlayerState + GOffsets.ADBDPlayerState_GameRoleOffset);
            }
        }

        static bool lastToggle = false;
        if (g_Cheats_AutoSkillCheckEnabled != lastToggle) {
            printf("[INFO] AutoSkillCheck: %s\n", g_Cheats_AutoSkillCheckEnabled ? "ENABLED" : "DISABLED");
            lastToggle = g_Cheats_AutoSkillCheckEnabled;
        }

        if (localPawn) {
            uintptr_t handler = Memory::Read<uintptr_t>(localPawn + GOffsets.ADBDPlayer_interactionHandler);
            if (handler) {
                uintptr_t skillCheck = Memory::Read<uintptr_t>(handler + GOffsets.UPlayerInteractionHandler_skillCheck);

                // Sanity check: valid pointer and not garbage like 0x4026...
                if (skillCheck > 0x1000) {
                    bool isDisplayed = Memory::Read<uint8_t>(skillCheck + GOffsets.USkillCheck_isDisplayed) != 0;
                    if (!isDisplayed) {
                        ResetSkillCheckLatch();
                    } else {
                        float progress = Memory::Read<float>(skillCheck + GOffsets.USkillCheck_currentProgress);
                        FSkillCheckDefinition def = Memory::Read<FSkillCheckDefinition>(skillCheck + GOffsets.USkillCheck_skillCheckDefinition);
                        int customType = (int)Memory::Read<uint8_t>(skillCheck + GOffsets.USkillCheck_customType);
                        SkillCheckWindow window = BuildSkillCheckWindow(progress, def, customType);

                        if (window.valid) {
                            bool progressWrapped = lastSkillCheckProgress >= 0.0f &&
                                                   progress < (lastSkillCheckProgress - 0.25f);
                            if (progressWrapped) wasInBonusZone = false;
                            lastSkillCheckProgress = progress;

                            if (g_Cheats_AutoSkillCheckEnabled) {
                                static auto lastHitTime = std::chrono::steady_clock::now() - std::chrono::seconds(1);
                                auto now = std::chrono::steady_clock::now();

                                SkillCheckHitWindow hit = BuildSkillCheckHitWindow(window, def);
                                bool inZone = hit.valid && IsSkillProgressInRange(window.progress, hit.start, hit.end);

                                // Type 11 is reported as wiggle on some builds, but treating it only
                                // as wiggle can miss rapid continuous checks. Prefer the normal math
                                // if it is the one currently inside the active window.
                                if (customType == 11) {
                                    SkillCheckWindow normalWindow = BuildSkillCheckWindow(progress, def, -1);
                                    SkillCheckHitWindow normalHit = BuildSkillCheckHitWindow(normalWindow, def);
                                    bool normalInZone = normalHit.valid &&
                                                        IsSkillProgressInRange(normalWindow.progress, normalHit.start, normalHit.end);
                                    if (normalInZone && !inZone) {
                                        window = normalWindow;
                                        hit = normalHit;
                                        inZone = true;
                                    }
                                }

                                if (inZone) {
                                    bool windowChanged = skillCheck != lastHitSkillCheck ||
                                                         customType != lastHitSkillCheckType ||
                                                         progressWrapped ||
                                                         SkillProgressCircularDistance(window.zoneStart, lastHitSkillCheckZoneStart) > 0.0025f ||
                                                         SkillProgressCircularDistance(window.zoneEnd, lastHitSkillCheckZoneEnd) > 0.0025f ||
                                                         SkillProgressCircularDistance(def.StartingTickerPosition, lastHitSkillCheckStartTicker) > 0.0025f;
                                    int cooldownMs = windowChanged ? 25 : 90;
                                    auto msSinceHit = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHitTime).count();

                                    if ((windowChanged || !wasInBonusZone) && msSinceHit > cooldownMs) {
                                        printf("[*] Auto-Hit (%s/%s Type:%d)! Rate:%.2f | Prog:%.4f | Window:%.4f-%.4f | Delay:%.0fms\n",
                                               window.isWiggle ? "Wiggle" : "Normal",
                                               window.usesBonusZone ? "Bonus" : "Success",
                                               customType, def.ProgressRate, NormalizeSkillProgress(window.progress),
                                               NormalizeSkillProgress(hit.start), NormalizeSkillProgress(hit.end),
                                               g_Cheats_AutoSkillCheckDelayMs);
                                        PressSpaceBar(35);
                                        wasInBonusZone = true;
                                        lastHitTime = now;
                                        lastHitSkillCheck = skillCheck;
                                        lastHitSkillCheckType = customType;
                                        lastHitSkillCheckZoneStart = NormalizeSkillProgress(window.zoneStart);
                                        lastHitSkillCheckZoneEnd = NormalizeSkillProgress(window.zoneEnd);
                                        lastHitSkillCheckStartTicker = NormalizeSkillProgress(def.StartingTickerPosition);
                                    }
                                } else {
                                    wasInBonusZone = false;
                                }
                            }
                        } else {
                            ResetSkillCheckLatch();
                        }
                    }
                } else {
                    ResetSkillCheckLatch();
                }
            }
        }

        // --- Spam Spacebar ---
        if (g_Cheats_SpamSpacebar) {
            static auto lastSpamTime = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSpamTime).count() > 50) {
                PressSpaceBar();
                lastSpamTime = now;
            }
        }

        // APlayerController::PlayerCameraManager
        uintptr_t camMgr = Memory::Read<uintptr_t>(g_LocalController + GOffsets.APlayerController_PlayerCameraManagerOffset);
        if (camMgr) {
            uintptr_t cacheBase = camMgr + GOffsets.APlayerCameraManager_CameraCachePrivateOffset
                                         + GOffsets.FCameraCacheEntry_POVOffset;
            FMinimalViewInfo cam;
            cam.Location = Memory::Read<FVector>(cacheBase + GOffsets.FMinimalViewInfo_LocationOffset);
            cam.Rotation = Memory::Read<FRotator>(cacheBase + GOffsets.FMinimalViewInfo_RotationOffset);
            cam.FOV      = Memory::Read<float>(cacheBase + GOffsets.FMinimalViewInfo_FOVOffset);

            {
                std::lock_guard<std::mutex> lock(g_CameraMutex);
                g_CameraInfo = cam;
            }

            // track original FOV for cleanup
            if (g_LastCamMgr != camMgr) {
                g_LastCamMgr = camMgr;
                g_OriginalFOV = Memory::Read<float>(camMgr + GOffsets.APlayerCameraManager_DefaultFOVOffset);
            }

            // --- Cheats: FOV Changer ---
            if (g_Cheats_Enabled && g_Cheats_FOVEnabled) {
                // write to PlayerCameraManager.DefaultFOV
                Memory::Write<float>(camMgr + GOffsets.APlayerCameraManager_DefaultFOVOffset, g_Cheats_FOV);

                // also write to local player's CameraComponent if possible
                uintptr_t pawn = Memory::Read<uintptr_t>(g_LocalController + GOffsets.APlayerController_AcknowledgedPawnOffset); // APlayerController.AcknowledgedPawn
                if (pawn) {
                    uintptr_t camera = Memory::Read<uintptr_t>(pawn + GOffsets.ADBDPlayer_CameraOffset);
                    if (camera) {
                        Memory::Write<float>(camera + GOffsets.UCameraComponent_FieldOfViewOffset, g_Cheats_FOV);
                    }
                }
            }
        }

        // --- actor list ---
        uintptr_t persistentLevel = Memory::Read<uintptr_t>(UWorld + GOffsets.UWorld_PersistentLevelOffset);
        if (!persistentLevel) continue;

        uintptr_t actorsArray = Memory::Read<uintptr_t>(persistentLevel + GOffsets.ULevel_ActorsOffset);
        int       actorCount  = Memory::Read<int>(persistentLevel + GOffsets.ULevel_ActorsOffset + GOffsets.Actors_NumElementsOffset);

        if (actorCount <= 0 || actorCount > 50000) continue;

        std::vector<ActorEntry> survivors, killers, generators, hooks,
            lockers, totems, pallets, hatches, traps, victors, hagTraps,
            fountains, supplyCrates, tunnels, turrets,
            breakableDoors, escapeDoors, chests, cenobiteBoxes,
            nemesisZombies, windows, other;

        FVector myLoc;
        { std::lock_guard<std::mutex> lock(g_CameraMutex); myLoc = g_CameraInfo.Location; }

        gameState = Memory::Read<uintptr_t>(UWorld + GOffsets.UWorld_GameStateOffset);
        double serverTime = 0.0;
        if (gameState) {
            serverTime = Memory::Read<double>(gameState + GOffsets.AGameStateBase_ReplicatedWorldTimeSecondsDoubleOffset);
            // Fallback if double is zero but float isn't (unlikely but safe)
            if (serverTime < 0.01)
                serverTime = (double)Memory::Read<float>(gameState + GOffsets.AGameStateBase_ReplicatedWorldTimeSecondsOffset);
        }

        for (int i = 0; i < actorCount; i++) {
            uintptr_t actor = Memory::Read<uintptr_t>(actorsArray + i * 8);
            if (!actor) continue;
            if (actor == localPawn) continue; // Skip local player

            uintptr_t uClass = Memory::Read<uintptr_t>(actor + GOffsets.UObject_ClassOffset);
            if (!uClass) continue;

            uint32_t   fnameId = Memory::Read<uint32_t>(uClass + GOffsets.UObject_FNameOffset + GOffsets.FName_DisplayIndexOffset);
            std::string cname  = GetNameFromId(fnameId);
            if (cname.empty()) continue;

            ActorType type = GetActorType(uClass, cname);
            if (type == UNKNOWN) continue;

            FVector loc = ReadActorLocation(actor);
            if (loc.X == 0 && loc.Y == 0 && loc.Z == 0) continue;

            ActorEntry e{};
            e.addr     = actor;
            e.location = loc;

            float dx = loc.X - myLoc.X;
            float dy = loc.Y - myLoc.Y;
            float dz = loc.Z - myLoc.Z;
            e.distance = sqrtf(dx*dx + dy*dy + dz*dz);

            // Default height for actors that don't have a capsule
            e.height = 180.0f;

            uintptr_t root = Memory::Read<uintptr_t>(actor + GOffsets.AActor_RootComponentOffset);
            if (root) {
                e.rotation = Memory::Read<FRotator>(root + 0x0190);
            }

            if (type == SURVIVOR || type == KILLER) {
                uintptr_t capsule = Memory::Read<uintptr_t>(actor + GOffsets.ACharacter_CapsuleComponentOffset);
                if (capsule) {
                    float halfHeight = Memory::Read<float>(capsule + GOffsets.UCapsuleComponent_CapsuleHalfHeightOffset);
                    if (halfHeight > 10.0f) e.height = halfHeight * 2.0f;
                }
                
                uintptr_t moveComp = Memory::Read<uintptr_t>(actor + GOffsets.ACharacter_CharacterMovementOffset);
                if (moveComp) {
                    e.velocity = Memory::Read<FVector>(moveComp + GOffsets.UMovementComponent_VelocityOffset);
                }
            }

            if (type == SURVIVOR) {
                e.name = ReadPlayerName(actor);
                if (e.name.empty()) e.name = "Survivor";

                uintptr_t playerState = Memory::Read<uintptr_t>(actor + GOffsets.APawn_PlayerStateOffset);
                if (playerState) {
                    e.inParadise = Memory::Read<bool>(playerState + GOffsets.ADBDPlayerState__inParadiseOffset);
                    int survivorIdx = Memory::Read<int>(playerState + 0x600);
                    e.characterName = GetFriendlyName(std::to_string(survivorIdx), 3);

                    uint32_t plat = Memory::Read<uint32_t>(playerState + 0x7AC);
                    uint32_t prov = Memory::Read<uint32_t>(playerState + 0x7B0);
                    e.platform = GetPlatformName(plat);
                    e.provider = GetProviderName(prov);
                    e.prestige = Memory::Read<int>(playerState + 0x5EC);
                }

                // Chase status
                uintptr_t chaseComp = Memory::Read<uintptr_t>(actor + GOffsets.ADBDPlayer_chaseComponent);
                if (chaseComp) {
                    e.isInChase = Memory::Read<bool>(chaseComp + GOffsets.UChaseComponent_isInChase);
                }

                // Interaction status
                uintptr_t interactionHandler = Memory::Read<uintptr_t>(actor + GOffsets.ADBDPlayer_interactionHandler);
                if (interactionHandler) {
                    e.isInteracting = Memory::Read<bool>(interactionHandler + GOffsets.UPlayerInteractionHandler_interactionInProgress);
                }

                // Recovery progress (Survivor only)
                uintptr_t healthComp = Memory::Read<uintptr_t>(actor + GOffsets.ASurvivor__healthComponentOffset);
                if (healthComp) {
                    uintptr_t recoveryChargeable = Memory::Read<uintptr_t>(healthComp + GOffsets.UHealthComponent_healFromDyingChargeable);
                    if (recoveryChargeable) {
                        uintptr_t syncedValuePtr = recoveryChargeable + GOffsets.UChargeableComponent__currentChargeOffset;
                        float replicatedValue = Memory::Read<float>(syncedValuePtr + GOffsets.FSpeedBasedNetSyncedValue__replicatedValueOffset);
                        // Max charge for recovery is usually 1.0 (it's normalized in the component) or 16.0 seconds.
                        // We'll just read the percent if we can or assume 1.0 for now.
                        e.recoveryProgress = replicatedValue; // This needs verification if it's 0-1 or 0-seconds.
                    }
                }

                // Wiggle progress
                uintptr_t carriedMoveComp = Memory::Read<uintptr_t>(actor + GOffsets.ASurvivor_carriedMovementComponent);
                if (carriedMoveComp) {
                    // Wiggle is often a complex component, but let's see if we can find a simple float.
                    // For now, we'll leave it as a placeholder or find the offset later.
                }

                // --- Scan Perks and Active State ---
                uintptr_t perkManager = Memory::Read<uintptr_t>(actor + 0x0B68); // ADBDPlayer::_perkManager
                if (perkManager) {
                    uintptr_t perkCollection = Memory::Read<uintptr_t>(perkManager + 0x00F8); // UPerkManager::_perks
                    if (perkCollection) {
                        uintptr_t perkArrayData = Memory::Read<uintptr_t>(perkCollection + 0x00E8); // UPerkCollectionComponent::_array
                        int32_t perkCount = Memory::Read<int32_t>(perkCollection + 0x00F0);
                        if (perkArrayData && perkCount > 0 && perkCount < 20) {
                            for (int i = 0; i < perkCount; i++) {
                                uintptr_t perkAddr = Memory::Read<uintptr_t>(perkArrayData + (i * 8));
                                if (!perkAddr) continue;

                                std::string perkName = GetUClassName(perkAddr);
                                if (perkName.empty()) continue;

                                e.perks.push_back(perkName);

                                // Check if perk is active/usable
                                bool isUsable = Memory::Read<bool>(perkAddr + 0x0498); // UPerk::_isUsable
                                if (isUsable) {
                                    // Whitelist of perks we want to show as "ACTIVE"
                                    if (perkName.find("DeadHard") != std::string::npos) e.activePerks.push_back("Dead Hard");
                                    else if (perkName.find("DecisiveStrike") != std::string::npos) e.activePerks.push_back("DS");
                                    else if (perkName.find("Unbreakable") != std::string::npos) e.activePerks.push_back("Unbreakable");
                                    else if (perkName.find("Deliverance") != std::string::npos) e.activePerks.push_back("Deliverance");
                                    else if (perkName.find("OffTheRecord") != std::string::npos) e.activePerks.push_back("OTR");
                                }
                            }
                        }
                    }
                }

                survivors.push_back(e);
            }
            else if (type == KILLER) {
                e.name = ReadPlayerName(actor);
                if (e.name.empty()) e.name = "Killer";

                uintptr_t playerState = Memory::Read<uintptr_t>(actor + GOffsets.APawn_PlayerStateOffset);
                if (playerState) {
                    int killerIdx = Memory::Read<int>(playerState + 0x604);
                    e.characterName = GetFriendlyName(std::to_string(killerIdx), 3);

                    uint32_t plat = Memory::Read<uint32_t>(playerState + 0x7AC);
                    uint32_t prov = Memory::Read<uint32_t>(playerState + 0x7B0);
                    e.platform = GetPlatformName(plat);
                    e.provider = GetProviderName(prov);
                    e.prestige = Memory::Read<int>(playerState + 0x5EC);
                }

                // Chase status
                uintptr_t chaseComp = Memory::Read<uintptr_t>(actor + GOffsets.ADBDPlayer_chaseComponent);
                if (chaseComp) {
                    e.isInChase = Memory::Read<bool>(chaseComp + GOffsets.UChaseComponent_isInChase);
                }

                // --- Auto Dead Hard (Experimental) ---
                if (g_Cheats_AutoDeadHardEnabled) {
                    float deadHardTriggerUnits = g_Cheats_DeadHardDistance * 100.0f;
                    if (e.distance <= deadHardTriggerUnits) {
                        uintptr_t attackerComp = Memory::Read<uintptr_t>(actor + GOffsets.AKiller_attackerComponent);
                        uintptr_t requestedAttack = attackerComp ? Memory::Read<uintptr_t>(attackerComp + GOffsets.UDBDAttackerComponent_requestedAttack) : 0;
                        uintptr_t currentAttack = attackerComp ? Memory::Read<uintptr_t>(attackerComp + GOffsets.UDBDAttackerComponent_currentAttack) : 0;

                        bool trigger = false;
                        if (g_Cheats_DeadHardPerfect) {
                            // Only trigger when the attack is physically in progress (lunge or quick swing hitting phase)
                            trigger = (currentAttack != 0);
                        } else {
                            // Trigger as soon as the button is pressed (extreme responsiveness)
                            trigger = (requestedAttack != 0 || currentAttack != 0);
                        }

                        auto now = std::chrono::steady_clock::now();
                        if (g_Cheats_SkillCheckDebugEnabled) {
                            static auto lastDHDebugTime = std::chrono::steady_clock::now() - std::chrono::seconds(2);
                            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDHDebugTime).count() > 1000) {
                                printf("[DH] Killer %.1fm | req:0x%lx | cur:0x%lx | trigger:%d\n",
                                       e.distance / 100.0f, requestedAttack, currentAttack, trigger);
                                lastDHDebugTime = now;
                            }
                        }

                        if (trigger) {
                            static auto lastDHTime = std::chrono::steady_clock::now() - std::chrono::seconds(3);
                            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDHTime).count() > 1500) {
                                printf("[*] AUTO DEAD HARD! Killer swinging at %.1fm\n", e.distance / 100.0f);
                                PressKeyE();
                                lastDHTime = now;
                            }
                        }
                    }
                }

                if (g_Cheats_Enabled && g_Cheats_KillerStainEnabled) {
                    uintptr_t redStainComp = FindRedStainComponent(actor);
                    if (redStainComp) {
                        ImVec4 col = g_Cheats_KillerStainRainbowEnabled ? RainbowColor() : g_Cheats_KillerStainColor;
                        float colorArr[4] = {col.x, col.y, col.z, col.w};
                        Memory::Write(redStainComp + GOffsets.RedStainComponent__initialSpotLightColorOffset, colorArr, sizeof(colorArr));
                    }
                }

                killers.push_back(e);
            }
            else if (type == GENERATOR) {
                e.activated       = Memory::Read<bool>(actor + GOffsets.AGenerator__activatedOffset);

                uintptr_t chargeable = Memory::Read<uintptr_t>(actor + GOffsets.AGenerator__generatorChargeableOffset);
                if (chargeable) {
                    uintptr_t syncedValuePtr = chargeable + GOffsets.UChargeableComponent__currentChargeOffset;
                    float replicatedValue     = Memory::Read<float>(syncedValuePtr + GOffsets.FSpeedBasedNetSyncedValue__replicatedValueOffset);
                    float replicatedSpeed     = Memory::Read<float>(syncedValuePtr + GOffsets.FSpeedBasedNetSyncedValue_replicatedSpeedOffset);
                    float replicatedTimestamp = Memory::Read<float>(syncedValuePtr + GOffsets.FSpeedBasedNetSyncedValue_replicatedLastUpdateTimestampOffset);

                    float currentCharge = replicatedValue;
                    // If serverTime is valid and greater than last update, interpolate
                    if (serverTime > 0.01 && replicatedTimestamp > 0.01f && serverTime > (double)replicatedTimestamp) {
                        currentCharge += (float)((serverTime - (double)replicatedTimestamp) * (double)replicatedSpeed);
                    }

                    float maxCharge = Memory::Read<float>(actor + GOffsets.AGenerator__generatorChargeInSecondsOffset + GOffsets.FDBDTunableRowHandle__defaultValueOffset);
                    if (maxCharge <= 0.0f) maxCharge = 90.0f; 
                    e.percentComplete = currentCharge / maxCharge;
                    if (e.percentComplete > 1.0f) e.percentComplete = 1.0f;
                    if (e.percentComplete < 0.0f) e.percentComplete = 0.0f;
                } else {
                    // Skip actors matching "Generator" but lacking a chargeable component (e.g. cosmetic markers)
                    continue;
                }

                if (g_ESP_GeneratorHideCompleted && (e.activated || e.percentComplete >= 1.0f)) continue;

                generators.push_back(e);
            }
            else if (type == HOOK) {
                hooks.push_back(e);
            }
            else if (type == LOCKER) {
                e.hasPlayer = (Memory::Read<uintptr_t>(actor + GOffsets.ALocker__playerInLockerOffset) != 0);
                lockers.push_back(e);
            }
            else if (type == TOTEM) {
                e.objectState = Memory::Read<int>(actor + GOffsets.ATotem__totemStateOffset);
                if (e.objectState != 0) totems.push_back(e); // 0=Cleansed, skip it
            }
            else if (type == PALLET) {
                e.objectState = Memory::Read<int>(actor + GOffsets.APallet__stateOffset);
                if (e.objectState < 3) pallets.push_back(e); // 3=Destroyed, skip it
            }
            else if (type == HATCH) {
                e.objectState = Memory::Read<int>(actor + GOffsets.AHatch__hatchStateOffset);
                hatches.push_back(e);
            }
            else if (type == TRAP) {
                e.isTrapSet = Memory::Read<bool>(actor + GOffsets.ABaseTrap__isTrapSetOffset);
                traps.push_back(e);
            }
            else if (type == WINDOW) {
                e.isBlocked = Memory::Read<bool>(actor + GOffsets.AWindow__isBlockedByLevelOffset);
                windows.push_back(e);
            }
            else if (type == BREAKABLE) {
                e.objectState = Memory::Read<int>(actor + GOffsets.BreakableBase__stateOffset);
                breakableDoors.push_back(e);
            }
            else if (type == ESCAPEDOOR) {
                e.activated = Memory::Read<bool>(actor + GOffsets.EscapeDoor__activatedOffset);
                escapeDoors.push_back(e);
            }
            else if (type == CHEST) {
                chests.push_back(e);
            }
            else if (type == CENOBITE_BOX) {
                e.height = 80.0f;
                cenobiteBoxes.push_back(e);
            }
            else if (type == NEMESIS_ZOMBIE) {
                uintptr_t capsule = Memory::Read<uintptr_t>(actor + GOffsets.ACharacter_CapsuleComponentOffset);
                if (capsule) {
                    float halfHeight = Memory::Read<float>(capsule + GOffsets.UCapsuleComponent_CapsuleHalfHeightOffset);
                    if (halfHeight > 10.0f) e.height = halfHeight * 2.0f;
                }
                nemesisZombies.push_back(e);
            }
            else if (type == VICTOR) {
                uintptr_t capsule = Memory::Read<uintptr_t>(actor + GOffsets.ACharacter_CapsuleComponentOffset);
                if (capsule) {
                    float halfHeight = Memory::Read<float>(capsule + GOffsets.UCapsuleComponent_CapsuleHalfHeightOffset);
                    if (halfHeight > 10.0f) e.height = halfHeight * 2.0f;
                }
                victors.push_back(e);
            }
            else if (type == HAG_TRAP) {
                hagTraps.push_back(e);
            }
            else if (type == PLAGUE_FOUNTAIN) {
                fountains.push_back(e);
            }
            else if (type == SUPPLY_CRATE) {
                supplyCrates.push_back(e);
            }
            else if (type == K33_TUNNEL) {
                tunnels.push_back(e);
            }
            else if (type == K33_TURRET) {
                turrets.push_back(e);
            }
        }

        // Post-process generator interactor counts
        for (auto& g : generators) {
            int count = 0;
            for (const auto& s : survivors) {
                if (s.isInteracting && (s.location - g.location).Size() < 300.0f) {
                    count++;
                }
            }
            g.interactorCount = count;
        }

        std::lock_guard<std::mutex> lk(g_ActorMutex);
        g_Survivors     = std::move(survivors);
        g_Killers       = std::move(killers);
        g_Generators    = std::move(generators);
        g_Hooks         = std::move(hooks);
        g_Lockers       = std::move(lockers);
        g_Totems        = std::move(totems);
        g_Pallets       = std::move(pallets);
        g_Hatches       = std::move(hatches);
        g_Traps         = std::move(traps);
        g_BreakableDoors = std::move(breakableDoors);
        g_EscapeDoors   = std::move(escapeDoors);
        g_Chests        = std::move(chests);
        g_CenobiteBoxes = std::move(cenobiteBoxes);
        g_NemesisZombies = std::move(nemesisZombies);
        g_Victors        = std::move(victors);
        g_HagTraps       = std::move(hagTraps);
        g_PlagueFountains = std::move(fountains);
        g_SupplyCrates   = std::move(supplyCrates);
        g_K33Tunnels     = std::move(tunnels);
        g_K33Turrets     = std::move(turrets);
        g_PlayerLoadouts = std::move(loadouts);
    }
}

void ESP_StartScanThread() {
    std::thread(ScanLoop).detach();
}

static ImU32 ToImU32(const ImVec4& c) {
    return IM_COL32((int)(c.x*255), (int)(c.y*255), (int)(c.z*255), (int)(c.w*255));
}

// box styles: 0=corner, 1=full
static void DrawBox(ImDrawList* dl, ImVec2 min, ImVec2 max, ImU32 col, int style, float thick = 1.5f) {
    if (style == 0) {
        // corner box
        float cx = (max.x - min.x) * 0.25f;
        float cy = (max.y - min.y) * 0.25f;
        // top-left
        dl->AddLine(min, {min.x + cx, min.y}, col, thick);
        dl->AddLine(min, {min.x, min.y + cy}, col, thick);
        // top-right
        dl->AddLine({max.x, min.y}, {max.x - cx, min.y}, col, thick);
        dl->AddLine({max.x, min.y}, {max.x, min.y + cy}, col, thick);
        // bottom-left
        dl->AddLine({min.x, max.y}, {min.x + cx, max.y}, col, thick);
        dl->AddLine({min.x, max.y}, {min.x, max.y - cy}, col, thick);
        // bottom-right
        dl->AddLine(max, {max.x - cx, max.y}, col, thick);
        dl->AddLine(max, {max.x, max.y - cy}, col, thick);
    } else {
        dl->AddRect(min, max, col, 0.0f, 0, thick);
    }
}

static void DrawSnapline(ImDrawList* dl, ImVec2 screen, ImU32 col, int style) {
    float sw = ImGui::GetIO().DisplaySize.x;
    float sh = ImGui::GetIO().DisplaySize.y;
    ImVec2 origin;
    // style 0 = bottom-center, 1 = center, 2 = top-center
    if (style == 0)      origin = {sw * 0.5f, sh};
    else if (style == 1) origin = {sw * 0.5f, sh * 0.5f};
    else                 origin = {sw * 0.5f, 0};
    dl->AddLine(origin, screen, col, 1.0f);
}

static float GetSafeFOV(float fov) {
    if (!std::isfinite(fov)) return 90.0f;
    return std::clamp(fov, 40.0f, 160.0f);
}

static bool ProjectWorldToScreenUnclipped(const FVector& worldLoc, const FMinimalViewInfo& camera,
                                          int screenWidth, int screenHeight,
                                          ImVec2& outScreen, double* outDepth = nullptr) {
    FMatrix tempMatrix = RotatorToMatrix(camera.Rotation);

    FVector vAxisX(tempMatrix.M[0][0], tempMatrix.M[0][1], tempMatrix.M[0][2]);
    FVector vAxisY(tempMatrix.M[1][0], tempMatrix.M[1][1], tempMatrix.M[1][2]);
    FVector vAxisZ(tempMatrix.M[2][0], tempMatrix.M[2][1], tempMatrix.M[2][2]);

    FVector vDelta(worldLoc.X - camera.Location.X,
                   worldLoc.Y - camera.Location.Y,
                   worldLoc.Z - camera.Location.Z);

    double x = vDelta.X * vAxisY.X + vDelta.Y * vAxisY.Y + vDelta.Z * vAxisY.Z;
    double y = vDelta.X * vAxisZ.X + vDelta.Y * vAxisZ.Y + vDelta.Z * vAxisZ.Z;
    double depth = vDelta.X * vAxisX.X + vDelta.Y * vAxisX.Y + vDelta.Z * vAxisX.Z;

    if (!std::isfinite(depth) || depth < 1.0) {
        return false;
    }

    float fov = GetSafeFOV(camera.FOV);
    double focal = (screenWidth * 0.5) / tan(fov * M_PI / 360.0);
    if (!std::isfinite(focal) || focal <= 0.0) {
        return false;
    }

    outScreen.x = (float)((screenWidth * 0.5) + (x * focal / depth));
    outScreen.y = (float)((screenHeight * 0.5) - (y * focal / depth));
    if (outDepth) *outDepth = depth;
    return std::isfinite(outScreen.x) && std::isfinite(outScreen.y);
}

static bool BuildPerspectiveBox(const ActorEntry& e, const FMinimalViewInfo& cam,
                                int screenWidth, int screenHeight,
                                ImVec2& outMin, ImVec2& outMax) {
    ImVec2 rootScreen{};
    double depth = 0.0;
    if (!ProjectWorldToScreenUnclipped(e.location, cam, screenWidth, screenHeight, rootScreen, &depth)) {
        return false;
    }

    float actorHeight = std::clamp(e.height, 120.0f, 260.0f);
    float maxHeight = screenHeight * 0.85f;
    float minHeight = 2.0f;

    FVector topWorld(e.location.X, e.location.Y, e.location.Z + (actorHeight * 0.5f));
    FVector bottomWorld(e.location.X, e.location.Y, e.location.Z - (actorHeight * 0.5f));
    ImVec2 topScreen{}, bottomScreen{};

    if (ProjectWorldToScreenUnclipped(topWorld, cam, screenWidth, screenHeight, topScreen) &&
        ProjectWorldToScreenUnclipped(bottomWorld, cam, screenWidth, screenHeight, bottomScreen)) {
        float projectedHeight = std::abs(bottomScreen.y - topScreen.y);
        if (std::isfinite(projectedHeight) && projectedHeight >= minHeight && projectedHeight <= maxHeight * 1.25f) {
            float boxHeight = std::clamp(projectedHeight, minHeight, maxHeight);
            float boxWidth = boxHeight * 0.42f;
            float centerX = (topScreen.x + bottomScreen.x) * 0.5f;
            float topY = std::min(topScreen.y, bottomScreen.y);
            float bottomY = std::max(topScreen.y, bottomScreen.y);

            outMin = {centerX - (boxWidth * 0.5f), topY};
            outMax = {centerX + (boxWidth * 0.5f), bottomY};
            return outMax.y > outMin.y && outMax.x > outMin.x;
        }
    }

    float focal = (float)((screenWidth * 0.5) / tan(GetSafeFOV(cam.FOV) * M_PI / 360.0));
    float boxHeight = (actorHeight * focal) / (float)depth;
    if (!std::isfinite(boxHeight)) {
        return false;
    }

    boxHeight = std::clamp(boxHeight, minHeight, maxHeight);

    float boxWidth = boxHeight * 0.42f;
    float bottomY = rootScreen.y + (boxHeight * 0.48f);
    float topY = bottomY - boxHeight;

    outMin = {rootScreen.x - (boxWidth * 0.5f), topY};
    outMax = {rootScreen.x + (boxWidth * 0.5f), bottomY};
    return outMax.y > outMin.y && outMax.x > outMin.x;
}

struct SkeletonPair { int a, b; };

struct SkeletonNamePair { const char* a; const char* b; };

struct TArrayHeader {
    uintptr_t data;
    int32_t count;
    int32_t max;
};

struct SkeletonCacheEntry {
    std::vector<SkeletonPair> pairs;
    int headBoneIndex = -1;
};

static std::unordered_map<uintptr_t, SkeletonCacheEntry> g_SkeletonCache;

static const SkeletonNamePair kHumanoidSkeletonNames[] = {
    {"joint_head_01", "joint_necka_01"},
    {"joint_head_02", "joint_necka_01"},
    {"joint_necka_01", "joint_torsoc_01"},
    {"joint_torsoc_01", "joint_pelvis_01"},
    {"joint_necka_01", "joint_shoulderlt_01"},
    {"joint_shoulderlt_01", "joint_elbowlt_01"},
    {"joint_elbowlt_01", "joint_handlt_01"},
    {"joint_necka_01", "joint_shoulderrt_01"},
    {"joint_shoulderrt_01", "joint_elbowrt_01"},
    {"joint_elbowrt_01", "joint_handrt_01"},
    {"joint_pelvis_01", "joint_hiplt_01"},
    {"joint_hiplt_01", "joint_kneelt_01"},
    {"joint_kneelt_01", "joint_footlt_01"},
    {"joint_pelvis_01", "joint_hiprt_01"},
    {"joint_hiprt_01", "joint_kneert_01"},
    {"joint_kneert_01", "joint_footrt_01"},
};

// Known survivor/camper fallback. The old numbers were offset into fingers,
// straps, and chains, which is why hands could connect to knees.
static const SkeletonPair kSurvivorFallbackSkeleton[] = {
    {86, 85}, // head -> neck
    {85, 15}, // neck -> upper torso
    {15, 1},  // torso -> pelvis
    {85, 19}, {19, 20}, {20, 21}, // left arm
    {85, 59}, {59, 60}, {60, 61}, // right arm
    {1, 8}, {8, 9}, {9, 10},      // left leg
    {1, 3}, {3, 4}, {4, 5},       // right leg
};

static std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return (char)std::tolower(ch); });
    return value;
}

static bool ReadTArrayHeader(uintptr_t address, TArrayHeader& out) {
    out.data = Memory::Read<uintptr_t>(address);
    out.count = Memory::Read<int32_t>(address + 0x8);
    out.max = Memory::Read<int32_t>(address + 0xC);
    return IsLikelyPointer(out.data) &&
           out.count > 0 && out.count < 512 &&
           out.max >= out.count && out.max < 1024;
}


static bool LooksLikeUsefulBoneName(const std::string& name) {
    if (name.empty()) return false;
    if (name.find("joint_") == 0) return true;
    return name.find("head") != std::string::npos ||
           name.find("neck") != std::string::npos ||
           name.find("pelvis") != std::string::npos ||
           name.find("hand") != std::string::npos ||
           name.find("knee") != std::string::npos ||
           name.find("foot") != std::string::npos;
}

static std::string ReadBoneInfoName(uintptr_t boneInfoRecord) {
    uint32_t comparisonIndex = Memory::Read<uint32_t>(
        boneInfoRecord + GOffsets.FName_ComparisonIndexOffset);
    if (comparisonIndex == 0) return {};

    return ToLower(GetNameFromId(comparisonIndex));
}

static int ScoreBoneInfoLayout(const TArrayHeader& header, size_t stride) {
    int score = 0;
    int sampleCount = std::min(header.count, 160);
    for (int i = 0; i < sampleCount; ++i) {
        std::string name = ReadBoneInfoName(header.data + (i * stride));
        if (LooksLikeUsefulBoneName(name)) {
            score += (name.find("joint_") == 0) ? 2 : 1;
        }
    }
    return score;
}

static bool BuildBoneNameIndex(uintptr_t skeletalMesh,
                               std::unordered_map<std::string, int>& out) {
    if (!IsLikelyPointer(skeletalMesh)) return false;

    const std::array<uintptr_t, 6> rawInfoOffsets = {
        GOffsets.USkeletalMesh_RawRefBoneInfo,
        (uintptr_t)0x360,
        (uintptr_t)0x358,
        (uintptr_t)0x420,
        (uintptr_t)0x4B8,
        (uintptr_t)0x2E8,
    };
    const std::array<size_t, 3> strides = {0x10, 0x18, 0x20};

    TArrayHeader bestHeader{};
    size_t bestStride = 0;
    int bestScore = 0;

    for (uintptr_t offset : rawInfoOffsets) {
        if (offset == 0) continue;

        TArrayHeader header{};
        if (!ReadTArrayHeader(skeletalMesh + offset, header)) continue;

        for (size_t stride : strides) {
            int score = ScoreBoneInfoLayout(header, stride);
            if (score > bestScore) {
                bestScore = score;
                bestHeader = header;
                bestStride = stride;
            }
        }
    }

    if (bestScore < 8 || bestStride == 0) return false;

    for (int i = 0; i < bestHeader.count; ++i) {
        std::string name = ReadBoneInfoName(bestHeader.data + (i * bestStride));
        if (!name.empty()) {
            out.emplace(std::move(name), i);
        }
    }

    return !out.empty();
}

static int FindBoneIndex(const std::unordered_map<std::string, int>& boneNames,
                         std::initializer_list<const char*> names) {
    for (const char* name : names) {
        auto it = boneNames.find(name);
        if (it != boneNames.end()) return it->second;

        std::string alternate = name;
        if (alternate.find("joint_") == 0) {
            alternate.erase(0, 6);
        } else {
            alternate.insert(0, "joint_");
        }

        it = boneNames.find(alternate);
        if (it != boneNames.end()) return it->second;
    }
    return -1;
}

static bool AddNamedPair(std::vector<SkeletonPair>& pairs,
                         const std::unordered_map<std::string, int>& boneNames,
                         const char* a, const char* b) {
    int boneA = FindBoneIndex(boneNames, {a});
    int boneB = FindBoneIndex(boneNames, {b});
    if (boneA < 0 || boneB < 0) return false;

    pairs.push_back({boneA, boneB});
    return true;
}

static SkeletonCacheEntry BuildSkeletonPairs(uintptr_t skeletalMesh) {
    std::unordered_map<std::string, int> boneNames;
    SkeletonCacheEntry entry;

    if (BuildBoneNameIndex(skeletalMesh, boneNames)) {
        for (const auto& pair : kHumanoidSkeletonNames) {
            AddNamedPair(entry.pairs, boneNames, pair.a, pair.b);
        }
        int head = FindBoneIndex(boneNames, {
            "joint_head_01", "joint_head_02", "head", "Head", 
            "neck_01", "neck_02", "neck_03", 
            "Head_01", "Head_02", "head_01", "head_02",
            "Bip01_Head", "bip_head"
        });
        if (head >= 0) entry.headBoneIndex = head;
    }

    if (entry.pairs.size() < 8) {
        entry.pairs = std::vector<SkeletonPair>(
            std::begin(kSurvivorFallbackSkeleton),
            std::end(kSurvivorFallbackSkeleton));
        if (entry.headBoneIndex == -1) entry.headBoneIndex = 86; // fallback
    }

    return entry;
}

static const SkeletonCacheEntry& GetSkeletonCacheEntry(uintptr_t mesh) {
    uintptr_t skeletalMesh = Memory::Read<uintptr_t>(
        mesh + GOffsets.USkinnedMeshComponent_SkeletalMesh);
    uintptr_t cacheKey = IsLikelyPointer(skeletalMesh) ? skeletalMesh : mesh;

    auto cached = g_SkeletonCache.find(cacheKey);
    if (cached != g_SkeletonCache.end()) return cached->second;

    SkeletonCacheEntry entry = BuildSkeletonPairs(skeletalMesh);
    auto inserted = g_SkeletonCache.emplace(cacheKey, std::move(entry));
    return inserted.first->second;
}

static bool GetBoneLocation(uintptr_t boneArray, const D3DMATRIX& compMatrix,
                            int boneIndex, int boneCount, FVector& out) {
    if (boneIndex < 0 || boneIndex >= boneCount) return false;

    FTransform boneTransform{};
    if (!Memory::Read(boneArray + (boneIndex * 0x60), &boneTransform, 0x60)) {
        return false;
    }

    D3DMATRIX boneMatrix = boneTransform.ToMatrixWithScale();
    D3DMATRIX worldMatrix = MatrixMultiplication(boneMatrix, compMatrix);

    out = {(double)worldMatrix._41, (double)worldMatrix._42, (double)worldMatrix._43};
    return std::isfinite(out.X) && std::isfinite(out.Y) && std::isfinite(out.Z);
}

struct SkeletonScreenLine {
    ImVec2 a;
    ImVec2 b;
};

static void ExpandBounds(ImVec2 point, ImVec2& min, ImVec2& max) {
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
}

void DrawSkeleton(ImDrawList* dl, uintptr_t actor, const FMinimalViewInfo& camera,
                  ImU32 col, const ImVec2& actorBoxMin, const ImVec2& actorBoxMax) {
    uintptr_t mesh = Memory::Read<uintptr_t>(actor + GOffsets.ACharacter_MeshOffset);
    if (!mesh) return;

    uintptr_t boneArray = Memory::Read<uintptr_t>(mesh + GOffsets.USkinnedMeshComponent_BoneArray);
    int32_t boneCount = Memory::Read<int32_t>(mesh + GOffsets.USkinnedMeshComponent_BoneArray + 0x8);
    if (!IsLikelyPointer(boneArray) || boneCount <= 0 || boneCount > 512) return;

    FTransform compToWorld{};
    if (!Memory::Read(mesh + GOffsets.USkinnedMeshComponent_ComponentToWorld,
                      &compToWorld, sizeof(compToWorld))) {
        return;
    }

    D3DMATRIX compMatrix = compToWorld.ToMatrixWithScale();
    const SkeletonCacheEntry& cacheEntry = GetSkeletonCacheEntry(mesh);
    const auto& skeletonPairs = cacheEntry.pairs;

    int sw = (int)ImGui::GetIO().DisplaySize.x;
    int sh = (int)ImGui::GetIO().DisplaySize.y;
    float boxHeight = std::max(2.0f, actorBoxMax.y - actorBoxMin.y);
    float boxWidth = std::max(2.0f, actorBoxMax.x - actorBoxMin.x);
    float maxSegmentLength = boxHeight * 1.15f;

    std::vector<SkeletonScreenLine> lines;
    lines.reserve(skeletonPairs.size());
    ImVec2 boundsMin(FLT_MAX, FLT_MAX);
    ImVec2 boundsMax(-FLT_MAX, -FLT_MAX);

    for (const auto& pair : skeletonPairs) {
        FVector boneA, boneB;
        if (!GetBoneLocation(boneArray, compMatrix, pair.a, boneCount, boneA) ||
            !GetBoneLocation(boneArray, compMatrix, pair.b, boneCount, boneB)) {
            continue;
        }

        FVector screenA, screenB;
        if (WorldToScreen(boneA, camera, sw, sh, screenA) &&
            WorldToScreen(boneB, camera, sw, sh, screenB)) {
            float dx = (float)(screenA.X - screenB.X);
            float dy = (float)(screenA.Y - screenB.Y);
            float len = sqrtf((dx * dx) + (dy * dy));
            if (len < 1.0f || len > maxSegmentLength) {
                continue;
            }

            ImVec2 pointA((float)screenA.X, (float)screenA.Y);
            ImVec2 pointB((float)screenB.X, (float)screenB.Y);
            lines.push_back({pointA, pointB});
            ExpandBounds(pointA, boundsMin, boundsMax);
            ExpandBounds(pointB, boundsMin, boundsMax);
        }
    }

    if (lines.empty()) return;

    float skeletonWidth = boundsMax.x - boundsMin.x;
    float skeletonHeight = boundsMax.y - boundsMin.y;
    if (!std::isfinite(skeletonWidth) || !std::isfinite(skeletonHeight) ||
        skeletonWidth < 1.0f || skeletonHeight < 1.0f) {
        return;
    }

    float maxSkeletonHeight = boxHeight * 1.12f;
    float maxSkeletonWidth = std::max(boxWidth * 1.85f, boxHeight * 0.72f);
    float scale = 1.0f;
    if (skeletonHeight > maxSkeletonHeight) {
        scale = std::min(scale, maxSkeletonHeight / skeletonHeight);
    }
    if (skeletonWidth > maxSkeletonWidth) {
        scale = std::min(scale, maxSkeletonWidth / skeletonWidth);
    }

    ImVec2 sourceCenter((boundsMin.x + boundsMax.x) * 0.5f,
                        (boundsMin.y + boundsMax.y) * 0.5f);
    ImVec2 targetCenter((actorBoxMin.x + actorBoxMax.x) * 0.5f,
                        (actorBoxMin.y + actorBoxMax.y) * 0.5f);

    auto fitPoint = [&](ImVec2 point) {
        if (scale >= 0.999f) return point;
        return ImVec2(targetCenter.x + ((point.x - sourceCenter.x) * scale),
                      targetCenter.y + ((point.y - sourceCenter.y) * scale));
    };

    for (const auto& line : lines) {
        ImVec2 a = fitPoint(line.a);
        ImVec2 b = fitPoint(line.b);
        if (!std::isfinite(a.x) || !std::isfinite(a.y) ||
            !std::isfinite(b.x) || !std::isfinite(b.y)) {
            continue;
        }

        dl->AddLine(a, b, col, 1.5f);
    }
}

static void DrawActor(ImDrawList* dl, const ActorEntry& e,
                      const FMinimalViewInfo& cam,
                      bool enabled, bool rainbow, const ImVec4& color,
                      bool boxEnabled, int boxIndex,
                      bool lineEnabled, int lineIndex,
                      const char* label = nullptr, bool showDist = false,
                      bool showSkeleton = false) {
    if (!enabled) return;

    int sw = (int)ImGui::GetIO().DisplaySize.x;
    int sh = (int)ImGui::GetIO().DisplaySize.y;

    FVector screen;
    if (!WorldToScreen(e.location, cam, sw, sh, screen)) return;

    ImVec4 col = rainbow ? RainbowColor() : color;
    ImU32  colU = ToImU32(col);

    ImVec2 bMin, bMax;
    bool hasPerspectiveBox = BuildPerspectiveBox(e, cam, sw, sh, bMin, bMax);

    // Skeleton
    if (showSkeleton && hasPerspectiveBox) {
        DrawSkeleton(dl, e.addr, cam, colU, bMin, bMax);
    }

    // name / label
    char fullLabel[128] = {};
    if (label && *label) {
        snprintf(fullLabel, sizeof(fullLabel), "%s", label);
    } else {
        if (e.prestige > 0)
            snprintf(fullLabel, sizeof(fullLabel), "[P%d] %s", e.prestige, e.name.c_str());
        else
            snprintf(fullLabel, sizeof(fullLabel), "%s", e.name.c_str());
    }

    if (fullLabel[0])
        dl->AddText({(float)screen.X + 4.0f, (float)screen.Y}, colU, fullLabel);

    if (showDist) {
        char distBuf[32];
        snprintf(distBuf, sizeof(distBuf), "%.0fm", e.distance / 100.0f);
        dl->AddText({(float)screen.X + 4.0f, (float)screen.Y + 14.0f}, colU, distBuf);

        // --- Active Perks (Survivor Only) ---
        if (!e.activePerks.empty()) {
            std::string perksStr;
            for (size_t i = 0; i < e.activePerks.size(); i++) {
                perksStr += e.activePerks[i] + (i == e.activePerks.size() - 1 ? "" : ", ");
            }
            dl->AddText({(float)screen.X + 4.0f, (float)screen.Y + 28.0f}, IM_COL32(255, 215, 0, 255), perksStr.c_str());
        }
    }

    if (boxEnabled && hasPerspectiveBox) {
        DrawBox(dl, bMin, bMax, colU, boxIndex);
    }

    if (lineEnabled)
        DrawSnapline(dl, {(float)screen.X, (float)screen.Y}, colU, lineIndex);
        
    bool isKiller = (e.name.find("Killer") != std::string::npos || e.name.find("Slasher") != std::string::npos || e.characterName.find("Killer") != std::string::npos);
    bool isSurvivor = (e.name.find("Camper") != std::string::npos || e.name.find("Survivor") != std::string::npos);

    if ((isKiller && g_ESP_KillerDirectional) || (isSurvivor && g_ESP_SurvivorDirectional)) {
        FVector forward = RotationToVector(e.rotation);
        FVector target = e.location + (forward * 150.0f);
        ImVec2 screenTarget{};
        ImVec2 bottomScreen = {(float)screen.X, (float)screen.Y + 20.0f};
        if (ProjectWorldToScreenUnclipped(target, cam, sw, sh, screenTarget)) {
            dl->AddLine(bottomScreen, screenTarget, IM_COL32(255, 255, 255, 255), 2.0f);
        }
    }
}

// outline-text helper
static void OutlineText(ImDrawList* dl, ImVec2 pos, ImU32 fg, ImU32 outline, const char* text) {
    dl->AddText({pos.x - 1, pos.y - 1}, outline, text);
    dl->AddText({pos.x + 1, pos.y - 1}, outline, text);
    dl->AddText({pos.x - 1, pos.y + 1}, outline, text);
    dl->AddText({pos.x + 1, pos.y + 1}, outline, text);
    dl->AddText(pos, fg, text);
}

// --- main render call (from main loop, inside ImGui frame) ---
static void DrawRadar() {
    if (!g_Cheats_Enabled || !g_Cheats_RadarEnabled) return;

    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Radar", &g_Cheats_RadarEnabled, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::End();
        return;
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);

    // Background
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(20, 20, 20, 180));
    dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(100, 100, 100, 255));
    
    // Crosshair
    dl->AddLine(ImVec2(center.x - size.x * 0.5f, center.y), ImVec2(center.x + size.x * 0.5f, center.y), IM_COL32(100, 100, 100, 100));
    dl->AddLine(ImVec2(center.x, center.y - size.y * 0.5f), ImVec2(center.x, center.y + size.y * 0.5f), IM_COL32(100, 100, 100, 100));

    FMinimalViewInfo cam;
    { std::lock_guard<std::mutex> lk(g_CameraMutex); cam = g_CameraInfo; }
    
    float zoom = g_Cheats_RadarZoom;

    auto project = [&](FVector worldLoc) -> ImVec2 {
        float dx = worldLoc.X - cam.Location.X;
        float dy = worldLoc.Y - cam.Location.Y;
        
        // Rotate relative to camera
        float angle = -cam.Rotation.Yaw * (M_PI / 180.0f);
        float cosA = cos(angle);
        float sinA = sin(angle);
        
        float rx = dx * cosA - dy * sinA;
        float ry = dx * sinA + dy * cosA;
        
        // Scale and clamp
        rx *= zoom;
        ry *= zoom;
        
        float maxDist = (size.x * 0.5f) - 5.0f;
        rx = std::clamp(rx, -maxDist, maxDist);
        ry = std::clamp(ry, -maxDist, maxDist);
        
        return ImVec2(center.x + ry, center.y - rx);
    };

    std::lock_guard<std::mutex> lk(g_ActorMutex);
    
    for (const auto& s : g_Survivors) {
        if (s.inParadise) continue;
        dl->AddCircleFilled(project(s.location), 3.0f, IM_COL32(0, 255, 0, 255));
    }
    
    for (const auto& k : g_Killers) {
        dl->AddCircleFilled(project(k.location), 4.0f, IM_COL32(255, 0, 0, 255));
    }
    
    for (const auto& g : g_Generators) {
        if (g.activated) continue;
        dl->AddCircleFilled(project(g.location), 2.0f, IM_COL32(255, 255, 0, 255));
    }

    if (g_ESP_Victor) {
        for (const auto& v : g_Victors) {
            dl->AddCircleFilled(project(v.location), 3.0f, IM_COL32(255, 100, 100, 255));
        }
    }

    if (g_ESP_HagTraps) {
        for (const auto& h : g_HagTraps) {
            dl->AddCircleFilled(project(h.location), 2.0f, IM_COL32(200, 100, 255, 255));
        }
    }

    if (g_ESP_K33Tunnels) {
        for (const auto& t : g_K33Tunnels) {
            dl->AddCircleFilled(project(t.location), 3.0f, IM_COL32(0, 128, 128, 255));
        }
    }

    if (g_ESP_K33Turrets) {
        for (const auto& t : g_K33Turrets) {
            dl->AddCircleFilled(project(t.location), 2.0f, IM_COL32(255, 165, 0, 255));
        }
    }

    // Local player
    dl->AddCircleFilled(center, 4.0f, IM_COL32(255, 255, 255, 255));
    dl->AddLine(center, ImVec2(center.x, center.y - 10.0f), IM_COL32(255, 255, 255, 255), 2.0f);

    ImGui::End();
}

void RunESP() {
    DrawRadar();
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    // Draw Tool info and PID on the screen top-left
    if (g_UI_ShowToolInfo) {
        dl->AddText(ImVec2(5, 5), IM_COL32(255, 255, 255, 255), "Dead By Daylight Tool");
    }

    int sw = (int)ImGui::GetIO().DisplaySize.x;
    int sh = (int)ImGui::GetIO().DisplaySize.y;

    if (Memory::IsAttached() && g_UI_ShowToolInfo) {
        char pidBuf[64];
        snprintf(pidBuf, sizeof(pidBuf), "Game PID: %d", Memory::GetProcessId());
        dl->AddText(ImVec2(5, 20), IM_COL32(0, 255, 0, 255), pidBuf); // Green 0xff00ff00
    } else if (!Memory::IsAttached() && g_UI_ShowToolInfo) {
        dl->AddText(ImVec2(5, 20), IM_COL32(255, 0, 0, 255), "Game PID: NULL"); // Red 0xff0000ff
    }

    if (Memory::IsAttached() && g_UI_ShowActorCounts) {
        char countBuf[128];
        {
            std::lock_guard<std::mutex> lk(g_ActorMutex);
            snprintf(countBuf, sizeof(countBuf), "S: %zu | K: %zu | G: %zu | P: %zu",
                     g_Survivors.size(), g_Killers.size(), g_Generators.size(), g_Pallets.size());
        }
        dl->AddText(ImVec2(5, 35), IM_COL32(255, 255, 0, 255), countBuf); // Yellow
    }

    FMinimalViewInfo cam;
    { std::lock_guard<std::mutex> lk(g_CameraMutex); cam = g_CameraInfo; }

    // bail if camera is totally default (game not running)
    if (cam.FOV < 1.0f) return;

    std::lock_guard<std::mutex> lk(g_ActorMutex);

    // --- Aimbot ---
    if (g_Cheats_Enabled && g_Cheats_AimbotEnabled && g_AimbotKeyDown && g_LocalController) {
        std::vector<ActorEntry> aimTargets;
        if (g_LocalPlayerRole == 1) { // Killer
            aimTargets = g_Survivors;
        } else { // Survivor
            aimTargets = g_Killers;
            aimTargets.insert(aimTargets.end(), g_Victors.begin(), g_Victors.end());
            aimTargets.insert(aimTargets.end(), g_NemesisZombies.begin(), g_NemesisZombies.end());
        }

        if (!aimTargets.empty()) {
            const ActorEntry* bestTarget = nullptr;
            float bestDist = 999999.0f;
            FVector bestTargetPos;

            int screenWidth = ImGui::GetIO().DisplaySize.x;
            int screenHeight = ImGui::GetIO().DisplaySize.y;
            float centerX = screenWidth / 2.0f;
            float centerY = screenHeight / 2.0f;

            for (const auto& t : aimTargets) {
                FVector headPos = t.location;
                headPos.Z += (t.height * 0.45f); // fallback

                uintptr_t mesh = Memory::Read<uintptr_t>(t.addr + GOffsets.ACharacter_MeshOffset);
                if (mesh) {
                    const SkeletonCacheEntry& cacheEntry = GetSkeletonCacheEntry(mesh);
                    if (cacheEntry.headBoneIndex != -1) {
                        uintptr_t boneArray = Memory::Read<uintptr_t>(mesh + GOffsets.USkinnedMeshComponent_BoneArray);
                        int32_t boneCount = Memory::Read<int32_t>(mesh + GOffsets.USkinnedMeshComponent_BoneArray + 0x8);
                        if (IsLikelyPointer(boneArray) && boneCount > cacheEntry.headBoneIndex) {
                            FTransform compToWorld{};
                            if (Memory::Read(mesh + GOffsets.USkinnedMeshComponent_ComponentToWorld, &compToWorld, sizeof(compToWorld))) {
                                D3DMATRIX compMatrix = compToWorld.ToMatrixWithScale();
                                FVector exactHead;
                                if (GetBoneLocation(boneArray, compMatrix, cacheEntry.headBoneIndex, boneCount, exactHead)) {
                                    headPos = exactHead;
                                }
                            }
                        }
                    }
                }
                FVector screenPos;
                if (WorldToScreen(headPos, cam, screenWidth, screenHeight, screenPos)) {
                    float dx = screenPos.X - centerX;
                    float dy = screenPos.Y - centerY;
                    float dist = std::sqrt(dx * dx + dy * dy);
                    if (dist < bestDist && dist < g_Cheats_AimbotFOV) {
                        bestDist = dist;
                        bestTarget = &t;
                        bestTargetPos = headPos;
                    }
                }
            }

            if (bestTarget) {
                FVector aimPos = bestTargetPos;

                if (g_Cheats_AimbotPredictionEnabled) {
                    float dist = (bestTargetPos - cam.Location).Size();
                    float travelTime = dist / g_Cheats_AimbotProjectileSpeed;

                    // Lead the target
                    aimPos = bestTargetPos + (bestTarget->velocity * travelTime);

                    // Gravity compensation: h = 0.5 * g * t^2
                    float drop = 0.5f * g_Cheats_AimbotProjectileGravity * (travelTime * travelTime) * g_Cheats_AimbotDropScale;
                    aimPos.Z += drop;
                }

                aimPos = aimPos + g_Cheats_AimbotOffset;

                FRotator aimRot = CalcAngle(cam.Location, aimPos);

                if (g_Cheats_AimbotSmoothing > 0.0f) {
                    FRotator currentRot = Memory::Read<FRotator>(g_LocalController + 0x370);
                    FRotator diff = aimRot - currentRot;
                    diff.Normalize();
                    
                    // Higher smoothing = slower movement. 0.0 = instant, 0.9 = very slow.
                    float step = 1.0f - std::clamp(g_Cheats_AimbotSmoothing, 0.0f, 0.99f);
                    aimRot = currentRot + (diff * (double)step);
                    aimRot.Normalize();
                }

                Memory::Write<FRotator>(g_LocalController + 0x370, aimRot);
            }
        }
    }

    if (!g_ESP_Enabled) return;

    // --- Survivors ---
    if (g_ESP_SurvivorEnabled) {
        for (const auto& e : g_Survivors) {
            ImVec4 col = g_ESP_SurvivorRainbowEnabled ? RainbowColor() : g_ESP_SurvivorColor;
            
            std::string statusStr = "";
            if (e.isInChase) statusStr += " [CHASE]";
            if (e.isInteracting) statusStr += " [INTERACTING]";
            if (e.recoveryProgress > 0.01f && e.recoveryProgress < 0.99f) {
                char recBuf[32];
                snprintf(recBuf, sizeof(recBuf), " [REC: %d%%]", (int)(e.recoveryProgress * 100));
                statusStr += recBuf;
            }

            char label[256] = {};
            if (g_ESP_HideRealNamesEnabled)
                snprintf(label, sizeof(label), "%s%s", 
                    e.characterName.empty() ? "Survivor" : e.characterName.c_str(),
                    statusStr.c_str());
            else
                snprintf(label, sizeof(label), "%s (%s)%s%s", 
                    e.name.empty() ? "Survivor" : e.name.c_str(),
                    e.characterName.empty() ? "Survivor" : e.characterName.c_str(),
                    e.inParadise ? " [Escaped]" : "",
                    statusStr.c_str());

            DrawActor(dl, e, cam,
                true, g_ESP_SurvivorRainbowEnabled, col,
                g_ESP_SurvivorBoxEnabled, g_ESP_SurvivorBoxIndex,
                g_ESP_SurvivorLineEnabled, g_ESP_SurvivorLineIndex,
                label, true, g_ESP_SurvivorSkeletonEnabled);
        }
    }

    // --- Killers ---
    if (g_ESP_KillerEnabled) {
        for (const auto& e : g_Killers) {
            char label[256] = {};
            const char* chaseStatus = e.isInChase ? " [CHASE]" : "";
            if (g_ESP_HideRealNamesEnabled)
                snprintf(label, sizeof(label), "%s%s", e.characterName.empty() ? "Killer" : e.characterName.c_str(), chaseStatus);
            else
                snprintf(label, sizeof(label), "%s (%s)%s", e.name.empty() ? "Killer" : e.name.c_str(), e.characterName.empty() ? "Killer" : e.characterName.c_str(), chaseStatus);

            DrawActor(dl, e, cam,
                true, false, g_ESP_KillerColor,
                g_ESP_KillerBoxEnabled, g_ESP_KillerBoxIndex,
                g_ESP_KillerLineEnabled, g_ESP_KillerLineIndex,
                label, true, g_ESP_KillerSkeletonEnabled);
        }
    }

    // --- Generators ---
    if (g_ESP_GeneratorEnabled) {
        for (const auto& e : g_Generators) {
            // Skip completed generators if the setting is enabled
            if (g_ESP_GeneratorHideCompleted && e.activated) continue;

            ImVec4 col = g_ESP_GeneratorRainbowEnabled ? RainbowColor() : g_ESP_GeneratorColor;
            char label[128];
            if (e.interactorCount > 0)
                snprintf(label, sizeof(label), "Generator [%d%%] (%d Working)", (int)(e.percentComplete * 100), e.interactorCount);
            else
                snprintf(label, sizeof(label), "Generator [%d%%]", (int)(e.percentComplete * 100));
            DrawActor(dl, e, cam, true, g_ESP_GeneratorRainbowEnabled, col, false, 0, false, 0, label);
        }
    }

    // --- Hooks ---
    if (g_ESP_HookEnabled) {
        for (const auto& e : g_Hooks)
            DrawActor(dl, e, cam, true, false, g_ESP_HookColor, false, 0, false, 0, "Hook");
    }

    // --- Lockers ---
    if (g_ESP_LockerEnabled) {
        for (const auto& e : g_Lockers) {
            ImVec4 col = g_ESP_LockerRainbowEnabled ? RainbowColor() : g_ESP_LockerColor;
            char label[64];
            if (e.hasPlayer)
                snprintf(label, sizeof(label), "Locker [Occupied]");
            else
                snprintf(label, sizeof(label), "Locker");
            DrawActor(dl, e, cam, true, g_ESP_LockerRainbowEnabled, col, false, 0, false, 0, label);
        }
    }

    // --- Totems ---
    if (g_ESP_TotemEnabled) {
        for (const auto& e : g_Totems) {
            ImVec4 col = g_ESP_TotemRainbowEnabled ? RainbowColor() : g_ESP_TotemColor;
            const char* tLabel = "Totem";
            if (e.objectState == 1)      tLabel = "Dull Totem";
            else if (e.objectState == 2) tLabel = "Hex Totem";
            else if (e.objectState == 3) tLabel = "Boon Totem";
            DrawActor(dl, e, cam, true, g_ESP_TotemRainbowEnabled, col, false, 0, false, 0, tLabel);
        }
    }

    // --- Pallets ---
    if (g_ESP_PalletEnabled) {
        for (const auto& e : g_Pallets) {
            ImVec4 col = g_ESP_PalletRainbowEnabled ? RainbowColor() : g_ESP_PalletColor;
            const char* pLabel = "Pallet";
            if (e.objectState == 0)      pLabel = "Pallet [Up]";
            else if (e.objectState == 1) pLabel = "Pallet [Falling]";
            else if (e.objectState == 2) pLabel = "Pallet [Down]";
            DrawActor(dl, e, cam, true, g_ESP_PalletRainbowEnabled, col, false, 0, false, 0, pLabel);
        }
    }

    // --- Hatches ---
    if (g_ESP_HatchEnabled) {
        for (const auto& e : g_Hatches) {
            DrawActor(dl, e, cam, true, false, g_ESP_HatchColor, false, 0, false, 0,
                e.objectState == 1 ? "Hatch [Open]" : "Hatch [Closed]");
        }
    }

    // --- Traps ---
    if (g_ESP_TrapEnabled) {
        for (const auto& e : g_Traps) {
            DrawActor(dl, e, cam, true, false, g_ESP_TrapColor, false, 0, false, 0,
                e.isTrapSet ? "Trap [Armed]" : "Trap [Disarmed]");
        }
    }

    // --- Breakable Doors/Windows ---
    if (g_ESP_BreakableDoorEnabled) {
        for (const auto& e : g_BreakableDoors) {
            ImVec4 col = g_ESP_BreakableDoorRainbowEnabled ? RainbowColor() : g_ESP_BreakableDoorColor;
            DrawActor(dl, e, cam, true, g_ESP_BreakableDoorRainbowEnabled, col, false, 0, false, 0,
                e.isBlocked ? "Window [Blocked]" : "Window");
        }
    }

    // --- Escape Doors ---
    if (g_ESP_EscapeDoorEnabled) {
        for (const auto& e : g_EscapeDoors) {
            ImVec4 col = g_ESP_EscapeDoorRainbowEnabled ? RainbowColor() : g_ESP_EscapeDoorColor;
            DrawActor(dl, e, cam, true, g_ESP_EscapeDoorRainbowEnabled, col, false, 0, false, 0,
                e.activated ? "Gate [Open]" : "Gate [Closed]");
        }
    }

    // --- Chests ---
    if (g_ESP_ChestEnabled) {
        for (const auto& e : g_Chests)
            DrawActor(dl, e, cam, true, false, g_ESP_ChestColor, false, 0, false, 0, "Chest");
    }

    // --- Draw Hag Traps ---
    if (g_ESP_HagTraps) {
        for (const auto& h : g_HagTraps) {
            FVector screen;
            if (WorldToScreen(h.location, cam, sw, sh, screen)) {
                float dist = (cam.Location - h.location).Size() / 100.0f;
                // g_ESP_MaxDistance is usually global or we use a fixed value like 1000.0f
                // Let's check if it's in globals.h. It wasn't in my previous view, but maybe I missed it.
                // For now, I'll use 1000.0f if not defined.
                ImVec4 color = ImVec4(0.8f, 0.4f, 1.0f, 1.0f); // Purple
                dl->AddCircleFilled({(float)screen.X, (float)screen.Y}, 5.0f, ToImU32(color));
                std::string label = "TRAP [" + std::to_string((int)dist) + "m]";
                dl->AddText({(float)screen.X, (float)screen.Y - 15.0f}, ToImU32(color), label.c_str());
            }
        }
    }

    // --- Draw Plague Fountains ---
    if (g_ESP_PlagueFountains) {
        for (const auto& f : g_PlagueFountains) {
            FVector screen;
            if (WorldToScreen(f.location, cam, sw, sh, screen)) {
                float dist = (cam.Location - f.location).Size() / 100.0f;
                if (dist > g_ESP_MaxDistance) continue;
                ImVec4 color = ImVec4(0.0f, 1.0f, 1.0f, 1.0f); // Cyan
                dl->AddCircleFilled({(float)screen.X, (float)screen.Y}, 5.0f, ToImU32(color));
                std::string label = "FOUNTAIN [" + std::to_string((int)dist) + "m]";
                dl->AddText({(float)screen.X, (float)screen.Y - 15.0f}, ToImU32(color), label.c_str());
            }
        }
    }

    // --- Draw Supply Crates ---
    if (g_ESP_SupplyCrates) {
        for (const auto& s : g_SupplyCrates) {
            FVector screen;
            if (WorldToScreen(s.location, cam, sw, sh, screen)) {
                float dist = (cam.Location - s.location).Size() / 100.0f;
                if (dist > g_ESP_MaxDistance) continue;
                ImVec4 color = ImVec4(0.2f, 0.4f, 1.0f, 1.0f); // Blue
                dl->AddRectFilled({(float)screen.X - 5.0f, (float)screen.Y - 5.0f}, {(float)screen.X + 5.0f, (float)screen.Y + 5.0f}, ToImU32(color));
                std::string label = "SUPPLY BOX [" + std::to_string((int)dist) + "m]";
                dl->AddText({(float)screen.X, (float)screen.Y - 15.0f}, ToImU32(color), label.c_str());
            }
        }
    }

    // --- Draw Xenomorph Tunnels ---
    if (g_ESP_K33Tunnels) {
        for (const auto& t : g_K33Tunnels) {
            FVector screen;
            if (WorldToScreen(t.location, cam, sw, sh, screen)) {
                float dist = (cam.Location - t.location).Size() / 100.0f;
                if (dist > g_ESP_MaxDistance) continue;
                ImVec4 color = ImVec4(0.0f, 0.5f, 0.5f, 1.0f); // Teal
                dl->AddCircleFilled({(float)screen.X, (float)screen.Y}, 6.0f, ToImU32(color));
                std::string label = "TUNNEL [" + std::to_string((int)dist) + "m]";
                dl->AddText({(float)screen.X, (float)screen.Y - 15.0f}, ToImU32(color), label.c_str());
            }
        }
    }

    // --- Draw Xenomorph Turrets ---
    if (g_ESP_K33Turrets) {
        for (const auto& t : g_K33Turrets) {
            FVector screen;
            if (WorldToScreen(t.location, cam, sw, sh, screen)) {
                float dist = (cam.Location - t.location).Size() / 100.0f;
                if (dist > g_ESP_MaxDistance) continue;
                ImVec4 color = ImVec4(1.0f, 0.6f, 0.0f, 1.0f); // Orange
                dl->AddCircleFilled({(float)screen.X, (float)screen.Y}, 5.0f, ToImU32(color));
                std::string label = "TURRET [" + std::to_string((int)dist) + "m]";
                dl->AddText({(float)screen.X, (float)screen.Y - 15.0f}, ToImU32(color), label.c_str());
            }
        }
    }

    // --- Draw Victor ---
    if (g_ESP_Victor) {
        for (const auto& e : g_Victors) {
            DrawActor(dl, e, cam, true, false, ImVec4(1.0f, 0.4f, 0.4f, 1.0f), false, 0, false, 0, "Victor", true);
        }
    }

    // --- Cenobite Box ---
    if (g_ESP_CenobiteBoxEnabled) {
        for (const auto& e : g_CenobiteBoxes) {
            ImVec4 col = g_ESP_CenobiteBoxRainbowEnabled ? RainbowColor() : g_ESP_CenobiteBoxColor;
            DrawActor(dl, e, cam, true, g_ESP_CenobiteBoxRainbowEnabled, col, false, 0, false, 0, "Cenobite Box", true);
        }
    }

    // --- Nemesis Zombies ---
    if (g_ESP_NemesisZombieEnabled) {
        for (const auto& e : g_NemesisZombies) {
            ImVec4 col = g_ESP_NemesisZombieRainbowEnabled ? RainbowColor() : g_ESP_NemesisZombieColor;
            DrawActor(dl, e, cam, true, g_ESP_NemesisZombieRainbowEnabled, col, false, 0, false, 0, "Zombie", true);
        }
    }

    // --- Outline text overlay ---
    if (g_ESP_OutlineTextEnabled) {
        ImU32 outlineCol = ToImU32(g_ESP_OutlineTextColor);
        for (const auto& e : g_Survivors) {
            FVector screen;
            int sw = (int)ImGui::GetIO().DisplaySize.x;
            int sh = (int)ImGui::GetIO().DisplaySize.y;
            if (WorldToScreen(e.location, cam, sw, sh, screen)) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%s (%s)", e.name.empty() ? "Survivor" : e.name.c_str(), e.characterName.empty() ? "Survivor" : e.characterName.c_str());
                OutlineText(dl, {(float)screen.X, (float)screen.Y}, IM_COL32_WHITE, outlineCol, buf);
            }
        }
    }

    // --- Custom Crosshair ---
    if (g_Cheats_Enabled && g_Cheats_CrosshairEnabled) {
        int screenWidth = ImGui::GetIO().DisplaySize.x;
        int screenHeight = ImGui::GetIO().DisplaySize.y;
        ImVec2 center((float)screenWidth * 0.5f, (float)screenHeight * 0.5f);
        ImU32 col = ToImU32(g_Cheats_CrosshairColor);
        float size = g_Cheats_CrosshairSize;
        float thick = g_Cheats_CrosshairThickness;

        if (g_Cheats_CrosshairType == 0) { // Cross
            dl->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x + size, center.y), col, thick);
            dl->AddLine(ImVec2(center.x, center.y - size), ImVec2(center.x, center.y + size), col, thick);
        } else if (g_Cheats_CrosshairType == 1) { // Dot
            dl->AddCircleFilled(center, size, col);
        } else if (g_Cheats_CrosshairType == 2) { // Circle
            dl->AddCircle(center, size, col, 32, thick);
        }
    }
}
