#pragma once
#include <string>
#include <nlohmann/json.hpp>

#define PRESET_REGISTRY_ID(id) \
std::string GetRegistryID() {return id;}\
static std::string StaticRegistryID() {return id;}

class Preset {
public:
    std::string name;

    virtual std::string GetRegistryID();
    virtual nlohmann::json ToJson();
    operator nlohmann::json () {
        return ToJson();
    }
    Preset();
    virtual ~Preset();
};
