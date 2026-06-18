#pragma once
#include <string>
#include <vector>

namespace Config {
    extern std::string currentConfig;
    void Save(const std::string& filename = "");
    void Load(const std::string& filename = "");
    void Delete(const std::string& name);
    std::vector<std::string> List();
    void SetDefault(const std::string& name);
    std::string GetDefault();
}
