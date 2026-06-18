#include "config.h"
#include "globals.h"
#include "esp.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace Config {
    std::string currentConfig = "default";

    static std::string GetPath(const std::string& name) {
        try {
            if (!fs::exists("configs")) {
                fs::create_directories("configs");
            }
        } catch (...) {}

        std::string path = name;
        if (path.empty()) path = "default";
        
        if (path.find("configs/") == 0) {
            path = path.substr(8);
        }
        if (path.size() > 5 && path.substr(path.size() - 5) == ".json") {
            path = path.substr(0, path.size() - 5);
        }

        return "configs/" + path + ".json";
    }

    void Save(const std::string& filename) {
        std::string name = filename.empty() ? currentConfig : filename;
        std::string path = GetPath(name);
        
        json j;

        auto vec4_to_json = [](const ImVec4& v) {
            return json::array({v.x, v.y, v.z, v.w});
        };
        auto fvec_to_json = [](const FVector& v) {
            return json::array({v.X, v.Y, v.Z});
        };
        auto win_to_json = [](const WindowState& w) {
            return json::object({{"X", w.X}, {"Y", w.Y}, {"W", w.W}, {"H", w.H}, {"Opened", w.Opened}});
        };

        // UI Windows
        auto& ui = j["UI"];
        ui["MainWindow"] = win_to_json(g_UI_MainWindow);
        ui["SkillcheckDebug"] = win_to_json(g_UI_SkillcheckDebug);
        ui["LoadoutWindow"] = win_to_json(g_UI_LoadoutWindow);
        ui["ShowToolInfo"] = g_UI_ShowToolInfo;
        ui["ShowActorCounts"] = g_UI_ShowActorCounts;

        // ESP
        auto& esp = j["ESP"];
        esp["Enabled"] = g_ESP_Enabled;
        esp["SurvivorEnabled"] = g_ESP_SurvivorEnabled;
        esp["SurvivorRainbow"] = g_ESP_SurvivorRainbowEnabled;
        esp["SurvivorColor"] = vec4_to_json(g_ESP_SurvivorColor);
        esp["SurvivorBox"] = g_ESP_SurvivorBoxEnabled;
        esp["SurvivorBoxIndex"] = g_ESP_SurvivorBoxIndex;
        esp["SurvivorLine"] = g_ESP_SurvivorLineEnabled;
        esp["SurvivorLineIndex"] = g_ESP_SurvivorLineIndex;
        esp["SurvivorSkeleton"] = g_ESP_SurvivorSkeletonEnabled;
        esp["HideRealNames"] = g_ESP_HideRealNamesEnabled;
        esp["LoadoutWindow"] = g_ESP_LoadoutWindowEnabled;
        esp["MatchInfoHUD"] = g_ESP_MatchInfoHUDEnabled;

        esp["KillerEnabled"] = g_ESP_KillerEnabled;
        esp["KillerColor"] = vec4_to_json(g_ESP_KillerColor);
        esp["KillerBox"] = g_ESP_KillerBoxEnabled;
        esp["KillerBoxIndex"] = g_ESP_KillerBoxIndex;
        esp["KillerLine"] = g_ESP_KillerLineEnabled;
        esp["KillerLineIndex"] = g_ESP_KillerLineIndex;
        esp["KillerSkeleton"] = g_ESP_KillerSkeletonEnabled;
        esp["KillerDirectional"] = g_ESP_KillerDirectional;
        esp["SurvivorDirectional"] = g_ESP_SurvivorDirectional;
        esp["MaxDistance"] = g_ESP_MaxDistance;

        esp["GeneratorEnabled"] = g_ESP_GeneratorEnabled;
        esp["GeneratorRainbow"] = g_ESP_GeneratorRainbowEnabled;
        esp["GeneratorColor"] = vec4_to_json(g_ESP_GeneratorColor);
        esp["GeneratorHideCompleted"] = g_ESP_GeneratorHideCompleted;

        esp["HookEnabled"] = g_ESP_HookEnabled;
        esp["HookColor"] = vec4_to_json(g_ESP_HookColor);

        esp["LockerEnabled"] = g_ESP_LockerEnabled;
        esp["LockerRainbow"] = g_ESP_LockerRainbowEnabled;
        esp["LockerColor"] = vec4_to_json(g_ESP_LockerColor);

        esp["TotemEnabled"] = g_ESP_TotemEnabled;
        esp["TotemRainbow"] = g_ESP_TotemRainbowEnabled;
        esp["TotemColor"] = vec4_to_json(g_ESP_TotemColor);

        esp["PalletEnabled"] = g_ESP_PalletEnabled;
        esp["PalletRainbow"] = g_ESP_PalletRainbowEnabled;
        esp["PalletColor"] = vec4_to_json(g_ESP_PalletColor);

        esp["HatchEnabled"] = g_ESP_HatchEnabled;
        esp["HatchColor"] = vec4_to_json(g_ESP_HatchColor);

        esp["TrapEnabled"] = g_ESP_TrapEnabled;
        esp["TrapColor"] = vec4_to_json(g_ESP_TrapColor);

        esp["BreakableDoorEnabled"] = g_ESP_BreakableDoorEnabled;
        esp["BreakableDoorRainbow"] = g_ESP_BreakableDoorRainbowEnabled;
        esp["BreakableDoorColor"] = vec4_to_json(g_ESP_BreakableDoorColor);

        esp["EscapeDoorEnabled"] = g_ESP_EscapeDoorEnabled;
        esp["EscapeDoorRainbow"] = g_ESP_EscapeDoorRainbowEnabled;
        esp["EscapeDoorColor"] = vec4_to_json(g_ESP_EscapeDoorColor);

        esp["ChestEnabled"] = g_ESP_ChestEnabled;
        esp["ChestColor"] = vec4_to_json(g_ESP_ChestColor);

        esp["WindowEnabled"] = g_ESP_WindowEnabled;
        esp["WindowColor"] = vec4_to_json(g_ESP_WindowColor);

        esp["OutlineTextEnabled"] = g_ESP_OutlineTextEnabled;
        esp["OutlineTextColor"] = vec4_to_json(g_ESP_OutlineTextColor);

        esp["CenobiteBoxEnabled"] = g_ESP_CenobiteBoxEnabled;
        esp["CenobiteBoxRainbow"] = g_ESP_CenobiteBoxRainbowEnabled;
        esp["CenobiteBoxColor"] = vec4_to_json(g_ESP_CenobiteBoxColor);

        esp["NemesisZombieColor"] = vec4_to_json(g_ESP_NemesisZombieColor);

        esp["Victor"] = g_ESP_Victor;
        esp["HagTraps"] = g_ESP_HagTraps;
        esp["PlagueFountains"] = g_ESP_PlagueFountains;
        esp["SupplyCrates"] = g_ESP_SupplyCrates;
        esp["K33Tunnels"] = g_ESP_K33Tunnels;
        esp["K33Turrets"] = g_ESP_K33Turrets;

        // Aura
        auto& aura = j["Aura"];
        aura["Enabled"] = g_Aura_Enabled;
        aura["SurvivorEnabled"] = g_Aura_SurvivorEnabled;
        aura["SurvivorRainbow"] = g_Aura_SurvivorRainbowEnabled;
        aura["SurvivorColor"] = vec4_to_json(g_Aura_SurvivorColor);
        aura["KillerEnabled"] = g_Aura_KillerEnabled;
        aura["KillerRainbow"] = g_Aura_KillerRainbowEnabled;
        aura["KillerColor"] = vec4_to_json(g_Aura_KillerColor);
        aura["GeneratorEnabled"] = g_Aura_GeneratorEnabled;
        aura["GeneratorRainbow"] = g_Aura_GeneratorRainbowEnabled;
        aura["GeneratorColor"] = vec4_to_json(g_Aura_GeneratorColor);
        aura["HookEnabled"] = g_Aura_HookEnabled;
        aura["HookColor"] = vec4_to_json(g_Aura_HookColor);
        aura["LockerEnabled"] = g_Aura_LockerEnabled;
        aura["LockerRainbow"] = g_Aura_LockerRainbowEnabled;
        aura["LockerColor"] = vec4_to_json(g_Aura_LockerColor);
        aura["TotemEnabled"] = g_Aura_TotemEnabled;
        aura["TotemRainbow"] = g_Aura_TotemRainbowEnabled;
        aura["TotemColor"] = vec4_to_json(g_Aura_TotemColor);
        aura["PalletEnabled"] = g_Aura_PalletEnabled;
        aura["PalletRainbow"] = g_Aura_PalletRainbowEnabled;
        aura["PalletColor"] = vec4_to_json(g_Aura_PalletColor);
        aura["HatchEnabled"] = g_Aura_HatchEnabled;
        aura["HatchColor"] = vec4_to_json(g_Aura_HatchColor);
        aura["TrapEnabled"] = g_Aura_TrapEnabled;
        aura["TrapColor"] = vec4_to_json(g_Aura_TrapColor);
        aura["BreakableDoorEnabled"] = g_Aura_BreakableDoorEnabled;
        aura["BreakableDoorRainbow"] = g_Aura_BreakableDoorRainbowEnabled;
        aura["BreakableDoorColor"] = vec4_to_json(g_Aura_BreakableDoorColor);
        aura["EscapeDoorEnabled"] = g_Aura_EscapeDoorEnabled;
        aura["EscapeDoorRainbow"] = g_Aura_EscapeDoorRainbowEnabled;
        aura["EscapeDoorColor"] = vec4_to_json(g_Aura_EscapeDoorColor);
        aura["ChestEnabled"] = g_Aura_ChestEnabled;
        aura["ChestColor"] = vec4_to_json(g_Aura_ChestColor);
        aura["WindowEnabled"] = g_Aura_WindowEnabled;
        aura["WindowRainbow"] = g_Aura_WindowRainbowEnabled;
        aura["WindowColor"] = vec4_to_json(g_Aura_WindowColor);
        aura["OtherEnabled"] = g_Aura_OtherEnabled;
        aura["OtherColor"] = vec4_to_json(g_Aura_OtherColor);

        // Cheats
        auto& cheats = j["Cheats"];
        cheats["Enabled"] = g_Cheats_Enabled;
        cheats["FOVEnabled"] = g_Cheats_FOVEnabled;
        cheats["FOV"] = g_Cheats_FOV;
        cheats["KillerStain"] = g_Cheats_KillerStainEnabled;
        cheats["KillerStainRainbow"] = g_Cheats_KillerStainRainbowEnabled;
        cheats["KillerStainColor"] = vec4_to_json(g_Cheats_KillerStainColor);
        cheats["AutoDeadHard"] = g_Cheats_AutoDeadHardEnabled;
        cheats["DeadHardDistance"] = g_Cheats_DeadHardDistance;
        cheats["DeadHardPerfect"] = g_Cheats_DeadHardPerfect;
        cheats["AutoSkillCheck"] = g_Cheats_AutoSkillCheckEnabled;
        cheats["AutoSkillCheckDelayMs"] = g_Cheats_AutoSkillCheckDelayMs;
        cheats["SkillCheckDebug"] = g_Cheats_SkillCheckDebugEnabled;
        cheats["SpamSpacebar"] = g_Cheats_SpamSpacebar;
        cheats["RadarEnabled"] = g_Cheats_RadarEnabled;
        cheats["RadarZoom"] = g_Cheats_RadarZoom;
        cheats["RadarRange"] = g_Cheats_RadarRange;
        cheats["AimbotEnabled"] = g_Cheats_AimbotEnabled;
        cheats["AimbotBind"] = g_Cheats_AimbotBind;
        cheats["AimbotPrediction"] = g_Cheats_AimbotPredictionEnabled;
        cheats["AimbotProjectileSpeed"] = g_Cheats_AimbotProjectileSpeed;
        cheats["AimbotProjectileGravity"] = g_Cheats_AimbotProjectileGravity;
        cheats["AimbotFOV"] = g_Cheats_AimbotFOV;
        cheats["AimbotSmoothing"] = g_Cheats_AimbotSmoothing;
        cheats["AimbotDropScale"] = g_Cheats_AimbotDropScale;
        cheats["AimbotOffset"] = fvec_to_json(g_Cheats_AimbotOffset);

        cheats["CrosshairEnabled"] = g_Cheats_CrosshairEnabled;
        cheats["CrosshairType"] = g_Cheats_CrosshairType;
        cheats["CrosshairSize"] = g_Cheats_CrosshairSize;
        cheats["CrosshairThickness"] = g_Cheats_CrosshairThickness;
        cheats["CrosshairColor"] = vec4_to_json(g_Cheats_CrosshairColor);

        j["FPS_Limit"] = g_FPS_Limit;

        std::ofstream file(path);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        }
    }

    void Load(const std::string& filename) {
        std::string name = filename.empty() ? currentConfig : filename;
        std::string path = GetPath(name);
        std::ifstream file(path);
        if (!file.is_open()) return;

        try {
            json j;
            file >> j;

            auto get_vec4 = [](const json& val, ImVec4& out) {
                if (val.is_array() && val.size() == 4) {
                    out = ImVec4(val[0].get<float>(), val[1].get<float>(), val[2].get<float>(), val[3].get<float>());
                }
            };
            auto get_fvec = [](const json& val, FVector& out) {
                if (val.is_array() && val.size() == 3) {
                    out = FVector(val[0].get<double>(), val[1].get<double>(), val[2].get<double>());
                }
            };
            auto get_win = [](const json& val, WindowState& out) {
                if (val.is_object()) {
                    out.X = val.value("X", out.X);
                    out.Y = val.value("Y", out.Y);
                    out.W = val.value("W", out.W);
                    out.H = val.value("H", out.H);
                    out.Opened = val.value("Opened", out.Opened);
                }
            };

            // UI Windows
            if (j.contains("UI")) {
                auto& ui = j["UI"];
                get_win(ui["MainWindow"], g_UI_MainWindow);
                get_win(ui["SkillcheckDebug"], g_UI_SkillcheckDebug);
                get_win(ui["LoadoutWindow"], g_UI_LoadoutWindow);
                if (ui.contains("ShowToolInfo")) g_UI_ShowToolInfo = ui["ShowToolInfo"];
                if (ui.contains("ShowActorCounts")) g_UI_ShowActorCounts = ui["ShowActorCounts"];
                g_UI_ShouldApplyConfigPos = true;
            }

            // ESP
            if (j.contains("ESP")) {
                auto& esp = j["ESP"];
                if (esp.contains("Enabled")) g_ESP_Enabled = esp["Enabled"];
                if (esp.contains("SurvivorEnabled")) g_ESP_SurvivorEnabled = esp["SurvivorEnabled"];
                if (esp.contains("SurvivorRainbow")) g_ESP_SurvivorRainbowEnabled = esp["SurvivorRainbow"];
                if (esp.contains("SurvivorColor")) get_vec4(esp["SurvivorColor"], g_ESP_SurvivorColor);
                if (esp.contains("SurvivorBox")) g_ESP_SurvivorBoxEnabled = esp["SurvivorBox"];
                if (esp.contains("SurvivorBoxIndex")) g_ESP_SurvivorBoxIndex = esp["SurvivorBoxIndex"];
                if (esp.contains("SurvivorLine")) g_ESP_SurvivorLineEnabled = esp["SurvivorLine"];
                if (esp.contains("SurvivorLineIndex")) g_ESP_SurvivorLineIndex = esp["SurvivorLineIndex"];
                if (esp.contains("SurvivorSkeleton")) g_ESP_SurvivorSkeletonEnabled = esp["SurvivorSkeleton"];
                if (esp.contains("HideRealNames")) g_ESP_HideRealNamesEnabled = esp["HideRealNames"];
                if (esp.contains("LoadoutWindow")) g_ESP_LoadoutWindowEnabled = esp["LoadoutWindow"];
                if (esp.contains("MatchInfoHUD")) g_ESP_MatchInfoHUDEnabled = esp["MatchInfoHUD"];

                if (esp.contains("KillerEnabled")) g_ESP_KillerEnabled = esp["KillerEnabled"];
                if (esp.contains("KillerColor")) get_vec4(esp["KillerColor"], g_ESP_KillerColor);
                if (esp.contains("KillerBox")) g_ESP_KillerBoxEnabled = esp["KillerBox"];
                if (esp.contains("KillerBoxIndex")) g_ESP_KillerBoxIndex = esp["KillerBoxIndex"];
                if (esp.contains("KillerLine")) g_ESP_KillerLineEnabled = esp["KillerLine"];
                if (esp.contains("KillerLineIndex")) g_ESP_KillerLineIndex = esp["KillerLineIndex"];
                if (esp.contains("KillerSkeleton")) g_ESP_KillerSkeletonEnabled = esp["KillerSkeleton"];
                if (esp.contains("KillerDirectional")) g_ESP_KillerDirectional = esp["KillerDirectional"];
                if (esp.contains("SurvivorDirectional")) g_ESP_SurvivorDirectional = esp["SurvivorDirectional"];
                if (esp.contains("MaxDistance")) g_ESP_MaxDistance = esp["MaxDistance"];

                if (esp.contains("GeneratorEnabled")) g_ESP_GeneratorEnabled = esp["GeneratorEnabled"];
                if (esp.contains("GeneratorRainbow")) g_ESP_GeneratorRainbowEnabled = esp["GeneratorRainbow"];
                if (esp.contains("GeneratorColor")) get_vec4(esp["GeneratorColor"], g_ESP_GeneratorColor);
                if (esp.contains("GeneratorHideCompleted")) g_ESP_GeneratorHideCompleted = esp["GeneratorHideCompleted"];

                if (esp.contains("HookEnabled")) g_ESP_HookEnabled = esp["HookEnabled"];
                if (esp.contains("HookColor")) get_vec4(esp["HookColor"], g_ESP_HookColor);

                if (esp.contains("LockerEnabled")) g_ESP_LockerEnabled = esp["LockerEnabled"];
                if (esp.contains("LockerRainbow")) g_ESP_LockerRainbowEnabled = esp["LockerRainbow"];
                if (esp.contains("LockerColor")) get_vec4(esp["LockerColor"], g_ESP_LockerColor);

                if (esp.contains("TotemEnabled")) g_ESP_TotemEnabled = esp["TotemEnabled"];
                if (esp.contains("TotemRainbow")) g_ESP_TotemRainbowEnabled = esp["TotemRainbow"];
                if (esp.contains("TotemColor")) get_vec4(esp["TotemColor"], g_ESP_TotemColor);

                if (esp.contains("PalletEnabled")) g_ESP_PalletEnabled = esp["PalletEnabled"];
                if (esp.contains("PalletRainbow")) g_ESP_PalletRainbowEnabled = esp["PalletRainbow"];
                if (esp.contains("PalletColor")) get_vec4(esp["PalletColor"], g_ESP_PalletColor);

                if (esp.contains("HatchEnabled")) g_ESP_HatchEnabled = esp["HatchEnabled"];
                if (esp.contains("HatchColor")) get_vec4(esp["HatchColor"], g_ESP_HatchColor);

                if (esp.contains("TrapEnabled")) g_ESP_TrapEnabled = esp["TrapEnabled"];
                if (esp.contains("TrapColor")) get_vec4(esp["TrapColor"], g_ESP_TrapColor);

                if (esp.contains("BreakableDoorEnabled")) g_ESP_BreakableDoorEnabled = esp["BreakableDoorEnabled"];
                if (esp.contains("BreakableDoorRainbow")) g_ESP_BreakableDoorRainbowEnabled = esp["BreakableDoorRainbow"];
                if (esp.contains("BreakableDoorColor")) get_vec4(esp["BreakableDoorColor"], g_ESP_BreakableDoorColor);

                if (esp.contains("EscapeDoorEnabled")) g_ESP_EscapeDoorEnabled = esp["EscapeDoorEnabled"];
                if (esp.contains("EscapeDoorRainbow")) g_ESP_EscapeDoorRainbowEnabled = esp["EscapeDoorRainbow"];
                if (esp.contains("EscapeDoorColor")) get_vec4(esp["EscapeDoorColor"], g_ESP_EscapeDoorColor);

                if (esp.contains("ChestEnabled")) g_ESP_ChestEnabled = esp["ChestEnabled"];
                if (esp.contains("ChestColor")) get_vec4(esp["ChestColor"], g_ESP_ChestColor);

                if (esp.contains("WindowEnabled")) g_ESP_WindowEnabled = esp["WindowEnabled"];
                if (esp.contains("WindowColor")) get_vec4(esp["WindowColor"], g_ESP_WindowColor);

                if (esp.contains("OutlineTextEnabled")) g_ESP_OutlineTextEnabled = esp["OutlineTextEnabled"];
                if (esp.contains("OutlineTextColor")) get_vec4(esp["OutlineTextColor"], g_ESP_OutlineTextColor);

                if (esp.contains("CenobiteBoxEnabled")) g_ESP_CenobiteBoxEnabled = esp["CenobiteBoxEnabled"];
                if (esp.contains("CenobiteBoxRainbow")) g_ESP_CenobiteBoxRainbowEnabled = esp["CenobiteBoxRainbow"];
                if (esp.contains("CenobiteBoxColor")) get_vec4(esp["CenobiteBoxColor"], g_ESP_CenobiteBoxColor);

                if (esp.contains("NemesisZombieColor")) get_vec4(esp["NemesisZombieColor"], g_ESP_NemesisZombieColor);

                if (esp.contains("Victor")) g_ESP_Victor = esp["Victor"];
                if (esp.contains("HagTraps")) g_ESP_HagTraps = esp["HagTraps"];
                if (esp.contains("PlagueFountains")) g_ESP_PlagueFountains = esp["PlagueFountains"];
                if (esp.contains("SupplyCrates")) g_ESP_SupplyCrates = esp["SupplyCrates"];
                if (esp.contains("K33Tunnels")) g_ESP_K33Tunnels = esp["K33Tunnels"];
                if (esp.contains("K33Turrets")) g_ESP_K33Turrets = esp["K33Turrets"];
            }

            // Aura
            if (j.contains("Aura")) {
                auto& aura = j["Aura"];
                if (aura.contains("Enabled")) g_Aura_Enabled = aura["Enabled"];
                if (aura.contains("SurvivorEnabled")) g_Aura_SurvivorEnabled = aura["SurvivorEnabled"];
                if (aura.contains("SurvivorRainbow")) g_Aura_SurvivorRainbowEnabled = aura["SurvivorRainbow"];
                if (aura.contains("SurvivorColor")) get_vec4(aura["SurvivorColor"], g_Aura_SurvivorColor);
                if (aura.contains("KillerEnabled")) g_Aura_KillerEnabled = aura["KillerEnabled"];
                if (aura.contains("KillerRainbow")) g_Aura_KillerRainbowEnabled = aura["KillerRainbow"];
                if (aura.contains("KillerColor")) get_vec4(aura["KillerColor"], g_Aura_KillerColor);
                if (aura.contains("GeneratorEnabled")) g_Aura_GeneratorEnabled = aura["GeneratorEnabled"];
                if (aura.contains("GeneratorRainbow")) g_Aura_GeneratorRainbowEnabled = aura["GeneratorRainbow"];
                if (aura.contains("GeneratorColor")) get_vec4(aura["GeneratorColor"], g_Aura_GeneratorColor);
                if (aura.contains("HookEnabled")) g_Aura_HookEnabled = aura["HookEnabled"];
                if (aura.contains("HookColor")) get_vec4(aura["HookColor"], g_Aura_HookColor);
                if (aura.contains("LockerEnabled")) g_Aura_LockerEnabled = aura["LockerEnabled"];
                if (aura.contains("LockerRainbow")) g_Aura_LockerRainbowEnabled = aura["LockerRainbow"];
                if (aura.contains("LockerColor")) get_vec4(aura["LockerColor"], g_Aura_LockerColor);
                if (aura.contains("TotemEnabled")) g_Aura_TotemEnabled = aura["TotemEnabled"];
                if (aura.contains("TotemRainbow")) g_Aura_TotemRainbowEnabled = aura["TotemRainbow"];
                if (aura.contains("TotemColor")) get_vec4(aura["TotemColor"], g_Aura_TotemColor);
                if (aura.contains("PalletEnabled")) g_Aura_PalletEnabled = aura["PalletEnabled"];
                if (aura.contains("PalletRainbow")) g_Aura_PalletRainbowEnabled = aura["PalletRainbow"];
                if (aura.contains("PalletColor")) get_vec4(aura["PalletColor"], g_Aura_PalletColor);
                if (aura.contains("HatchEnabled")) g_Aura_HatchEnabled = aura["HatchEnabled"];
                if (aura.contains("HatchColor")) get_vec4(aura["HatchColor"], g_Aura_HatchColor);
                if (aura.contains("TrapEnabled")) g_Aura_TrapEnabled = aura["TrapEnabled"];
                if (aura.contains("TrapColor")) get_vec4(aura["TrapColor"], g_Aura_TrapColor);
                if (aura.contains("BreakableDoorEnabled")) g_Aura_BreakableDoorEnabled = aura["BreakableDoorEnabled"];
                if (aura.contains("BreakableDoorRainbow")) g_Aura_BreakableDoorRainbowEnabled = aura["BreakableDoorRainbow"];
                if (aura.contains("BreakableDoorColor")) get_vec4(aura["BreakableDoorColor"], g_Aura_BreakableDoorColor);
                if (aura.contains("EscapeDoorEnabled")) g_Aura_EscapeDoorEnabled = aura["EscapeDoorEnabled"];
                if (aura.contains("EscapeDoorRainbow")) g_Aura_EscapeDoorRainbowEnabled = aura["EscapeDoorRainbow"];
                if (aura.contains("EscapeDoorColor")) get_vec4(aura["EscapeDoorColor"], g_Aura_EscapeDoorColor);
                if (aura.contains("ChestEnabled")) g_Aura_ChestEnabled = aura["ChestEnabled"];
                if (aura.contains("ChestColor")) get_vec4(aura["ChestColor"], g_Aura_ChestColor);
                if (aura.contains("WindowEnabled")) g_Aura_WindowEnabled = aura["WindowEnabled"];
                if (aura.contains("WindowRainbow")) g_Aura_WindowRainbowEnabled = aura["WindowRainbow"];
                if (aura.contains("WindowColor")) get_vec4(aura["WindowColor"], g_Aura_WindowColor);
                if (aura.contains("OtherEnabled")) g_Aura_OtherEnabled = aura["OtherEnabled"];
                if (aura.contains("OtherColor")) get_vec4(aura["OtherColor"], g_Aura_OtherColor);
            }

            // Cheats
            if (j.contains("Cheats")) {
                auto& cheats = j["Cheats"];
                if (cheats.contains("Enabled")) g_Cheats_Enabled = cheats["Enabled"];
                if (cheats.contains("FOVEnabled")) g_Cheats_FOVEnabled = cheats["FOVEnabled"];
                if (cheats.contains("FOV")) g_Cheats_FOV = cheats["FOV"];
                if (cheats.contains("KillerStain")) g_Cheats_KillerStainEnabled = cheats["KillerStain"];
                if (cheats.contains("KillerStainRainbow")) g_Cheats_KillerStainRainbowEnabled = cheats["KillerStainRainbow"];
                if (cheats.contains("KillerStainColor")) get_vec4(cheats["KillerStainColor"], g_Cheats_KillerStainColor);
                if (cheats.contains("AutoDeadHard")) g_Cheats_AutoDeadHardEnabled = cheats["AutoDeadHard"];
                if (cheats.contains("DeadHardDistance")) g_Cheats_DeadHardDistance = cheats["DeadHardDistance"];
                if (cheats.contains("DeadHardPerfect")) g_Cheats_DeadHardPerfect = cheats["DeadHardPerfect"];
                if (cheats.contains("AutoSkillCheck")) g_Cheats_AutoSkillCheckEnabled = cheats["AutoSkillCheck"];
                if (cheats.contains("AutoSkillCheckDelayMs")) g_Cheats_AutoSkillCheckDelayMs = cheats["AutoSkillCheckDelayMs"];
                if (cheats.contains("SkillCheckDebug")) g_Cheats_SkillCheckDebugEnabled = cheats["SkillCheckDebug"];
                if (cheats.contains("SpamSpacebar")) g_Cheats_SpamSpacebar = cheats["SpamSpacebar"];
                if (cheats.contains("RadarEnabled")) g_Cheats_RadarEnabled = cheats["RadarEnabled"];
                if (cheats.contains("RadarZoom")) g_Cheats_RadarZoom = cheats["RadarZoom"];
                if (cheats.contains("RadarRange")) g_Cheats_RadarRange = cheats["RadarRange"];
                if (cheats.contains("AimbotEnabled")) g_Cheats_AimbotEnabled = cheats["AimbotEnabled"];
                if (cheats.contains("AimbotBind")) g_Cheats_AimbotBind = cheats["AimbotBind"];
                if (cheats.contains("AimbotPrediction")) g_Cheats_AimbotPredictionEnabled = cheats["AimbotPrediction"];
                if (cheats.contains("AimbotProjectileSpeed")) g_Cheats_AimbotProjectileSpeed = cheats["AimbotProjectileSpeed"];
                if (cheats.contains("AimbotProjectileGravity")) g_Cheats_AimbotProjectileGravity = cheats["AimbotProjectileGravity"];
                if (cheats.contains("AimbotFOV")) g_Cheats_AimbotFOV = cheats["AimbotFOV"];
                if (cheats.contains("AimbotSmoothing")) g_Cheats_AimbotSmoothing = cheats["AimbotSmoothing"];
                if (cheats.contains("AimbotDropScale")) g_Cheats_AimbotDropScale = cheats["AimbotDropScale"];
                if (cheats.contains("AimbotOffset")) get_fvec(cheats["AimbotOffset"], g_Cheats_AimbotOffset);

                if (cheats.contains("CrosshairEnabled")) g_Cheats_CrosshairEnabled = cheats["CrosshairEnabled"];
                if (cheats.contains("CrosshairType")) g_Cheats_CrosshairType = cheats["CrosshairType"];
                if (cheats.contains("CrosshairSize")) g_Cheats_CrosshairSize = cheats["CrosshairSize"];
                if (cheats.contains("CrosshairThickness")) g_Cheats_CrosshairThickness = cheats["CrosshairThickness"];
                if (cheats.contains("CrosshairColor")) get_vec4(cheats["CrosshairColor"], g_Cheats_CrosshairColor);
            }

            if (j.contains("FPS_Limit")) g_FPS_Limit = j["FPS_Limit"];

        } catch (...) {}
    }

    void Delete(const std::string& name) {
        std::string path = GetPath(name);
        if (fs::exists(path)) {
            fs::remove(path);
        }
    }

    std::vector<std::string> List() {
        std::vector<std::string> configs;
        try {
            if (fs::exists("configs") && fs::is_directory("configs")) {
                for (const auto& entry : fs::directory_iterator("configs")) {
                    if (entry.is_regular_file() && entry.path().extension() == ".json") {
                        configs.push_back(entry.path().stem().string());
                    }
                }
            }
        } catch (...) {}
        return configs;
    }

    void SetDefault(const std::string& name) {
        json j;
        j["DefaultConfig"] = name;
        std::ofstream file("settings.json");
        if (file.is_open()) {
            file << j.dump(4);
        }
    }

    std::string GetDefault() {
        std::ifstream file("settings.json");
        if (!file.is_open()) return "default";
        try {
            json j;
            file >> j;
            return j.value("DefaultConfig", "default");
        } catch (...) {
            return "default";
        }
    }
}
