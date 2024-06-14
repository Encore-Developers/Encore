#pragma once
//
// Created by marie on 09/06/2024.
//

#include <utility>
#include "game/player.h"

class gameplayRenderer {
    void RenderNotes(Player& player, Chart& curChart, double time, RenderTexture2D& notes_tex, float length);
    void RenderHud(Player& player, RenderTexture2D&);
    void RenderExpertHighway(Player& player, Song song, double time, RenderTexture2D& highway_tex);
    void RenderEmhHighway(Player& player, Song song, double time, RenderTexture2D& highway_tex);
    void DrawBeatlines(Player& player, Song song, float length, double musicTime);
    void DrawOverdrive(Player& player, Chart& curChart, float length, double musicTime);
    void DrawSolo(Player& player,  Chart& curChart, float length, double musicTime);
    void RenderClassicNotes(Player& player, Chart& curChart, double time, RenderTexture2D &notes_tex, float length);
public:
    std::vector<bool> heldFrets{ false,false,false,false,false };
    std::vector<bool> heldFretsAlt{ false,false,false,false,false };
    std::vector<bool> overhitFrets{ false,false,false,false,false };
    std::vector<bool> tapRegistered{ false,false,false,false,false };
    std::vector<bool> liftRegistered{ false,false,false,false,false };
    std::vector<int> curNoteIdx = { 0,0,0,0,0 };
    bool songEnded = false;
    bool overstrum = false;
    int selectedSongInt = 0;
    int curBPM = 0;
    int curBeatLine = 0;
    int curODPhrase = 0;
    int curSolo = 0;
    int curNoteInt = 0;

    Mesh sustainPlane;

    Camera3D camera = { 0 };

    void RenderGameplay(Player& player, double time, Song song, RenderTexture2D&, RenderTexture2D&, RenderTexture2D&);

    bool upStrum = false;
    bool downStrum = false;
};

