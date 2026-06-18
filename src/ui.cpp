#include "ui.h"
#include "config.h"
#include <imgui.h>
#include "globals.h"
#include "esp.h"
#include "memory.h"
#include "classes.h"
#include "name_mapping.h"
#include <stdio.h>

void InitializeUI() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding  = 6.0f;
    s.FrameRounding   = 3.0f;
    s.ScrollbarRounding = 3.0f;
    s.GrabRounding    = 3.0f;
    s.WindowBorderSize = 1.0f;
    s.FrameBorderSize  = 0.0f;
    s.WindowPadding   = {10, 10};
    s.FramePadding    = {6, 3};
    s.ItemSpacing     = {6, 4};

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]        = {0.08f, 0.08f, 0.10f, 0.92f};
    c[ImGuiCol_TitleBg]         = {0.06f, 0.06f, 0.08f, 1.0f};
    c[ImGuiCol_TitleBgActive]   = {0.12f, 0.12f, 0.18f, 1.0f};
    c[ImGuiCol_Header]          = {0.15f, 0.15f, 0.22f, 1.0f};
    c[ImGuiCol_HeaderHovered]   = {0.20f, 0.20f, 0.30f, 1.0f};
    c[ImGuiCol_HeaderActive]    = {0.25f, 0.25f, 0.38f, 1.0f};
    c[ImGuiCol_Tab]             = {0.10f, 0.10f, 0.15f, 1.0f};
    c[ImGuiCol_TabHovered]      = {0.22f, 0.22f, 0.32f, 1.0f};
    c[ImGuiCol_TabActive]       = {0.18f, 0.18f, 0.28f, 1.0f};
    c[ImGuiCol_FrameBg]         = {0.12f, 0.12f, 0.18f, 1.0f};
    c[ImGuiCol_FrameBgHovered]  = {0.18f, 0.18f, 0.26f, 1.0f};
    c[ImGuiCol_FrameBgActive]   = {0.22f, 0.22f, 0.32f, 1.0f};
    c[ImGuiCol_CheckMark]       = {0.40f, 0.80f, 0.40f, 1.0f};
    c[ImGuiCol_SliderGrab]      = {0.40f, 0.60f, 0.90f, 1.0f};
    c[ImGuiCol_Button]          = {0.15f, 0.15f, 0.22f, 1.0f};
    c[ImGuiCol_ButtonHovered]   = {0.25f, 0.25f, 0.38f, 1.0f};
    c[ImGuiCol_ButtonActive]    = {0.30f, 0.30f, 0.45f, 1.0f};
    c[ImGuiCol_Text]            = {0.90f, 0.90f, 0.90f, 1.0f};
    c[ImGuiCol_Separator]       = {0.25f, 0.25f, 0.35f, 1.0f};
}


void RenderUI(bool isMenuOpen) {
    if (isMenuOpen) {
        if (g_UI_ShouldApplyConfigPos) {
            ImGui::SetNextWindowPos({g_UI_MainWindow.X, g_UI_MainWindow.Y}, ImGuiCond_Always);
            ImGui::SetNextWindowSize({g_UI_MainWindow.W, g_UI_MainWindow.H}, ImGuiCond_Always);
        } else {
            ImGui::SetNextWindowSize({550, 650}, ImGuiCond_FirstUseEver);
        }
        
        ImGui::Begin("DBD Tool", nullptr, ImGuiWindowFlags_NoCollapse);
        
        // Update state for saving
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        g_UI_MainWindow.X = pos.x; g_UI_MainWindow.Y = pos.y;
        g_UI_MainWindow.W = size.x; g_UI_MainWindow.H = size.y;

        if (ImGui::BeginTabBar("##tabs")) {

        // ===== ESP =====
        if (ImGui::BeginTabItem("ESP")) {
            ImGui::Checkbox("Enable ESP", &g_ESP_Enabled);
            ImGui::Separator();
            if (!g_ESP_Enabled) ImGui::BeginDisabled();

            if (ImGui::CollapsingHeader("Survivors")) {
                ImGui::Checkbox("Enabled##surv", &g_ESP_SurvivorEnabled);
                ImGui::Checkbox("Hide Real Names", &g_ESP_HideRealNamesEnabled);
                ImGui::Checkbox("Show Loadout Window", &g_ESP_LoadoutWindowEnabled);
                ImGui::ColorEdit4("Color##sc", (float*)&g_ESP_SurvivorColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine(); ImGui::Checkbox("Rainbow##sr", &g_ESP_SurvivorRainbowEnabled);
                ImGui::Checkbox("Box##sb", &g_ESP_SurvivorBoxEnabled);
                ImGui::SameLine(); ImGui::Combo("Style##sbx", &g_ESP_SurvivorBoxIndex, "Corner\0Full\0");
                ImGui::Checkbox("Skeleton##sskel", &g_ESP_SurvivorSkeletonEnabled);
                ImGui::Checkbox("Snapline##sl", &g_ESP_SurvivorLineEnabled);
                ImGui::SameLine(); ImGui::Combo("Origin##slo", &g_ESP_SurvivorLineIndex, "Bottom\0Center\0Top\0");
                ImGui::Checkbox("Directional Indicator##sd", &g_ESP_SurvivorDirectional);
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Killer")) {
                ImGui::Checkbox("Enabled##kill", &g_ESP_KillerEnabled);
                ImGui::ColorEdit4("Color##kc", (float*)&g_ESP_KillerColor, ImGuiColorEditFlags_NoInputs);
                ImGui::Checkbox("Box##kb", &g_ESP_KillerBoxEnabled);
                ImGui::SameLine(); ImGui::Combo("Style##kbx", &g_ESP_KillerBoxIndex, "Corner\0Full\0");
                ImGui::Checkbox("Skeleton##kskel", &g_ESP_KillerSkeletonEnabled);
                ImGui::Checkbox("Snapline##kl", &g_ESP_KillerLineEnabled);
                ImGui::SameLine(); ImGui::Combo("Origin##klo", &g_ESP_KillerLineIndex, "Bottom\0Center\0Top\0");
                ImGui::Checkbox("Directional Indicator", &g_ESP_KillerDirectional);
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Generators")) {
                ImGui::Checkbox("Enabled##gen", &g_ESP_GeneratorEnabled);
                ImGui::Checkbox("Hide Completed##genh", &g_ESP_GeneratorHideCompleted);
                ImGui::ColorEdit4("Color##gc", (float*)&g_ESP_GeneratorColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine(); ImGui::Checkbox("Rainbow##gr", &g_ESP_GeneratorRainbowEnabled);
            }
            if (ImGui::CollapsingHeader("Hooks"))     { ImGui::Checkbox("Enabled##hook", &g_ESP_HookEnabled);     ImGui::ColorEdit4("Color##hkc", (float*)&g_ESP_HookColor, ImGuiColorEditFlags_NoInputs); }
            if (ImGui::CollapsingHeader("Lockers"))   { ImGui::Checkbox("Enabled##lck", &g_ESP_LockerEnabled);    ImGui::ColorEdit4("Color##lkc", (float*)&g_ESP_LockerColor, ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##lkr", &g_ESP_LockerRainbowEnabled); }
            if (ImGui::CollapsingHeader("Totems"))    { ImGui::Checkbox("Enabled##tot", &g_ESP_TotemEnabled);     ImGui::ColorEdit4("Color##tc",  (float*)&g_ESP_TotemColor,  ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##tr", &g_ESP_TotemRainbowEnabled); }
            if (ImGui::CollapsingHeader("Pallets"))   { ImGui::Checkbox("Enabled##pal", &g_ESP_PalletEnabled);    ImGui::ColorEdit4("Color##pc",  (float*)&g_ESP_PalletColor,  ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##pr", &g_ESP_PalletRainbowEnabled); }
            if (ImGui::CollapsingHeader("Hatch"))     { ImGui::Checkbox("Enabled##hat", &g_ESP_HatchEnabled);     ImGui::ColorEdit4("Color##hac", (float*)&g_ESP_HatchColor,   ImGuiColorEditFlags_NoInputs); }
            if (ImGui::CollapsingHeader("Traps"))     { ImGui::Checkbox("Enabled##trp", &g_ESP_TrapEnabled);      ImGui::ColorEdit4("Color##trc", (float*)&g_ESP_TrapColor,    ImGuiColorEditFlags_NoInputs); }
            if (ImGui::CollapsingHeader("Escape Doors")) { ImGui::Checkbox("Enabled##esc", &g_ESP_EscapeDoorEnabled); ImGui::ColorEdit4("Color##ec", (float*)&g_ESP_EscapeDoorColor, ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##er", &g_ESP_EscapeDoorRainbowEnabled); }
            if (ImGui::CollapsingHeader("Breakable Doors")) { ImGui::Checkbox("Enabled##brk", &g_ESP_BreakableDoorEnabled); ImGui::ColorEdit4("Color##bc", (float*)&g_ESP_BreakableDoorColor, ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##br", &g_ESP_BreakableDoorRainbowEnabled); }
            if (ImGui::CollapsingHeader("Chests"))    { ImGui::Checkbox("Enabled##chst", &g_ESP_ChestEnabled);    ImGui::ColorEdit4("Color##cc", (float*)&g_ESP_ChestColor,    ImGuiColorEditFlags_NoInputs); }
            if (ImGui::CollapsingHeader("Outline Text")) {
                ImGui::Checkbox("Enabled##ot", &g_ESP_OutlineTextEnabled);
                ImGui::ColorEdit4("Color##otc", (float*)&g_ESP_OutlineTextColor, ImGuiColorEditFlags_NoInputs);
            }

            if (!g_ESP_Enabled) ImGui::EndDisabled();
            ImGui::EndTabItem();
        }

        // ===== Killer Entities =====
        if (ImGui::BeginTabItem("Killer Entities")) {
            ImGui::Text("Global ESP Distance");
            ImGui::SliderFloat("##maxdist", &g_ESP_MaxDistance, 10.0f, 2000.0f, "%.0f m");
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Nemesis Zombies")) {
                ImGui::Checkbox("Enabled##zomb", &g_ESP_NemesisZombieEnabled);
                ImGui::ColorEdit4("Color##zombc", (float*)&g_ESP_NemesisZombieColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine(); ImGui::Checkbox("Rainbow##zombr", &g_ESP_NemesisZombieRainbowEnabled);
            }
            if (ImGui::CollapsingHeader("Victor (Twins)")) {
                ImGui::Checkbox("Enabled##victor", &g_ESP_Victor);
            }
            if (ImGui::CollapsingHeader("Hag Traps")) {
                ImGui::Checkbox("Enabled##hag", &g_ESP_HagTraps);
            }
            if (ImGui::CollapsingHeader("Plague Fountains")) {
                ImGui::Checkbox("Enabled##plague", &g_ESP_PlagueFountains);
            }
            if (ImGui::CollapsingHeader("Supply Boxes (Nemesis/Wesker)")) {
                ImGui::Checkbox("Enabled##supply", &g_ESP_SupplyCrates);
            }
            if (ImGui::CollapsingHeader("Xenomorph Tunnels")) {
                ImGui::Checkbox("Enabled##tunnel", &g_ESP_K33Tunnels);
            }
            if (ImGui::CollapsingHeader("Xenomorph Turrets")) {
                ImGui::Checkbox("Enabled##turret", &g_ESP_K33Turrets);
            }
            if (ImGui::CollapsingHeader("Cenobite Box")) {
                ImGui::Checkbox("Enabled##cbox", &g_ESP_CenobiteBoxEnabled);
                ImGui::ColorEdit4("Color##cboxc", (float*)&g_ESP_CenobiteBoxColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine(); ImGui::Checkbox("Rainbow##cboxr", &g_ESP_CenobiteBoxRainbowEnabled);
            }
            ImGui::EndTabItem();
        }

        // ===== Aura =====
#if 0
        if (ImGui::BeginTabItem("Aura")) {
            ImGui::Checkbox("Enable Aura", &g_Aura_Enabled);
            ImGui::Separator();
            if (!g_Aura_Enabled) ImGui::BeginDisabled();

            if (ImGui::CollapsingHeader("Survivors##a"))    { ImGui::Checkbox("Enabled##as", &g_Aura_SurvivorEnabled);    ImGui::ColorEdit4("Color##asc", (float*)&g_Aura_SurvivorColor,    ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##asr", &g_Aura_SurvivorRainbowEnabled); }
            if (ImGui::CollapsingHeader("Killer##a"))       { ImGui::Checkbox("Enabled##ak", &g_Aura_KillerEnabled);      ImGui::ColorEdit4("Color##akc", (float*)&g_Aura_KillerColor,      ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##akr", &g_Aura_KillerRainbowEnabled); }
            if (ImGui::CollapsingHeader("Generators##a"))   { ImGui::Checkbox("Enabled##ag", &g_Aura_GeneratorEnabled);   ImGui::ColorEdit4("Color##agc", (float*)&g_Aura_GeneratorColor,   ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##agr", &g_Aura_GeneratorRainbowEnabled); }
            if (ImGui::CollapsingHeader("Hooks##a"))        { ImGui::Checkbox("Enabled##ah", &g_Aura_HookEnabled);        ImGui::ColorEdit4("Color##ahc", (float*)&g_Aura_HookColor,        ImGuiColorEditFlags_NoInputs); }
            if (ImGui::CollapsingHeader("Lockers##a"))      { ImGui::Checkbox("Enabled##al", &g_Aura_LockerEnabled);      ImGui::ColorEdit4("Color##alc", (float*)&g_Aura_LockerColor,      ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##alr", &g_Aura_LockerRainbowEnabled); }
            if (ImGui::CollapsingHeader("Totems##a"))       { ImGui::Checkbox("Enabled##at", &g_Aura_TotemEnabled);       ImGui::ColorEdit4("Color##atc", (float*)&g_Aura_TotemColor,       ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##atr", &g_Aura_TotemRainbowEnabled); }
            if (ImGui::CollapsingHeader("Pallets##a"))      { ImGui::Checkbox("Enabled##ap", &g_Aura_PalletEnabled);      ImGui::ColorEdit4("Color##apc", (float*)&g_Aura_PalletColor,      ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##apr", &g_Aura_PalletRainbowEnabled); }
            if (ImGui::CollapsingHeader("Hatch##a"))        { ImGui::Checkbox("Enabled##aha", &g_Aura_HatchEnabled);      ImGui::ColorEdit4("Color##ahac",(float*)&g_Aura_HatchColor,       ImGuiColorEditFlags_NoInputs); }
            if (ImGui::CollapsingHeader("Traps##a"))        { ImGui::Checkbox("Enabled##atr2",&g_Aura_TrapEnabled);       ImGui::ColorEdit4("Color##atrc",(float*)&g_Aura_TrapColor,        ImGuiColorEditFlags_NoInputs); }
            if (ImGui::CollapsingHeader("Escape Doors##a")) { ImGui::Checkbox("Enabled##ae", &g_Aura_EscapeDoorEnabled);  ImGui::ColorEdit4("Color##aec", (float*)&g_Aura_EscapeDoorColor,  ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##aer", &g_Aura_EscapeDoorRainbowEnabled); }
            if (ImGui::CollapsingHeader("Breakable Doors##a")) { ImGui::Checkbox("Enabled##ab", &g_Aura_BreakableDoorEnabled); ImGui::ColorEdit4("Color##abc",(float*)&g_Aura_BreakableDoorColor,ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##abr", &g_Aura_BreakableDoorRainbowEnabled); }
            if (ImGui::CollapsingHeader("Chests##a"))       { ImGui::Checkbox("Enabled##ach",&g_Aura_ChestEnabled);       ImGui::ColorEdit4("Color##acc", (float*)&g_Aura_ChestColor,       ImGuiColorEditFlags_NoInputs); }
            if (ImGui::CollapsingHeader("Windows##a"))      { ImGui::Checkbox("Enabled##aw", &g_Aura_WindowEnabled);      ImGui::ColorEdit4("Color##awc", (float*)&g_Aura_WindowColor,      ImGuiColorEditFlags_NoInputs); ImGui::SameLine(); ImGui::Checkbox("Rainbow##awr", &g_Aura_WindowRainbowEnabled); }
            if (ImGui::CollapsingHeader("Other##a"))        { ImGui::Checkbox("Enabled##ao", &g_Aura_OtherEnabled);       ImGui::ColorEdit4("Color##aoc", (float*)&g_Aura_OtherColor,       ImGuiColorEditFlags_NoInputs); }

            if (!g_Aura_Enabled) ImGui::EndDisabled();
            ImGui::EndTabItem();
        }
#endif

        // ===== Cheats =====
        if (ImGui::BeginTabItem("Cheats")) {
            ImGui::Checkbox("Enable Cheats", &g_Cheats_Enabled);
            ImGui::Separator();
            if (!g_Cheats_Enabled) ImGui::BeginDisabled();

            ImGui::Checkbox("FOV Changer", &g_Cheats_FOVEnabled);
            if (g_Cheats_FOVEnabled) {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(150);
                ImGui::SliderFloat("##fovv", &g_Cheats_FOV, 70.0f, 150.0f, "%.0f");
            }
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Custom Crosshair")) {
                ImGui::Checkbox("Enable Crosshair", &g_Cheats_CrosshairEnabled);
                if (g_Cheats_CrosshairEnabled) {
                    ImGui::Combo("Type", &g_Cheats_CrosshairType, "Cross\0Dot\0Circle\0\0");
                    ImGui::SliderFloat("Size", &g_Cheats_CrosshairSize, 1.0f, 50.0f, "%.1f px");
                    ImGui::SliderFloat("Thickness", &g_Cheats_CrosshairThickness, 1.0f, 10.0f, "%.1f px");
                    ImGui::ColorEdit4("Color", (float*)&g_Cheats_CrosshairColor, ImGuiColorEditFlags_NoInputs);
                }
            }

            if (ImGui::CollapsingHeader("Killer Stain")) {
                ImGui::Checkbox("Enabled##ks", &g_Cheats_KillerStainEnabled);
                ImGui::ColorEdit4("Color##ksc", (float*)&g_Cheats_KillerStainColor, ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine(); ImGui::Checkbox("Rainbow##ksr", &g_Cheats_KillerStainRainbowEnabled);
            }

            ImGui::Checkbox("Auto-Hit Great Skillchecks", &g_Cheats_AutoSkillCheckEnabled);
            ImGui::SetNextItemWidth(150);
            ImGui::SliderFloat("Skillcheck Hit Delay", &g_Cheats_AutoSkillCheckDelayMs, 0.0f, 40.0f, "%.0f ms");
            ImGui::Checkbox("Skillcheck / Dead Hard Debug", &g_Cheats_SkillCheckDebugEnabled);
            ImGui::Checkbox("Spam Spacebar Simulation", &g_Cheats_SpamSpacebar);
            ImGui::Separator();
            ImGui::Checkbox("Auto Dead Hard", &g_Cheats_AutoDeadHardEnabled);
            if (g_Cheats_AutoDeadHardEnabled) {
                ImGui::Checkbox("Perfect Trigger (Wait for Lunge)", &g_Cheats_DeadHardPerfect);
                ImGui::SliderFloat("Trigger Distance", &g_Cheats_DeadHardDistance, 1.0f, 6.0f, "%.1f m");
                ImGui::TextDisabled("Automatically presses 'E' when Killer swings nearby.");
            }
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Radar")) {
                ImGui::Checkbox("Enable Radar", &g_Cheats_RadarEnabled);
                if (g_Cheats_RadarEnabled) {
                    ImGui::SliderFloat("Zoom", &g_Cheats_RadarZoom, 0.01f, 0.2f, "%.3f");
                    ImGui::SliderFloat("Range", &g_Cheats_RadarRange, 500.0f, 10000.0f, "%.0f m");
                    ImGui::TextDisabled("Mini-map showing players and generators.");
                }
            }
            ImGui::Separator();
            ImGui::Checkbox("Aimbot", &g_Cheats_AimbotEnabled);
            if (g_Cheats_AimbotEnabled) {
                ImGui::SameLine();
                std::string btnText = g_AimbotIsBinding ? "Press any key..." : GetBindName(g_Cheats_AimbotBind);
                if (ImGui::Button(btnText.c_str(), ImVec2(120, 0))) {
                    g_AimbotIsBinding = true;
                }
                ImGui::Checkbox("Advanced Calculations (Prediction/Drop)", &g_Cheats_AimbotPredictionEnabled);
                if (g_Cheats_AimbotPredictionEnabled) {
                    ImGui::SliderFloat("Projectile Speed", &g_Cheats_AimbotProjectileSpeed, 500.0f, 10000.0f, "%.0f cm/s");
                    ImGui::SliderFloat("Projectile Gravity", &g_Cheats_AimbotProjectileGravity, 0.0f, 2000.0f, "%.0f cm/s^2");
                    ImGui::SliderFloat("Drop Scale", &g_Cheats_AimbotDropScale, 0.0f, 5.0f, "%.1fx");
                    ImGui::TextDisabled("Adjust 'Drop Scale' if aiming too high/low at distance.");
                    ImGui::TextDisabled("Huntress Fully Charged: 4000 cm/s");
                    ImGui::TextDisabled("Huntress Uncharged: 2500 cm/s");
                    ImGui::TextDisabled("Slinger: 4000 cm/s");
                }
                ImGui::Separator();
                ImGui::SliderFloat("Aimbot FOV", &g_Cheats_AimbotFOV, 10.0f, 1000.0f, "%.0f px");
                static double offsetMin = -100.0, offsetMax = 100.0;
                if (ImGui::TreeNode("Manual Target Offset")) {
                    ImGui::SliderScalar("Offset X", ImGuiDataType_Double, &g_Cheats_AimbotOffset.X, &offsetMin, &offsetMax, "%.1f");
                    ImGui::SliderScalar("Offset Y", ImGuiDataType_Double, &g_Cheats_AimbotOffset.Y, &offsetMin, &offsetMax, "%.1f");
                    ImGui::SliderScalar("Offset Z", ImGuiDataType_Double, &g_Cheats_AimbotOffset.Z, &offsetMin, &offsetMax, "%.1f");
                    if (ImGui::Button("Reset Offset")) g_Cheats_AimbotOffset = {0,0,0};
                    ImGui::TreePop();
                }
                ImGui::SliderFloat("Smoothing", &g_Cheats_AimbotSmoothing, 0.0f, 0.99f, "%.2f");
                ImGui::TextDisabled("Aims at the target's head while the bind is held.");
            }

            if (!g_Cheats_Enabled) ImGui::EndDisabled();
            ImGui::EndTabItem();
        }

        // ===== Settings =====
        if (ImGui::BeginTabItem("Settings")) {
            ImGui::SliderInt("FPS Limit", &g_FPS_Limit, 30, 360);
            ImGui::Separator();
            ImGui::Checkbox("Show Tool Title & PID", &g_UI_ShowToolInfo);
            ImGui::Checkbox("Show Actor Counts (S/K/G/P)", &g_UI_ShowActorCounts);
            ImGui::Separator();
            ImGui::TextDisabled("Press HOME to toggle menu / passthrough");
            ImGui::TextDisabled("Press END to exit");
            ImGui::EndTabItem();
        }

        // ===== Configs =====
        if (ImGui::BeginTabItem("Configs")) {
            static char newConfigName[64] = "";
            static std::string selectedConfig = "";
            static std::string defaultConfig = Config::GetDefault();
            auto configs = Config::List();

            ImGui::InputText("New Config Name", newConfigName, 64);
            if (ImGui::Button("Save New", ImVec2(120, 0))) {
                if (strlen(newConfigName) > 0) {
                    Config::Save(newConfigName);
                    newConfigName[0] = '\0';
                }
            }

            ImGui::Separator();
            ImGui::Text("Saved Configurations:");
            
            if (ImGui::BeginChild("ConfigList", ImVec2(0, 150), true)) {
                for (const auto& conf : configs) {
                    bool isSelected = (selectedConfig == conf);
                    if (ImGui::Selectable(conf.c_str(), isSelected)) {
                        selectedConfig = conf;
                    }
                    if (conf == defaultConfig) {
                        ImGui::SameLine(ImGui::GetWindowWidth() - 70);
                        ImGui::TextDisabled("[Def]");
                    }
                }
                ImGui::EndChild();
            }

            if (!selectedConfig.empty()) {
                if (ImGui::Button("Load", ImVec2(60, 0))) {
                    Config::Load(selectedConfig);
                }
                ImGui::SameLine();
                if (ImGui::Button("Save", ImVec2(60, 0))) {
                    Config::Save(selectedConfig);
                }
                ImGui::SameLine();
                if (ImGui::Button("Default", ImVec2(70, 0))) {
                    Config::SetDefault(selectedConfig);
                    defaultConfig = selectedConfig;
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete", ImVec2(60, 0))) {
                    Config::Delete(selectedConfig);
                    selectedConfig = "";
                }
            }

            ImGui::EndTabItem();
        }

        // ===== Debug =====
        if (ImGui::BeginTabItem("Debug")) {
            if (Memory::IsAttached()) {
                ImGui::Text("Process: DeadByDaylight-Win64-Shipping.exe");
                ImGui::Text("PID: %d", Memory::GetProcessId());
                ImGui::Text("Base Address: 0x%lx", (unsigned long)Memory::GetBaseAddress());
                ImGui::Separator();
                ImGui::Text("Resolved Offsets (RVA):");
                ImGui::Text("GWorld:   0x%lx", (unsigned long)GOffsets.GWorldOffset);
                ImGui::Text("GNames:   0x%lx", (unsigned long)GOffsets.GNameOffset);
                ImGui::Text("GObjects: 0x%lx", (unsigned long)GOffsets.GObjectsOffset);
                
                ImGui::Separator();
                ImGui::Text("Live World Info:");
                uintptr_t base = Memory::GetBaseAddress();
                uintptr_t uworld = Memory::Read<uintptr_t>(base + GOffsets.GWorldOffset);
                ImGui::Text("UWorld: 0x%lx", (unsigned long)uworld);
                if (uworld) {
                    uintptr_t level = Memory::Read<uintptr_t>(uworld + GOffsets.UWorld_PersistentLevelOffset);
                    ImGui::Text("PersistentLevel: 0x%lx", (unsigned long)level);
                    uintptr_t gState = Memory::Read<uintptr_t>(uworld + GOffsets.UWorld_GameStateOffset);
                    ImGui::Text("GameState: 0x%lx", (unsigned long)gState);
                }

                ImGui::Spacing();
                if (ImGui::Button("Manual Re-Scan Signatures")) {
                    ResolveSignatures();
                }
            } else {
                ImGui::TextColored({1,0,0,1}, "Process not attached.");
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    }

    if (g_Cheats_SkillCheckDebugEnabled) {
        if (g_UI_ShouldApplyConfigPos) {
            ImGui::SetNextWindowPos({g_UI_SkillcheckDebug.X, g_UI_SkillcheckDebug.Y}, ImGuiCond_Always);
            ImGui::SetNextWindowSize({g_UI_SkillcheckDebug.W, g_UI_SkillcheckDebug.H}, ImGuiCond_Always);
        }
        ImGui::Begin("Skillcheck Debug", &g_Cheats_SkillCheckDebugEnabled);
        
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        g_UI_SkillcheckDebug.X = pos.x; g_UI_SkillcheckDebug.Y = pos.y;
        g_UI_SkillcheckDebug.W = size.x; g_UI_SkillcheckDebug.H = size.y;
        g_UI_SkillcheckDebug.Opened = g_Cheats_SkillCheckDebugEnabled;
        if (g_LocalController) {
            uintptr_t localPawn = Memory::Read<uintptr_t>(g_LocalController + GOffsets.APlayerController_AcknowledgedPawnOffset);
            if (localPawn) {
                uintptr_t handler = Memory::Read<uintptr_t>(localPawn + GOffsets.ADBDPlayer_interactionHandler);
                if (handler) {
                    uintptr_t skillCheck = Memory::Read<uintptr_t>(handler + GOffsets.UPlayerInteractionHandler_skillCheck);
                    if (skillCheck) {
                        uintptr_t currentInteraction = Memory::Read<uintptr_t>(skillCheck + 0x1D0);

                        ImGui::Text("Active: %s", currentInteraction ? "YES" : "NO");
                        
                        // --- Manual Overrides ---
                        static int manualProgressOffset = -1;
                        static int manualDefOffset = -1;
                        if (manualProgressOffset == -1) manualProgressOffset = GOffsets.USkillCheck_currentProgress;
                        if (manualDefOffset == -1) manualDefOffset = GOffsets.USkillCheck_skillCheckDefinition;

                        ImGui::InputInt("Needle Offset", &manualProgressOffset, 4, 16, ImGuiInputTextFlags_CharsHexadecimal);
                        ImGui::InputInt("Def Offset", &manualDefOffset, 4, 16, ImGuiInputTextFlags_CharsHexadecimal);
                        
                        // Apply overrides to GOffsets (affects both UI and ESP thread)
                        GOffsets.USkillCheck_currentProgress = manualProgressOffset;
                        GOffsets.USkillCheck_skillCheckDefinition = manualDefOffset;

                        float progress = Memory::Read<float>(skillCheck + GOffsets.USkillCheck_currentProgress);
                        FSkillCheckDefinition def = Memory::Read<FSkillCheckDefinition>(skillCheck + GOffsets.USkillCheck_skillCheckDefinition);

                        ImGui::Text("Header Progress: %.4f", progress);
                        ImGui::Text("Header Bonus Start: %.4f", def.BonusZoneStart);
                        ImGui::Text("Header Success Start: %.4f", def.SuccessZoneStart);
                        ImGui::Text("Header Success End: %.4f", def.SuccessZoneEnd);

                        ImGui::Separator();
                        ImGui::Text("Value Searcher (Find Offset by Value):");
                        static float searchVal = 0.5995f;
                        ImGui::InputFloat("Search Value", &searchVal, 0.0001f, 0.01f, "%.4f");
                        for (int i = 0x80; i < 0x500; i += 4) {
                            float val = Memory::Read<float>(skillCheck + i);
                            if (fabs(val - searchVal) < 0.001f) {
                                ImGui::TextColored(ImVec4(0,1,0,1), "MATCH FOUND at Offset 0x%X: %.4f", i, val);
                            }
                        }

                        ImGui::Separator();
                        ImGui::Text("Float Scanner (0x100 - 0x400):");
                        for (int i = 0x100; i < 0x400; i += 4) {
                            float val = Memory::Read<float>(skillCheck + i);
                            if (val != 0.0f) {
                                ImGui::Text("Offset 0x%X: %.4f", i, val);
                            }
                        }
                    } else { ImGui::Text("SkillCheck Component Null"); }
                } else { ImGui::Text("Interaction Handler Null"); }
            } else { ImGui::Text("Local Pawn Null"); }
        } else { ImGui::Text("Local Controller Null"); }
        ImGui::End();
    }

    // --- Loadout Window ---
    if (g_ESP_LoadoutWindowEnabled) {
        if (g_UI_ShouldApplyConfigPos) {
            ImGui::SetNextWindowPos({g_UI_LoadoutWindow.X, g_UI_LoadoutWindow.Y}, ImGuiCond_Always);
            ImGui::SetNextWindowSize({g_UI_LoadoutWindow.W, g_UI_LoadoutWindow.H}, ImGuiCond_Always);
        } else {
            ImGui::SetNextWindowSize({350, 400}, ImGuiCond_FirstUseEver);
        }
        if (ImGui::Begin("Player Loadouts", &g_ESP_LoadoutWindowEnabled)) {
            ImVec2 pos = ImGui::GetWindowPos();
            ImVec2 size = ImGui::GetWindowSize();
            g_UI_LoadoutWindow.X = pos.x; g_UI_LoadoutWindow.Y = pos.y;
            g_UI_LoadoutWindow.W = size.x; g_UI_LoadoutWindow.H = size.y;
            g_UI_LoadoutWindow.Opened = g_ESP_LoadoutWindowEnabled;
            std::lock_guard<std::mutex> lk(g_ActorMutex);

            auto DrawLoadout = [](const PlayerLoadout& pl) {
                ImVec4 roleCol = (pl.role == 1) ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f) : ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
                ImGui::TextColored(roleCol, "[P%d | Lvl %d] %s", 
                    pl.prestige,
                    pl.characterLevel,
                    pl.name.empty() ? "Unknown" : pl.name.c_str());
                ImGui::TextDisabled("Platform: %s | Provider: %s | Crossplay: %s", 
                    pl.platform.c_str(), 
                    pl.provider.c_str(), 
                    pl.crossplayAllowed ? "Yes" : "No");
                ImGui::TextDisabled("Ping: %d ms | Pips: %d | %s", 
                    pl.ping, 
                    (pl.role == 1) ? pl.killerPips : pl.survivorPips, 
                    pl.characterName.empty() ? (pl.role == 1 ? "Killer" : "Survivor") : pl.characterName.c_str());

                if (!pl.item.empty()) {
                    std::string itemName = GetFriendlyName(pl.item, 1);
                    ImGui::Text("  Item/Power: %s", itemName.c_str());
                }
                if (!pl.offering.empty()) {
                    std::string offName = GetFriendlyName(pl.offering, 4);
                    ImGui::Text("  Offering: %s", offName.c_str());
                }
                if (!pl.addons.empty()) {
                    ImGui::Text("  Addons:");
                    for (const auto& addon : pl.addons) {
                        std::string addonName = GetFriendlyName(addon, 2);
                        ImGui::Text("   - %s", addonName.c_str());
                    }
                }
                if (!pl.perks.empty()) {
                    ImGui::Text("  Perks:");
                    for (const auto& perk : pl.perks) {
                        std::string perkName = GetFriendlyName(perk, 0);
                        ImGui::Text("   - %s", perkName.c_str());
                    }
                }
                ImGui::Separator();
            };

            bool foundKiller = false;
            for (const auto& pl : g_PlayerLoadouts) {
                if (pl.role == 1) {
                    if (!foundKiller) ImGui::TextColored({1,0,0,1}, "[ KILLER ]");
                    DrawLoadout(pl);
                    foundKiller = true;
                }
            }

            bool foundSurvivor = false;
            for (const auto& pl : g_PlayerLoadouts) {
                if (pl.role == 2) {
                    if (!foundSurvivor) {
                        ImGui::Spacing();
                        ImGui::TextColored({0,1,0,1}, "[ SURVIVORS ]");
                    }
                    DrawLoadout(pl);
                    foundSurvivor = true;
                }
            }

            if (!foundKiller && !foundSurvivor) {
                ImGui::TextDisabled("No loadouts available or waiting for match...");
            }
        }
        ImGui::End();
    }

    g_UI_ShouldApplyConfigPos = false;
}
