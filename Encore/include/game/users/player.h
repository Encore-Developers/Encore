//
// Created by marie on 04/05/2024.
//

#ifndef ENCORE_PLAYER_H
#define ENCORE_PLAYER_H

#include <string>
#include <filesystem>
#include <map>

#include "raylib.h"
#include "bass/bass.h"
#include "libstud-uuid/uuid/uuid.hxx"
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

class PlayerGameplayStats {
public:
    PlayerGameplayStats(int inst, int diff);

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
    float Health;
    int MaxNotes;

    float overdriveFill;
    float overdriveActiveFill;
    double overdriveActiveTime;
    double overdriveActivateTime;

    int Instrument;
    int Difficulty;
    int BaseScore;
    float xStarThreshold[6] = { 0.05f, 0.175f, 0.325f, 0.5f, 0.7f,  1.0f };

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



    float comboFillCalc() {
        if (Instrument == 0 || Instrument == 2 || Instrument == 4 || Instrument == 6) {
            // For instruments 0 and 2, limit the float value to 0.0 to 0.4
            if (Combo >= 30) {
                return 1.0f; // If combo is 30 or more, set float value to 1.0
            }
            else {
                return static_cast<float>(Combo % 10) / 10.0f; // Float value from 0.0 to 0.9 every 10 notes
            }
        }
        else {
            // For instruments 1 and 3, limit the float value to 0.0 to 0.6
            if (Combo >= 50) {
                return 1.0f; // If combo is 50 or more, set float value to 1.0
            }
            else {
                return static_cast<float>(Combo % 10) / 10.0f; // Float value from 0.0 to 0.9 every 10 notes
            }
        }
    }

};

// acts as an individual save-file
class Player {
public:
    Player(std::string name);

    std::string Name;       // display name
    std::string PlayerID;   // UUID
    std::filesystem::path SettingsFile;
    std::filesystem::path ScoreFile;
    PlayerGameplayStats stats;
    int Difficulty;
    int Instrument;
    float InputCalibration = 0.0f;
    float NoteSpeed = 1.0f;
    bool ClassicMode;
    bool ReadiedUpBefore;
    bool Bot;
    int SongsPlayed;
    bool LeftyFlip;
    bool Online;
    int ActiveSlot;
    bool ResetGameplayStats();
                    // zero indexed. local would be 0-3, online would be 4-7.
                   // NOTE! this is only for like. local information and
                   // not actually shared information. i was thinking of a UUID system for online
};

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

class PlayerManager {
    bool MakePlayerDirectory(); // run on initialization?
    int LoadPlayerList(std::string PlayerListSaveFile); // make player, load player stuff to PlayerList
    int SavePlayerList(std::string PlayerListSaveFile);
    std::vector<Player> PlayerList;
    std::vector<Player*> ActivePlayers;

    Player* GetActivePlayer(int slot) {
        return ActivePlayers[slot];
    }

    void AddActivePlayer(Player* player) {
        ActivePlayers.push_back(player);
        player->ActiveSlot = 0;
    }

    void RemoveActivePlayer(int slot) {
        ActivePlayers.erase(ActivePlayers.begin() + slot);
    }

    bool CreatePlayer(std::string PlayerName); // set it as the next one in PlayerList
    bool DeletePlayer(Player PlayerToDelete); // remove player, reload playerlist
    bool RenamePlayer(Player PlayerToRename); // rename player
};



#endif //ENCORE_PLAYER_H
