#pragma once
#include "menus/util/locale/Locale.h"

#include <string>
#include <vector>

class TextureAsset;
namespace Encore::RhythmEngine {

    /// Specifies both what engine to use for a song part and what loader to use.
    /// TODO: 5 lane drums archetype
    enum EngineArchetype {
        ArchetypePad,
        ArchetypeDrums,
        ArchetypeGuitar
    };

    enum PartFlag {
        /// JSON key "cymbals"
        /// This song part detects cymbals in drums charts. Used to differentiate drums from pro drums
        FlagCymbals = 1 << 0,
        /// JSON key "bass"
        /// This song part can get "Bass Groove"
        FlagBass = 1 << 1
    };


    class SongPart {
    public:
        /// Part identifier. Stored in replays and player profiles. Also used as the localization key.
        std::string id;
        EngineArchetype archetype;
        // GH1 midis use "T1 GEMS" as the midi track name for Lead Guitar. As far as I know, this is the only time where 1 part has multiple track names.
        std::vector<std::string> midiTrackNames;
        std::vector<std::string> dotChartSectionNames;
        /// diff_* key this part uses in the ini
        std::string iniDifficultyKey;
        int maxMultiplier;
        /// ID of an another song part that this part will be generated from if it does not exist.
        std::string convertsFrom;
        /// Ordered from highest priority to lowest priority
        std::vector<std::string> assignedStems;
        /// The higher this is, the lower priority this part will be when deciding how to assign stems
        int stemPriorityOffset;
        uint64_t flags;

        int sortOrder;

        /// See Assets::AddRingsAndInstruments for names
        std::string icon;
        std::string presenceIconName;

        /// Used for pitched vocals so it can have a difficulty but not be playable (yet?)
        bool playable;

        LocalizedString GetLocalizedName() const;
        TextureAsset* GetIcon() const;

        bool IsAssignedStem(const std::string& stem) const;

        static std::vector<SongPart*> GetFromMidiTrackName(const std::string& midiTrackName);
        static std::unordered_map<std::string, SongPart> LoadedParts;
        static SongPart* Get(const std::string& id);
        /// Call this very early in init process, before players & songs are loaded
        static void LoadSongParts(const std::filesystem::path& partsFolder);
    };
}

NLOHMANN_JSON_SERIALIZE_ENUM(Encore::RhythmEngine::PartFlag, {
    {Encore::RhythmEngine::FlagCymbals, "cymbals"},
    {Encore::RhythmEngine::FlagBass, "bass"}
})

NLOHMANN_JSON_SERIALIZE_ENUM(Encore::RhythmEngine::EngineArchetype, {
    {Encore::RhythmEngine::ArchetypePad, "pad"},
    {Encore::RhythmEngine::ArchetypeDrums, "drums"},
    {Encore::RhythmEngine::ArchetypeGuitar, "guitar"}
})

void from_json(const nlohmann::json& j, Encore::RhythmEngine::SongPart& p);

