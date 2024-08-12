//
// Created by marie on 04/05/2024.
//

#ifndef ENCORE_PLAYER_H
#define ENCORE_PLAYER_H

#include <string>
#include <filesystem>
#include <map>

#include "raylib.h"
#include "rapidjson/document.h"
#include "uuid.h"
#include "song/chart.h"
// #include "libstud-uuid/uuid/uuid.hxx"
/*

 note! this is kinda just me throwing shit at the wall to see what makes sense.
 kinda just makin the points and then connecting them later to fit into Encore itself
 this shouldnt exactly impede on builds yet i think
 also yes i know i should probably put the band and player stuff in their own headers
 ill do that later once i got this theorized. already have band.h made so once i get to that point ill slap it there
 its just here for convenience.


*/

// realizing how i could just make a "SelectableEntity" class and then extend Band and Player from it instead of having
// the logic rewritten between the two



// acts as an individual save-file


// acts like a system-wide save-file
// think AC:NH islands
// note: will we really have PVP stuff? like. thinking like RB3 here, would there be PVP attributed to bands?
// cuz i think it would severely complicate it if PvP stuff was more "oh this band won with these players" instead of
// just noting in a save file "p1 and p3 worked together and won against p2 and p4" instead of "band 1 with p1 and p3 won
// over band 2 with p2 and p4, especially when that stuff will probably add a win count to players who didnt even
// participate but won (because they were part of the band)
// literally every team game i can think of thats pvp doesnt really do this unless its strict about teams i think
// correct me if im wrong
// still would be useful for co-op band stuff
class Band {
    std::filesystem::path ScoreFile;
    bool SoloGameplay = true; // to be true until multiple players
};

class PlayerGameplayStats {
public:
    PlayerGameplayStats();


    bool Quit;
    bool FC;
    bool Paused;
    bool GoldStars;
    bool Overdrive;

    int Score;
    int Combo;
    int MaxCombo;
    int Overhits;
    int Notes;
    int NotesHit;
    int PerfectHit;
    int NotesMissed;

    std::vector<bool> HeldFrets{ false,false,false,false,false };
    std::vector<bool> HeldFretsAlt{ false,false,false,false,false };
    std::vector<bool> OverhitFrets{ false,false,false,false,false };
    std::vector<bool> TapRegistered{ false,false,false,false,false };
    std::vector<bool> LiftRegistered{ false,false,false,false,false };
    double StartTime = 0.0;
    double SongStartTime = 0.0;

    int curBPM = 0;
    int curBeatLine = 0;
    int curODPhrase = 0;
    int curSolo = 0;
    int curFill = 0;
    int curNoteInt = 0;

    double lastAxesTime = 0.0;
    std::vector<float> axesValues{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::vector<int> buttonValues{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<float> axesValues2{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
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
    std::vector<bool> overdriveLanesHit{false, false, false, false, false};
    double overdriveHitTime = 0.0;
    std::vector<int> lastHitLifts{-1, -1, -1, -1, -1};


    std::vector<int> curNoteIdx = { 0,0,0,0,0 };

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
    float xStarThreshold[6] = { 0.05f, 0.175f, 0.325f, 0.5f, 0.7f,  1.0f };

    void HitNote(bool perfect) {
        NotesHit += 1;
        Notes += 1;
        Combo += 1;
        if (Combo > MaxCombo)
            MaxCombo = Combo;
        float perfectMult = perfect ? 1.2f : 1.0f;
        Score += (int)((30.0f * (multiplier()) * perfectMult));
        PerfectHit += perfect ? 1 : 0;
        // mute = false;
    }
    void HitDrumsNote(bool perfect, bool cymbal) {
        NotesHit += 1;
        Notes += 1;
        Combo += 1;
        if (Combo > MaxCombo)
            MaxCombo = Combo;
        float cymbMult = cymbal ? 1.3f : 1.0f;
        float perfectMult = perfect ? 1.2f : 1.0f;
        Score += (int)((30.0f * (multiplier()) * perfectMult) * cymbMult);
        PerfectHit += perfect ? 1 : 0;
        // mute = false;
    }
    void HitPlasticNote(Note note) {
        NotesHit += 1;
        Notes += 1;
        Combo += 1;
        if (Combo > MaxCombo)
            MaxCombo = Combo;
        float perfectMult = note.perfect ? 1.2f : 1.0f;
        Score += (note.chordSize * (int)((30.0f * (multiplier()) * perfectMult)));
        PerfectHit += note.perfect ? 1 : 0;
        // mute = false;
    }
    void MissNote() {
        NotesMissed += 1;
        Notes += 1;
        // if (combo != 0)
        //     playerAudioManager.playSample("miss", sfxVolume);
        if (Combo > MaxCombo)
            MaxCombo = Combo;
        Combo = 0;
        FC = false;
        // mute = true;
    }
    void OverHit() {
        // if (combo != 0)
        //     playerAudioManager.playSample("miss", sfxVolume);
        if (Combo > MaxCombo)
            MaxCombo = Combo;
        Combo = 0;
        Overhits += 1;
        FC = false;
        // mute = true;
    }

    int maxMultForMeter() {
        if (Instrument == 1 || Instrument == 3 || Instrument == 5)
            return 5;
        else
            return 3;
    }

    int Stars() {
        float starPercent = (float)MaxCombo/(float)BaseScore;
        if (starPercent < xStarThreshold[0]) {return 0;}
        else if (starPercent < xStarThreshold[1]) { return 1; }
        else if (starPercent < xStarThreshold[2]) {return 2;}
        else if (starPercent < xStarThreshold[3]) {return 3;}
        else if (starPercent < xStarThreshold[4]) {return 4;}
        else if (starPercent < xStarThreshold[5]) {return 5;}
        else if (starPercent >= xStarThreshold[5] && Difficulty == 3) { GoldStars = true; return 5; }
        else return 5;

        return 0;
    }

    float uvOffsetX = 0;
    float uvOffsetY = 0;

    int multiplier() {
        int od = Overdrive ? 2 : 1;

        if (Instrument == 1 || Instrument == 3 || Instrument == 5){

            if (Combo < 10) { uvOffsetX = 0; uvOffsetY = 0 + (Overdrive ? 0.5f:0); return 1 * od; }
            else if (Combo < 20) { uvOffsetX = 0.25f; uvOffsetY = 0 + (Overdrive ? 0.5f : 0);  return 2 * od; }
            else if (Combo < 30) { uvOffsetX = 0.5f; uvOffsetY = 0 + (Overdrive ? 0.5f : 0);  return 3 * od; }
            else if (Combo < 40) { uvOffsetX = 0.75f; uvOffsetY = 0 + (Overdrive ? 0.5f : 0); return 4 * od; }
            else if (Combo < 50) { uvOffsetX = 0; uvOffsetY = 0.25f + (Overdrive ? 0.5f : 0); return 5 * od; }
            else if (Combo >= 50) { uvOffsetX = 0.25f; uvOffsetY = 0.25f + (Overdrive ? 0.5f : 0); return 6 * od; }
            else { return 1 * od; };
        }
        else {
            if (Combo < 10) { uvOffsetX = 0; uvOffsetY = 0 + (Overdrive ? 0.5 : 0); return 1 * od; }
            else if (Combo < 20) { uvOffsetX = 0.25f; uvOffsetY = 0 + (Overdrive ? 0.5 : 0); return 2 * od; }
            else if (Combo < 30) { uvOffsetX = 0.5f; uvOffsetY = 0 + (Overdrive ? 0.5 : 0); return 3 * od; }
            else if (Combo >= 30) { uvOffsetX = 0.75f; uvOffsetY = 0 + (Overdrive ? 0.5 : 0); return 4 * od; }
            else { return 1 * od; }
        };
    }

    int noODmultiplier() {

        if (Instrument == 1 || Instrument == 3 || Instrument == 5){

            if (Combo < 10) { uvOffsetX = 0; uvOffsetY = 0; return 1; }
            else if (Combo < 20) { uvOffsetX = 0.25f; uvOffsetY = 0;  return 2; }
            else if (Combo < 30) { uvOffsetX = 0.5f; uvOffsetY = 0;  return 3; }
            else if (Combo < 40) { uvOffsetX = 0.75f; uvOffsetY = 0; return 4; }
            else if (Combo < 50) { uvOffsetX = 0; uvOffsetY = 0.25f; return 5; }
            else if (Combo >= 50) { uvOffsetX = 0.25f; uvOffsetY = 0.25f; return 6; }
            else { return 1; };
        }
        else {
            if (Combo < 10) { uvOffsetX = 0; uvOffsetY = 0; return 1; }
            else if (Combo < 20) { uvOffsetX = 0.25f; uvOffsetY = 0; return 2; }
            else if (Combo < 30) { uvOffsetX = 0.5f; uvOffsetY = 0; return 3; }
            else if (Combo >= 30) { uvOffsetX = 0.75f; uvOffsetY = 0; return 4; }
            else { return 1; }
        };
    }

    bool IsBassOrVox(){
        if (Instrument == 0 || Instrument == 2 || Instrument == 4 || Instrument == 6) {
            return false;
        } else {
            return true;
        }

    }

    float comboFillCalc() {
        if (Combo == 0) {
            return 0;
        }
        if (Instrument == 0 || Instrument == 2 || Instrument == 4 || Instrument == 6) {
            // For instruments 0 and 2, limit the float value to 0.0 to 0.4
            if (Combo >= 30) {
                return 1.0f; // If combo is 30 or more, set float value to 1.0
            }
            else {
                int ComboMod = Combo % 10;
                if (ComboMod == 0)
                    return 1.0f;
                else {
                    return (static_cast<float>(ComboMod) / 10.0f); // Float value from 0.0 to 0.9 every 10 notes
                }
            }
        }
        else {
            if (Combo >= 50) {
                return 1.0f; // If combo is 30 or more, set float value to 1.0
            }
            // For instruments 1 and 3, limit the float value to 0.0 to 0.6
            int ComboMod = Combo % 10;
            if (ComboMod == 0)
                return 1.0f;
            else {
                return (static_cast<float>(ComboMod) / 10.0f); // Float value from 0.0 to 0.9 every 10 notes
            }
        }
    }

};

class Player {
public:
    Player();
    Color AccentColor = {255,0,255,255};
    std::string Name;       // display name
    std::string PlayerID;   // UUID
    // std::filesystem::path SettingsFile;
    PlayerGameplayStats* stats;
    int Difficulty;
    int Instrument;
    float InputCalibration = 0.0f;
    float NoteSpeed = 1.0f;
    bool ClassicMode;
    bool ProDrums;
    bool ReadiedUpBefore;
    bool Bot;
    int SongsPlayed;
    int joypadID;
    bool LeftyFlip;
    bool Online;
    int ActiveSlot;
    int HighwayLength;

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
    std::vector<int> OverdriveMultiplier{1,2,4,6,8};
    int PlayersInOverdrive = 0;
    void AddNotePoint(bool perfect, int playerMult);
    void DrumNotePoint(bool perfect, int playerMult, bool cymbal);
};

class PlayerManager {
    PlayerManager() {}
public:

    static PlayerManager& getInstance() {
        static PlayerManager instance; // This is the single instance
        return instance;
    }

    // Delete copy constructor and assignment operator
    PlayerManager(const PlayerManager&) = delete;
    void operator=(const PlayerManager&) = delete;

    void MakePlayerDirectory(); // run on initialization?
    void LoadPlayerList(std::filesystem::path PlayerListSaveFile); // make player, load player stuff to PlayerList
    void SavePlayerList(std::filesystem::path PlayerListSaveFile);
    BandGameplayStats BandStats = BandGameplayStats();
    rapidjson::Document PlayerListFile;
    std::vector<Player> PlayerList;
    std::vector<int> ActivePlayers{-1,-1,-1,-1};
    int PlayersActive = 0;

    Player* GetActivePlayer(int slot) {
        return &PlayerList[ActivePlayers[slot]];
    }

    void AddActivePlayer(int playerNum, int slot) {
        ActivePlayers[slot] = playerNum;
        PlayersActive += 1;
    }

    void RemoveActivePlayer(int slot) {
        ActivePlayers[slot] = -1;
        PlayersActive -= 1;
    }

    Player* GetPlayerGamepad(int joystickID) {
        for (int playesr = 0 ; playesr < PlayersActive; playesr++) {
            if (GetActivePlayer(playesr)->joypadID == joystickID) {
                return GetActivePlayer(playesr);
            }
        }
        return nullptr;
    }

    void CreatePlayer(Player player);
    void DeletePlayer(Player PlayerToDelete); // remove player, reload playerlist
    void RenamePlayer(Player PlayerToRename); // rename player
};



#endif //ENCORE_PLAYER_H
