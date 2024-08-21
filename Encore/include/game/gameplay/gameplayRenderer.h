#pragma once
//
// Created by marie on 09/06/2024.
//

#include <utility>
#include "game/users/player.h"
#include "song/song.h"

class gameplayRenderer {
    void RenderNotes(Player* player, Chart& curChart, double time, RenderTexture2D& notes_tex, float length);
    void RenderHud(Player* player, RenderTexture2D&, float);
    void RenderExpertHighway(Player* player, Song song, double time, RenderTexture2D& highway_tex, RenderTexture2D& highwayStatus_tex, RenderTexture2D& smasher_tex);
	void RenderPDrumsHighway(Player* player, Song song, double time, RenderTexture2D& highway_tex, RenderTexture2D& highwayStatus_tex, RenderTexture2D& smasher_tex);
    void RenderEmhHighway(Player* player, Song song, double time, RenderTexture2D& highway_tex);
    void DrawBeatlines(Player* player, Song song, float length, double musicTime);
    void DrawOverdrive(Player* player, Chart& curChart, float length, double musicTime);
    void DrawSolo(Player* player,  Chart& curChart, float length, double musicTime);

	void DrawFill(Player *player, Chart &curChart, float length, double musicTime);

	void RenderClassicNotes(Player* player, Chart& curChart, double time, RenderTexture2D &notes_tex, float length);
	void RenderPDrumsNotes(Player* player, Chart& curChart, double time, RenderTexture2D& notes_tex, float length);
public:
	float highwayLevel = 0;
	float smasherPos = 2.4f;
	float HitAnimDuration = 0.15f;
	bool highwayInAnimation = false;
	bool highwayInEndAnim = false;
	bool highwayOutAnimation = false;
	bool highwayOutEndAnim = false;
	float animDuration = 1.0f;
    bool songEnded = false;
    bool overstrum = false;
    int selectedSongInt = 0;
	bool songPlaying = false;
    bool showHitwindow = false;

    bool songOver = false;
	bool extendedSustainActive = false;
	float textureOffset = 0;
	double audioStartTime = 0.0;
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
	double startTime = 0.0;

	std::vector<Camera3D> camera3pVector;

    void RenderGameplay(Player* player, double time, Song song, RenderTexture2D&, RenderTexture2D&, RenderTexture2D&, RenderTexture2D&, RenderTexture2D&);

    bool upStrum = false;
    bool downStrum = false;
    bool FAS = false;
    bool processingStrum = false;

    void RaiseHighway();

    void LowerHighway();

	void NoteMultiplierEffect(double time, double hitTime, bool miss, Player& player);
	double multiplierEffectTime = 1.0;
};

