#pragma once
//
// Created by marie on 09/06/2024.
//

#include <utility>
#include "game/player.h"

class gameplayRenderer {
    void RenderNotes(Player& player, Chart& curChart, double time, RenderTexture2D& notes_tex, float length);
    void RenderHud(Player& player, RenderTexture2D&, float);
    void RenderExpertHighway(Player& player, Song song, double time, RenderTexture2D& highway_tex, RenderTexture2D& highwayStatus_tex, RenderTexture2D& smasher_tex);
    void RenderEmhHighway(Player& player, Song song, double time, RenderTexture2D& highway_tex);
    void RenderPDrumsHighway(Player& player, Song song, double time, RenderTexture2D& highway_tex);

    void DrawBeatlines(Player& player, Song song, float length, double musicTime);
    void DrawOverdrive(Player& player, Chart& curChart, float length, double musicTime);
    void DrawSolo(Player& player,  Chart& curChart, float length, double musicTime);
    void RenderClassicNotes(Player& player, Chart& curChart, double time, RenderTexture2D &notes_tex, float length);
    void RenderPDrumsNotes(Player& player, Chart& curChart, double time, RenderTexture2D& notes_tex, float length);
public:
    std::vector<bool> heldFrets{ false,false,false,false,false };
    std::vector<bool> heldFretsAlt{ false,false,false,false,false };
    std::vector<bool> overhitFrets{ false,false,false,false,false };
    std::vector<bool> tapRegistered{ false,false,false,false,false };
    std::vector<bool> liftRegistered{ false,false,false,false,false };
    bool bot = false;
    bool proDrum = false;
    bool dblKick = false;
    double startTime = 0.0;
	double songStartTime = 0.0;
    bool highwayInAnimation = false;
    bool highwayInEndAnim = false;
    bool highwayOutAnimation = false;
    bool highwayOutEndAnim = false;
    float animDuration = 1.0f;
    float highwayLevel = 0;
    std::vector<int> curNoteIdx = { 0,0,0,0,0 };
    bool songEnded = false;
    bool overstrum = false;
    int selectedSongInt = 0;
    bool showHitwindow = false;
    int curBPM = 0;
    int curBeatLine = 0;
    int curODPhrase = 0;
    int curSolo = 0;
    int curNoteInt = 0;
    bool songOver = false;
	bool extendedSustainActive = false;
	float textureOffset = 0;
	float renderPos = 0;
	int cameraSel = 0;
    Mesh sustainPlane;
    Mesh soloPlane;

    Camera3D camera = { 0 };
	Camera3D camera1 = { 0 };
	Camera3D camera2 = { 0 };
	Camera3D camera3 = { 0 };
	/*
	gpr.camera.position = Vector3{ 0.0f, 7.0f, -10.0f };
	// 0.0f, 0.0f, 6.5f
	gpr.camera.target = Vector3{ 0.0f, 0.0f, 13.0f };
	 */
	std::vector<Camera3D> camera3pVector;

    void RenderGameplay(Player& player, double time, Song song, RenderTexture2D&, RenderTexture2D&, RenderTexture2D&, RenderTexture2D&, RenderTexture2D&);

    bool upStrum = false;
    bool downStrum = false;
    bool FAS = false;
    bool processingStrum = false;

    void RaiseHighway();

    void LowerHighway();
};

