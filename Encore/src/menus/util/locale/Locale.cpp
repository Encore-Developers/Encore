#include "Locale.h"

#include "tracy/Tracy.hpp"
#include "assets.h"
#include "settings/settings.h"

#include <nlohmann/json.hpp>
#include <fstream>

using namespace Encore;
using json = nlohmann::json;

std::vector<LocaleLayer> Locale::layers = {};
std::unordered_set<std::string> Locale::unlocalizedTokens;
bool Locale::debugLongStrings = false;

void LocaleLayer::AddEntries(const std::string& stem, nlohmann::basic_json<> &json) {
    for (auto& [key, value] : json.items()) {
        auto fullKey = stem + key;
        if (value.type() == nlohmann::json::value_t::string) {
            entries.emplace(fullKey, value);
        }
        if (value.type() == nlohmann::json::value_t::array) {
            if (!lists.contains(fullKey)) {
                lists.emplace(fullKey, std::make_shared<std::vector<std::string>>());
            }
            auto list = lists.find(fullKey);
            for (auto& [index, str] : value.items()) {
                list->second->push_back(str);
            }
        }
        if (value.type() == nlohmann::json::value_t::object) {
            AddEntries(fullKey + ".", value);
        }
    }
}
LocaleLayer::LocaleLayer(const std::string &name, bool fallback) : name(name), fallback(fallback) {
    auto path = FileAsset::ResolveAssetPath("locale/" + name + ".json");
    std::ifstream file(path);
    auto json = json::parse(file);
    AddEntries("", json);
}
const std::string *LocaleLayer::FetchValue(const std::string &token) const {
    auto found = entries.find(token);
    if (found == entries.end()) return nullptr;
    return &found->second;
}
const LocaleList *LocaleLayer::FetchList(const std::string &token) const {
    auto found = lists.find(token);
    if (found == lists.end()) return nullptr;
    return found->second.get();
}
void Locale::Init() {
    layers.clear();
    unlocalizedTokens.clear();
    Log::Warn("Loading locale. Using locale from settings: {}", TheGameSettings.Language);
    AddLayer(TheGameSettings.Language, false);
    AddLayer("en_US", true); // Fall back to English
}
void Locale::AddLayer(const std::string &name, bool fallback) {
    // Check if a locale layer with this name already exists
    for (auto& layer : layers) {
        if (layer.name == name) {
            return;
        }
    }
    auto path = FileAsset::ResolveAssetPath("locale/" + name + ".json");
    if (std::filesystem::exists(path)) {
        layers.emplace_back(name, fallback);
    } else {
        Log::Error("Could not find locale file for {}", name);
    }
}
LocalizedString Locale::Localize(const std::string &token) {
    ZoneScoped
    if (debugLongStrings) {
        return LocalizedString::FromStaticChars("_____________________LOCALIZED_____________________");
    }
    for (auto &layer : layers) {
        if (auto localized = layer.FetchValue(token)) {
            if (layer.fallback) {
                unlocalizedTokens.insert(token);
            }
            return LocalizedString::FromStringPtr(localized);
        }
    }
    unlocalizedTokens.insert(token);
    return token;
}
const LocaleList *Locale::GetLocaleList(const std::string &token) {
    for (auto &layer : layers) {
        if (auto list = layer.FetchList(token)) {
            if (layer.fallback) {
                unlocalizedTokens.insert(token);
            }
            return list;
        }
    }
    unlocalizedTokens.insert(token);
    static LocaleList emptyList = {};
    return &emptyList;
}