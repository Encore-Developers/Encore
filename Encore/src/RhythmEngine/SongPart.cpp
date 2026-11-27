#include "SongPart.h"

#include "assets.h"

#include <fstream>

using namespace Encore::RhythmEngine;

using json = nlohmann::json;

std::unordered_map<std::string, SongPart> SongPart::LoadedParts;

Encore::LocalizedString SongPart::GetLocalizedName() const {
    return LOCALIZE("parts." + id);
}
TextureAsset *SongPart::GetIcon() const {
    return TheAssets.InstIcons[icon];
}
bool SongPart::IsAssignedStem(const std::string &stem) const {
    for (auto& checkStem : assignedStems) {
        if (checkStem == stem) return true;
    }
    return false;
}
std::vector<SongPart*> SongPart::GetFromMidiTrackName(const std::string &midiTrackName){
    std::vector<SongPart*> ret;
    for (auto &part : LoadedParts | std::views::values) {
        for (auto& findName : part.midiTrackNames) {
            if (findName == midiTrackName) {
                ret.push_back(&part);
                break;
            }
        }
    }
    return ret;
}
SongPart *SongPart::Get(const std::string &id) {
    return &LoadedParts[id];
}
void SongPart::LoadSongParts(const std::filesystem::path &partsFolder) {
    Log::Info("Loading song parts from {}", partsFolder.string());
    for (auto& entry : std::filesystem::directory_iterator(partsFolder)) {
        if (entry.is_directory()) {
            LoadSongParts(entry.path());
        } else {
            std::ifstream f(entry.path());
            json SongPartJson = json::parse(f);
            std::string id = SongPartJson["id"].get<std::string>();
            SongPart part;
            from_json(SongPartJson, part);
            LoadedParts.insert({id, part});
        }
    }
}
void from_json(const nlohmann::json &j, Encore::RhythmEngine::SongPart &p) {
    p.id = j["id"];
    from_json(j["archetype"], p.archetype);
    p.midiTrackNames = j["midiTrackNames"];
    if (j.contains("dotChartSectionNames")) {
        p.dotChartSectionNames = j["dotChartSectionNames"];
    } else {
        if (p.archetype != ArchetypePad) {
            Encore::Log::Warn("Non-pad song part {} does not have .chart section names defined", p.id);
        }
    }
    p.iniDifficultyKey = j["iniDifficultyKey"];
    p.maxMultiplier = j["maxMultiplier"];
    if (j.contains("convertsFrom")) {
        p.convertsFrom = j["convertsFrom"];
    }
    p.assignedStems = j["assignedStems"];
    if (j.contains("stemPriorityOffset")) {
        p.stemPriorityOffset = j["stemPriorityOffset"];
    }
    if (j.contains("playable")) {
        p.playable = j["playable"];
    } else {
        p.playable = true;
    }
    p.flags = 0;
    if (j.contains("flags")) {
        for (auto& flagName : j["flags"]) {
            PartFlag flag;
            from_json(flagName, flag);
            p.flags |= flag;
        }
    }
    p.sortOrder = j["sortOrder"];
    p.icon = j["icon"];
    p.presenceIconName = j["presenceIconName"];
}