#pragma once

//
// Created by marie on 04/05/2024.
//

#include <string>
#include <filesystem>

#include "raylib.h"
#include "song/chart.h"
// #include "libstud-uuid/uuid/uuid.hxx"
/*

 note! this is kinda just me throwing shit at the wall to see what makes sense.
 kinda just makin the points and then connecting them later to fit into Encore itself
 this shouldnt exactly impede on builds yet i think
 also yes i know i should probably put the band and player stuff in their own headers
 ill do that later once i got this theorized. already have band.h made so once i get to
 that point ill slap it there its just here for convenience.


*/

// realizing how i could just make a "SelectableEntity" class and then extend Band and
// Player from it instead of having the logic rewritten between the two

// acts as an individual save-file

// acts like a system-wide save-file
// think AC:NH islands
// note: will we really have PVP stuff? like. thinking like RB3 here, would there be PVP
// attributed to bands? cuz i think it would severely complicate it if PvP stuff was more
// "oh this band won with these players" instead of just noting in a save file "p1 and p3
// worked together and won against p2 and p4" instead of "band 1 with p1 and p3 won over
// band 2 with p2 and p4, especially when that stuff will probably add a win count to
// players who didnt even participate but won (because they were part of the band)
// literally every team game i can think of thats pvp doesnt really do this unless its
// strict about teams i think correct me if im wrong still would be useful for co-op band
// stuff
class Band {
    std::filesystem::path ScoreFile;
    bool SoloGameplay = true; // to be true until multiple players
};

enum Instruments {
    PAD_DRUMS,
    PAD_BASS,
    PAD_LEAD,
    PAD_KEYS,
    PAD_VOCALS,
    PLASTIC_DRUMS,
    PLASTIC_BASS,
    PLASTIC_LEAD,
    PLASTIC_KEYS,
    PLASTIC_VOCALS
};

class PlayerGameplayStats {
public:
    PlayerGameplayStats();

    bool Quit;
    bool FC;
    bool Paused;
    bool GoldStars;
    bool Overdrive;
    bool Mute;

    int Score;
    int Combo;
    int MaxCombo;
    int Overhits;
    int Notes;
    int NotesHit;
    int GoodHit;
    int PerfectHit;
    int NotesMissed;

    int ScoreToDisplay();

    std::vector<bool> HeldFrets { false, false, false, false, false };
    std::vector<bool> HeldFretsAlt { false, false, false, false, false };
    std::vector<bool> OverhitFrets { false, false, false, false, false };
    std::vector<bool> TapRegistered { false, false, false, false, false };
    std::vector<bool> LiftRegistered { false, false, false, false, false };
    double StartTime = 0.0;
    double SongStartTime = 0.0;
    std::vector<float> SustainScoreBuffer { 0.0, 0.0, 0.0, 0.0, 0.0 };
    int curBPM = 0;
    int curBeatLine = 0;
    int curODPhrase = 0;
    int curSolo = 0;
    int curFill = 0;
    int curNoteInt = 0;
    int curSection = 0;

    double lastAxesTime = 0.0;
    std::vector<float> axesValues { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    std::vector<int> buttonValues { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::vector<float> axesValues2 { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    int pressedGamepadInput = -999;
    int axisDirection = -1;
    bool FAS = false;
    double StrumNoFretTime = -1.0;
    int strummedNote;
    bool Overstrum = false;
    bool DownStrum = false;
    bool UpStrum = false;
    bool extendedSustainActive = false;

    bool overdriveHeld = false;
    bool overdriveAltHeld = false;
    bool overdriveHitAvailable = false;
    bool overdriveLiftAvailable = false;
    std::vector<bool> overdriveLanesHit { false, false, false, false, false };
    double overdriveHitTime = 0.0;
    std::vector<int> lastHitLifts { -1, -1, -1, -1, -1 };

    std::vector<int> curNoteIdx = { 0, 0, 0, 0, 0 };

    float Health;
    Chart CurPlayingChart;
    bool Multiplayer = false;
    float overdriveFill;
    float overdriveActiveFill;
    double overdriveActiveTime;
    double overdriveActivateTime;

    int Instrument;
    int Difficulty;
    int BaseScore;
    float xStarThreshold[6] = { 0.05f, 0.175f, 0.325f, 0.5f, 0.7f, 1.0f };

    void HitNote(bool perfect);
    void HitDrumsNote(bool perfect, bool cymbal);
    void HitPlasticNote(Note note);
    void MissNote();
    void OverHit();

    int maxMultForMeter();

    int maxComboForMeter();

    int Stars();

    float uvOffsetX = 0;
    float uvOffsetY = 0;

    int multiplier();

    int noODmultiplier();

    bool IsBassOrVox();

    float comboFillCalc();
};

#define PLAYER_JSON_SETTINGS                                     \
    SETTING_ACTION(int,     Difficulty,         "diff")         \
    SETTING_ACTION(int,     Instrument,         "inst")         \
    SETTING_ACTION(float,   InputCalibration,   "inputOffset")  \
    SETTING_ACTION(float,   NoteSpeed,          "NoteSpeed")    \
    SETTING_ACTION(bool,    ProDrums,           "proDrums")     \
    SETTING_ACTION(bool,    Bot,                "bot")          \
    SETTING_ACTION(float,   HighwayLength,      "length")       \
    SETTING_ACTION(bool,    ClassicMode,        "classic")      \
    SETTING_ACTION(bool,    LeftyFlip,          "lefty")

#define PLAYER_CONFIG_LIST                                     \
    SETTING_ACTION(int,     Difficulty)         \
    SETTING_ACTION(int,     Instrument)         \
    SETTING_ACTION(float,   InputCalibration)  \
    SETTING_ACTION(float,   NoteSpeed)    \
    SETTING_ACTION(bool,    ProDrums)     \
    SETTING_ACTION(bool,    Bot)          \
    SETTING_ACTION(float,   HighwayLength)       \
    SETTING_ACTION(bool,    ClassicMode)      \
    SETTING_ACTION(bool,    LeftyFlip)

class Player {
public:
    Player();

    std::string Name; // display name
    std::string PlayerID; // UUID
    // std::filesystem::path SettingsFile;
    PlayerGameplayStats *stats;

    Color AccentColor = { 255, 0, 255, 255 };
#define SETTING_ACTION(type, name) type name;
    PLAYER_CONFIG_LIST;
#undef SETTING_ACTION
    std::string playerJsonObjectName;
    int SongsPlayed;
    int joypadID;
    bool ReadiedUpBefore;
    bool Online;
    int ActiveSlot{};

    void ResetGameplayStats();

    bool ReadyUpMenu = false;
    bool diffSelected = false;
    bool diffSelection = false;
    bool instSelection = true;
    bool instSelected = false;
    // zero indexed. local would be 0-3, online would be 4-7.
    // NOTE! this is only for like. local information and
    // not actually shared information. i was thinking of a UUID system for online
};


class BandGameplayStats : public PlayerGameplayStats {
public:
    BandGameplayStats();

    void ResetBandGameplayStats();
    bool EligibleForGoldStars = false;
    bool Multiplayer = false;
    std::vector<int> OverdriveMultiplier { 1, 2, 4, 6, 8 };
    int PlayersInOverdrive = 0;

    void AddNotePoint(bool perfect, int playerMult);
    int Stars() {
        float starPercent = (float)Score / (float)BaseScore;
        if (starPercent < xStarThreshold[0]) {
            return 0;
        } else if (starPercent < xStarThreshold[1]) {
            return 1;
        } else if (starPercent < xStarThreshold[2]) {
            return 2;
        } else if (starPercent < xStarThreshold[3]) {
            return 3;
        } else if (starPercent < xStarThreshold[4]) {
            return 4;
        } else if (starPercent < xStarThreshold[5]) {
            return 5;
        } else if (starPercent >= xStarThreshold[5] && EligibleForGoldStars) {
            GoldStars = true;
            return 5;
        } else
            return 5;

        return 0;
    }
    void AddClassicNotePoint(bool perfect, int playerMult, int chordSize);

    void DrumNotePoint(bool perfect, int playerMult, bool cymbal);
};



