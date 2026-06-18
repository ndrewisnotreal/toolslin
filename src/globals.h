#pragma once
#include <imgui.h>
#include <string>
#include "math.h"

struct WindowState {
    float X = 100.0f;
    float Y = 100.0f;
    float W = 400.0f;
    float H = 500.0f;
    bool Opened = true;
};

// UI Globals
extern WindowState g_UI_MainWindow;
extern WindowState g_UI_SkillcheckDebug;
extern WindowState g_UI_LoadoutWindow;
extern bool g_UI_ShouldApplyConfigPos;
extern bool g_UI_ShowToolInfo;
extern bool g_UI_ShowActorCounts;

// ESP globals
extern bool g_ESP_Enabled;
extern float g_ESP_MaxDistance;
extern bool g_ESP_SurvivorEnabled;
extern bool g_ESP_SurvivorRainbowEnabled;
extern ImVec4 g_ESP_SurvivorColor;
extern bool g_ESP_SurvivorBoxEnabled;
extern int g_ESP_SurvivorBoxIndex;
extern bool g_ESP_SurvivorLineEnabled;
extern int g_ESP_SurvivorLineIndex;
extern bool g_ESP_LoadoutWindowEnabled;
extern bool g_ESP_SurvivorSkeletonEnabled;
extern bool g_ESP_KillerSkeletonEnabled;
extern bool g_ESP_HideRealNamesEnabled;

extern bool g_ESP_KillerEnabled;
extern ImVec4 g_ESP_KillerColor;
extern bool g_ESP_KillerBoxEnabled;
extern int g_ESP_KillerBoxIndex;
extern bool g_ESP_KillerLineEnabled;
extern int g_ESP_KillerLineIndex;

extern bool g_ESP_GeneratorEnabled;
extern bool g_ESP_GeneratorRainbowEnabled;
extern ImVec4 g_ESP_GeneratorColor;
extern bool g_ESP_GeneratorHideCompleted;

extern bool g_ESP_HookEnabled;
extern ImVec4 g_ESP_HookColor;

extern bool g_ESP_LockerEnabled;
extern bool g_ESP_LockerRainbowEnabled;
extern ImVec4 g_ESP_LockerColor;

extern bool g_ESP_TotemEnabled;
extern bool g_ESP_TotemRainbowEnabled;
extern ImVec4 g_ESP_TotemColor;

extern bool g_ESP_PalletEnabled;
extern bool g_ESP_PalletRainbowEnabled;
extern ImVec4 g_ESP_PalletColor;

extern bool g_ESP_HatchEnabled;
extern ImVec4 g_ESP_HatchColor;

extern bool g_ESP_TrapEnabled;
extern ImVec4 g_ESP_TrapColor;

extern bool g_ESP_BreakableDoorEnabled;
extern bool g_ESP_BreakableDoorRainbowEnabled;
extern ImVec4 g_ESP_BreakableDoorColor;

extern bool g_ESP_EscapeDoorEnabled;
extern bool g_ESP_EscapeDoorRainbowEnabled;
extern ImVec4 g_ESP_EscapeDoorColor;

extern bool g_ESP_ChestEnabled;
extern ImVec4 g_ESP_ChestColor;

extern bool g_ESP_Victor;
extern bool g_ESP_HagTraps;
extern bool g_ESP_PlagueFountains;
extern bool g_ESP_SupplyCrates;
extern bool g_ESP_K33Tunnels;
extern bool g_ESP_K33Turrets;
extern bool g_ESP_KillerDirectional;
extern bool g_ESP_SurvivorDirectional;
extern bool g_ESP_CenobiteBoxEnabled;
extern bool g_ESP_CenobiteBoxRainbowEnabled;
extern ImVec4 g_ESP_CenobiteBoxColor;

extern bool g_ESP_NemesisZombieEnabled;
extern bool g_ESP_NemesisZombieRainbowEnabled;
extern ImVec4 g_ESP_NemesisZombieColor;

extern bool g_ESP_WindowEnabled;
extern ImVec4 g_ESP_WindowColor;

extern bool g_ESP_OutlineTextEnabled;
extern ImVec4 g_ESP_OutlineTextColor;

extern bool g_ESP_MatchInfoHUDEnabled;

// Aura globals
extern bool g_Aura_Enabled;
extern bool g_Aura_SurvivorEnabled;
extern bool g_Aura_SurvivorRainbowEnabled;
extern ImVec4 g_Aura_SurvivorColor;

extern bool g_Aura_KillerEnabled;
extern bool g_Aura_KillerRainbowEnabled;
extern ImVec4 g_Aura_KillerColor;

extern bool g_Aura_GeneratorEnabled;
extern bool g_Aura_GeneratorRainbowEnabled;
extern ImVec4 g_Aura_GeneratorColor;

extern bool g_Aura_HookEnabled;
extern ImVec4 g_Aura_HookColor;

extern bool g_Aura_LockerEnabled;
extern bool g_Aura_LockerRainbowEnabled;
extern ImVec4 g_Aura_LockerColor;

extern bool g_Aura_TotemEnabled;
extern bool g_Aura_TotemRainbowEnabled;
extern ImVec4 g_Aura_TotemColor;

extern bool g_Aura_PalletEnabled;
extern bool g_Aura_PalletRainbowEnabled;
extern ImVec4 g_Aura_PalletColor;

extern bool g_Aura_HatchEnabled;
extern ImVec4 g_Aura_HatchColor;

extern bool g_Aura_TrapEnabled;
extern ImVec4 g_Aura_TrapColor;

extern bool g_Aura_BreakableDoorEnabled;
extern bool g_Aura_BreakableDoorRainbowEnabled;
extern ImVec4 g_Aura_BreakableDoorColor;

extern bool g_Aura_EscapeDoorEnabled;
extern bool g_Aura_EscapeDoorRainbowEnabled;
extern ImVec4 g_Aura_EscapeDoorColor;

extern bool g_Aura_ChestEnabled;
extern ImVec4 g_Aura_ChestColor;

extern bool g_Aura_WindowEnabled;
extern bool g_Aura_WindowRainbowEnabled;
extern ImVec4 g_Aura_WindowColor;

extern bool g_Aura_OtherEnabled;
extern ImVec4 g_Aura_OtherColor;

// Cheats
extern bool g_Cheats_Enabled;
extern bool g_Cheats_FOVEnabled;
extern float g_Cheats_FOV;
extern bool g_Cheats_KillerStainEnabled;
extern bool g_Cheats_KillerStainRainbowEnabled;
extern ImVec4 g_Cheats_KillerStainColor;
extern bool g_Cheats_RadarEnabled;
extern float g_Cheats_RadarZoom;
extern float g_Cheats_RadarRange;

extern bool g_Cheats_AutoDeadHardEnabled;
extern float g_Cheats_DeadHardDistance;
extern bool g_Cheats_DeadHardPerfect; // Wait for lunge/hit animation
extern bool g_Cheats_AutoSkillCheckEnabled;
extern float g_Cheats_AutoSkillCheckDelayMs;
extern bool g_Cheats_SkillCheckDebugEnabled;
extern bool g_Cheats_SpamSpacebar;
extern bool g_Cheats_AimbotEnabled;
extern int g_Cheats_AimbotBind;
extern bool g_Cheats_AimbotPredictionEnabled;
extern float g_Cheats_AimbotProjectileSpeed;
extern float g_Cheats_AimbotProjectileGravity;
extern float g_Cheats_AimbotFOV;
extern float g_Cheats_AimbotSmoothing;
extern float g_Cheats_AimbotDropScale;
extern FVector g_Cheats_AimbotOffset;

// Crosshair
extern bool g_Cheats_CrosshairEnabled;
extern int g_Cheats_CrosshairType; // 0 = Cross, 1 = Dot, 2 = Circle
extern float g_Cheats_CrosshairSize;
extern float g_Cheats_CrosshairThickness;
extern ImVec4 g_Cheats_CrosshairColor;

// Settings
extern int g_FPS_Limit;

extern bool g_InLobby;
extern bool g_AimbotKeyDown;
extern bool g_AimbotIsBinding;

std::string GetBindName(int bind);
