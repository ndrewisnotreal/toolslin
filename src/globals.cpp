#include "globals.h"

WindowState g_UI_MainWindow = { 100.0f, 100.0f, 550.0f, 650.0f, true };
WindowState g_UI_SkillcheckDebug = { 700.0f, 100.0f, 300.0f, 400.0f, false };
WindowState g_UI_LoadoutWindow = { 700.0f, 550.0f, 400.0f, 300.0f, false };
bool g_UI_ShouldApplyConfigPos = false;
bool g_UI_ShowToolInfo = true;
bool g_UI_ShowActorCounts = true;

bool g_ESP_Victor = true;
bool g_ESP_HagTraps = true;
bool g_ESP_PlagueFountains = true;
bool g_ESP_SupplyCrates = true;
bool g_ESP_K33Tunnels = true;
bool g_ESP_K33Turrets = true;
bool g_ESP_KillerDirectional = true;
bool g_ESP_SurvivorDirectional = false;
bool g_ESP_CenobiteBox = true;
bool g_ESP_Enabled = true;
float g_ESP_MaxDistance = 1000.0f;
bool g_ESP_SurvivorEnabled = true;
bool g_ESP_SurvivorRainbowEnabled = false;
ImVec4 g_ESP_SurvivorColor = {0.2f, 0.8f, 0.2f, 1.0f};
bool g_ESP_SurvivorBoxEnabled = false;
int  g_ESP_SurvivorBoxIndex = 0;
bool g_ESP_SurvivorLineEnabled = false;
int  g_ESP_SurvivorLineIndex = 0;
bool g_ESP_LoadoutWindowEnabled = true;
bool g_ESP_SurvivorSkeletonEnabled = true;
bool g_ESP_KillerSkeletonEnabled = true;
bool g_ESP_HideRealNamesEnabled = false;

bool g_ESP_KillerEnabled = false;
ImVec4 g_ESP_KillerColor = {0.9f, 0.1f, 0.1f, 1.0f};
bool g_ESP_KillerBoxEnabled = false;
int  g_ESP_KillerBoxIndex = 0;
bool g_ESP_KillerLineEnabled = false;
int  g_ESP_KillerLineIndex = 0;

bool g_ESP_GeneratorEnabled = false;
bool g_ESP_GeneratorRainbowEnabled = false;
ImVec4 g_ESP_GeneratorColor = {1.0f, 1.0f, 0.0f, 1.0f};
bool g_ESP_GeneratorHideCompleted = true;

bool g_ESP_HookEnabled = false;
ImVec4 g_ESP_HookColor = {0.7f, 0.4f, 0.1f, 1.0f};

bool g_ESP_LockerEnabled = false;
bool g_ESP_LockerRainbowEnabled = false;
ImVec4 g_ESP_LockerColor = {0.4f, 0.4f, 0.8f, 1.0f};

bool g_ESP_TotemEnabled = false;
bool g_ESP_TotemRainbowEnabled = false;
ImVec4 g_ESP_TotemColor = {0.6f, 0.0f, 0.8f, 1.0f};

bool g_ESP_PalletEnabled = false;
bool g_ESP_PalletRainbowEnabled = false;
ImVec4 g_ESP_PalletColor = {0.8f, 0.6f, 0.0f, 1.0f};

bool g_ESP_HatchEnabled = false;
ImVec4 g_ESP_HatchColor = {0.0f, 0.8f, 0.4f, 1.0f};

bool g_ESP_TrapEnabled = false;
ImVec4 g_ESP_TrapColor = {0.8f, 0.2f, 0.0f, 1.0f};

bool g_ESP_BreakableDoorEnabled = false;
bool g_ESP_BreakableDoorRainbowEnabled = false;
ImVec4 g_ESP_BreakableDoorColor = {0.6f, 0.6f, 0.6f, 1.0f};

bool g_ESP_EscapeDoorEnabled = false;
bool g_ESP_EscapeDoorRainbowEnabled = false;
ImVec4 g_ESP_EscapeDoorColor = {0.0f, 0.6f, 1.0f, 1.0f};

bool g_ESP_ChestEnabled = false;
ImVec4 g_ESP_ChestColor = {1.0f, 0.8f, 0.0f, 1.0f};

bool g_ESP_CenobiteBoxEnabled = false;
bool g_ESP_CenobiteBoxRainbowEnabled = false;
ImVec4 g_ESP_CenobiteBoxColor = {0.7f, 0.2f, 1.0f, 1.0f};

bool g_ESP_NemesisZombieEnabled = false;
bool g_ESP_NemesisZombieRainbowEnabled = false;
ImVec4 g_ESP_NemesisZombieColor = {0.2f, 0.9f, 0.45f, 1.0f};

bool g_ESP_WindowEnabled = false;
ImVec4 g_ESP_WindowColor = {0.5f, 0.8f, 0.9f, 1.0f};

bool g_ESP_OutlineTextEnabled = false;
ImVec4 g_ESP_OutlineTextColor = {0.0f, 0.0f, 0.0f, 1.0f};

bool g_ESP_MatchInfoHUDEnabled = true;

bool g_Aura_Enabled = false;
bool g_Aura_SurvivorEnabled = false;
bool g_Aura_SurvivorRainbowEnabled = false;
ImVec4 g_Aura_SurvivorColor = {0.2f, 0.8f, 0.2f, 1.0f};

bool g_Aura_KillerEnabled = false;
bool g_Aura_KillerRainbowEnabled = false;
ImVec4 g_Aura_KillerColor = {0.9f, 0.1f, 0.1f, 1.0f};

bool g_Aura_GeneratorEnabled = false;
bool g_Aura_GeneratorRainbowEnabled = false;
ImVec4 g_Aura_GeneratorColor = {1.0f, 1.0f, 0.0f, 1.0f};

bool g_Aura_HookEnabled = false;
ImVec4 g_Aura_HookColor = {0.7f, 0.4f, 0.1f, 1.0f};

bool g_Aura_LockerEnabled = false;
bool g_Aura_LockerRainbowEnabled = false;
ImVec4 g_Aura_LockerColor = {0.4f, 0.4f, 0.8f, 1.0f};

bool g_Aura_TotemEnabled = false;
bool g_Aura_TotemRainbowEnabled = false;
ImVec4 g_Aura_TotemColor = {0.6f, 0.0f, 0.8f, 1.0f};

bool g_Aura_PalletEnabled = false;
bool g_Aura_PalletRainbowEnabled = false;
ImVec4 g_Aura_PalletColor = {0.8f, 0.6f, 0.0f, 1.0f};

bool g_Aura_HatchEnabled = false;
ImVec4 g_Aura_HatchColor = {0.0f, 0.8f, 0.4f, 1.0f};

bool g_Aura_TrapEnabled = false;
ImVec4 g_Aura_TrapColor = {0.8f, 0.2f, 0.0f, 1.0f};

bool g_Aura_BreakableDoorEnabled = false;
bool g_Aura_BreakableDoorRainbowEnabled = false;
ImVec4 g_Aura_BreakableDoorColor = {0.6f, 0.6f, 0.6f, 1.0f};

bool g_Aura_EscapeDoorEnabled = false;
bool g_Aura_EscapeDoorRainbowEnabled = false;
ImVec4 g_Aura_EscapeDoorColor = {0.0f, 0.6f, 1.0f, 1.0f};

bool g_Aura_ChestEnabled = false;
ImVec4 g_Aura_ChestColor = {1.0f, 0.8f, 0.0f, 1.0f};

bool g_Aura_WindowEnabled = false;
bool g_Aura_WindowRainbowEnabled = false;
ImVec4 g_Aura_WindowColor = {0.5f, 0.8f, 0.9f, 1.0f};

bool g_Aura_OtherEnabled = false;
ImVec4 g_Aura_OtherColor = {1.0f, 1.0f, 1.0f, 1.0f};

bool g_Cheats_Enabled = false;
bool g_Cheats_FOVEnabled = false;
float g_Cheats_FOV = 90.0f;
bool g_Cheats_KillerStainEnabled = false;
bool g_Cheats_KillerStainRainbowEnabled = false;
ImVec4 g_Cheats_KillerStainColor = {1.0f, 0.0f, 0.0f, 1.0f};
bool g_Cheats_RadarEnabled = false;
float g_Cheats_RadarZoom = 0.05f;
float g_Cheats_RadarRange = 3000.0f;

bool g_Cheats_AutoDeadHardEnabled = false;
float g_Cheats_DeadHardDistance = 4.0f;
bool g_Cheats_DeadHardPerfect = true;

bool g_Cheats_AutoSkillCheckEnabled = false;
float g_Cheats_AutoSkillCheckDelayMs = 12.0f;
bool g_Cheats_SkillCheckDebugEnabled = false;
bool g_Cheats_SpamSpacebar = false;

bool g_Cheats_AimbotEnabled = false;
int g_Cheats_AimbotBind = 0x02; // Right Mouse
bool g_Cheats_AimbotPredictionEnabled = false;
float g_Cheats_AimbotProjectileSpeed = 4000.0f; // Huntress hatchet
float g_Cheats_AimbotProjectileGravity = 980.0f;
float g_Cheats_AimbotFOV = 150.0f;
float g_Cheats_AimbotSmoothing = 0.0f;
float g_Cheats_AimbotDropScale = 1.0f;
FVector g_Cheats_AimbotOffset = {0, 0, 0};

bool g_Cheats_CrosshairEnabled = false;
int g_Cheats_CrosshairType = 0;
float g_Cheats_CrosshairSize = 10.0f;
float g_Cheats_CrosshairThickness = 2.0f;
ImVec4 g_Cheats_CrosshairColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

int g_FPS_Limit = 144;

bool g_InLobby = true;
bool g_AimbotKeyDown = false;
bool g_AimbotIsBinding = false;
