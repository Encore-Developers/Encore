#pragma once

//
// Created by marie on 04/05/2024.
//

#include <string>
#include <filesystem>

#include "raylib.h"
#include "song/chart.h"
#include "song/scoring.h"
// #include "libstud-uuid/uuid/uuid.hxx"

class Band {
    /**
     * @brief Do Not Use. Outdated.
     * Already achieved with Band Gameplay Stats and the Band shit
     */
    std::filesystem::path ScoreFile;
    bool SoloGameplay = true; // to be true until multiple players

};

enum NoteHitType {
    STANDARD, // strums/ptaps
    ALTERNATIVE // hopos/ctaps/lifts
};

class PlayerGameplayStats {
    /**
     * @brief Statistics/statistics manager for individual players during gameplay
     */
public:
    PlayerGameplayStats();

    bool Quit;
    bool FC;
    bool Paused;
    double LastPerfectTime = -2.0;
    bool GoldStars() {
        float starPercent = Score / BaseScore;
        if (starPercent >= STAR_THRESHOLDS[Instrument][5])
            return true;
        return false;
    };
    bool Overdrive;
    bool Mute;

    double Score;
    // extra scoring information
    double SustainScore = 0;
    double MultiplierScore = 0;
    double OverdriveScore = 0;
    double PerfectScore = 0;
    double NoteScore = 0;

    int Combo;
    int MaxCombo;
    int Overhits;
    int Notes;
    int NotesHit;
    int GoodHit;
    int PerfectHit;
    int NotesMissed;
    int PressedMask = 0;
    double MultiplierEffectTime = 0.0;
    bool Miss = false;
    int ScoreToDisplay();

    std::vector<float> drumSmasherRotations = { 0, 0, 0, 0 };
    std::vector<float> drumSmasherHeights = { 0, 0, 0, 0 };

    std::vector<std::pair<float, int> > HitwindowNoteHitOffset = {};

    std::vector<float> fiveLaneSmasherRotation = { 0, 0, 0, 0, 0 };
    std::vector<float> fiveLaneSmasherHeights = { 0, 0, 0, 0, 0 };

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
    double LastTick = 0.0;

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

    float Health = 0.75f;
    Chart CurPlayingChart;
    bool Multiplayer = false;
    float overdriveFill;
    float overdriveActiveFill;
    double overdriveActiveTime;
    double overdriveActivateTime;

    int Instrument;
    int Difficulty;
    int BaseScore;

    void AddHealth();
    void LoseHealth();
    PlayerGameplayStats(int difficulty, int instrument);
    void HitNote(bool perfect);
    void HitDrumsNote(bool perfect, bool cymbal);
    void HitPlasticNote(Note note);
    void MissNote();
    void OverHit();

    int maxMultForMeter();

    int maxComboForMeter();

    int Stars();
    void MultiplierUVCalculation();

    float uvOffsetX = 0;
    float uvOffsetY = 0;

    int multiplier();

    int noODmultiplier();

    bool IsBassOrVox();

    float comboFillCalc();
};

#define PLAYER_JSON_SETTINGS                                                             \
    SETTING_ACTION(int, Difficulty, "diff")                                              \
    SETTING_ACTION(int, Instrument, "inst")                                              \
    SETTING_ACTION(float, InputCalibration, "inputOffset")                               \
    SETTING_ACTION(float, NoteSpeed, "NoteSpeed")                                        \
    SETTING_ACTION(bool, ProDrums, "proDrums")                                           \
    SETTING_ACTION(bool, Bot, "bot")                                                     \
    SETTING_ACTION(float, HighwayLength, "length")                                       \
    SETTING_ACTION(bool, ClassicMode, "classic")                                         \
    SETTING_ACTION(bool, LeftyFlip, "lefty")                                             \
    SETTING_ACTION(bool, BrutalMode, "BrutalMode")

#define PLAYER_CONFIG_LIST                                                               \
    SETTING_ACTION(int, Difficulty)                                                      \
    SETTING_ACTION(int, Instrument)                                                      \
    SETTING_ACTION(float, InputCalibration)                                              \
    SETTING_ACTION(float, NoteSpeed)                                                     \
    SETTING_ACTION(bool, ProDrums)                                                       \
    SETTING_ACTION(bool, Bot)                                                            \
    SETTING_ACTION(float, HighwayLength)                                                 \
    SETTING_ACTION(bool, ClassicMode)                                                    \
    SETTING_ACTION(bool, LeftyFlip)                                                      \
    SETTING_ACTION(bool, BrutalMode)
class Player {
    /**
     * @brief Player information. What else could be said?
     */
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
    int ActiveSlot {};

    void ResetGameplayStats();

    enum ReadyUpStates {
        PREVIEW,
        INSTRUMENT,
        DIFFICULTY
    };
    int ReadyUpMenuState = 1;

    bool instSelected = false;
    bool diffSelected = false;
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
    bool GoldStars() {
        float starPercent = (float)Score / (float)BaseScore;
        if (starPercent >= BAND_STAR_THRESHOLD[5])
            return true;
        return false;
    };
    void AddNotePoint(bool perfect, int playerMult);
    int Stars() {
        float starPercent = (float)Score / (float)BaseScore;
        if (starPercent < BAND_STAR_THRESHOLD[0]) {
            return 0;
        } else if (starPercent < BAND_STAR_THRESHOLD[1]) {
            return 1;
        } else if (starPercent < BAND_STAR_THRESHOLD[2]) {
            return 2;
        } else if (starPercent < BAND_STAR_THRESHOLD[3]) {
            return 3;
        } else if (starPercent < BAND_STAR_THRESHOLD[4]) {
            return 4;
        } else if (starPercent < BAND_STAR_THRESHOLD[5]) {
            return 5;
        }

        return 0;
    }
    void AddClassicNotePoint(bool perfect, int playerMult, int chordSize);

    void DrumNotePoint(bool perfect, int playerMult, bool cymbal);
};
