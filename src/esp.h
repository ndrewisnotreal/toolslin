#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <cstdint>
#include "math.h"

enum ActorType {
    SURVIVOR,
    KILLER,
    GENERATOR,
    HOOK,
    LOCKER,
    TOTEM,
    PALLET,
    WINDOW,
    CHEST,
    CENOBITE_BOX,
    NEMESIS_ZOMBIE,
    VICTOR,
    HAG_TRAP,
    PLAGUE_FOUNTAIN,
    SUPPLY_CRATE,
    K33_TUNNEL,
    K33_TURRET,
    HATCH,
    TRAP,
    BREAKABLE,
    ESCAPEDOOR,
    UNKNOWN
};

// mirrors the per-type actor lists the original tool maintains
struct ActorEntry {
    uintptr_t addr;
    FVector    location;
    FVector    velocity;
    std::string name;
    float      distance;
    float      height; // real height (capsule half-height * 2)

    // survivor-specific
    // survivor-specific
    int   healthState;
    bool  inParadise;
    std::vector<std::string> perks;
    std::vector<std::string> activePerks;

    // Identification
    std::string platform;
    std::string provider;
    int   prestige;
    std::string characterName;

    // generator-specific
    bool  activated;
    float percentComplete;

    // status
    bool  isInChase;
    bool  isInteracting;
    float interactionProgress;
    float recoveryProgress;
    float wiggleProgress;
    int   interactorCount;
    FRotator rotation;

    // object state
    int   objectState;
    bool  isTrapSet;
    bool  isBlocked;
    bool  hasPlayer;
};

struct PlayerLoadout {
    std::string name;
    int role; // 1 = Killer, 2 = Survivor
    std::string platform;
    std::string provider;
    int prestige;
    int ping;
    bool crossplayAllowed;
    int characterLevel;
    int survivorPips;
    int killerPips;
    std::vector<std::string> perks;
    std::string item;
    std::vector<std::string> addons;
    std::string offering;
    std::string characterName;
};

extern std::vector<ActorEntry> g_Survivors, g_Killers, g_Generators, g_Hooks, g_Lockers, g_Totems, g_Pallets, g_Windows, g_Chests, g_CenobiteBoxes, g_NemesisZombies, g_Victors, g_HagTraps, g_PlagueFountains, g_SupplyCrates, g_K33Tunnels, g_K33Turrets, g_Hatches, g_Traps, g_BreakableDoors, g_EscapeDoors, g_OtherActors;
extern std::vector<PlayerLoadout> g_PlayerLoadouts;

extern std::mutex g_ActorMutex;
extern FMinimalViewInfo g_CameraInfo;
extern std::mutex g_CameraMutex;

extern int g_LocalPlayerRole; // 1 = Killer, 2 = Survivor, 0 = None
extern uintptr_t g_LocalController;

void ESP_StartScanThread();
struct FSkillCheckDefinition {
    float SuccessZoneStart;        // 0x00
    float SuccessZoneEnd;          // 0x04
    float BonusZoneLength;         // 0x08
    float BonusZoneStart;          // 0x0C
    float ProgressRate;            // 0x10
    float StartingTickerPosition;  // 0x14
};

extern bool g_Cheats_AutoSkillCheckEnabled;
extern float g_Cheats_AutoSkillCheckDelayMs;
extern bool g_Cheats_SkillCheckDebugEnabled;
extern bool g_Cheats_SpamSpacebar;

void RunESP();
void ESP_Cleanup();
void PressSpaceBar(int holdMs);
void PressSpaceBar();
bool SetupUinput();
