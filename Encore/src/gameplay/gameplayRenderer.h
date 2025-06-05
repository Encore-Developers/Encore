#pragma once
//
// Created by marie on 09/06/2024.
//

#include <utility>
#include "users/player.h"
#include "song.h"



class gameplayRenderer {
public:
    void ProcessSustainScoring(
        int lane,
        double beatsLen,
        double heldTime,
        double lenTime,
        bool perfect,
        PlayerGameplayStats *&stats
    );
    void AddSustainPoints(int lane, PlayerGameplayStats *&stats);
    float GetNoteXPosition(Player &player, float diffDistance, int lane);
    void RenderPadNotes(Player &player, Chart &curChart, double curSongTime, float length);
    void RenderHud(Player &player, float length);
    void RenderExpertHighway(
        Player &player,
        Song song,
        double time
    );
    void RenderPDrumsHighway(Player &player, Song song, double curSongTime);
    void DrawHighwayMesh(
        float LengthMultiplier, bool Overdrive, float ActiveTime, float SongTime, bool EMH
    );
    void StartRenderTexture();
    void DrawSmashers(Player &player);

    void
    RenderEmhHighway(Player &player, Song song, double time);
    void DrawBeatlines(Player &player, Song &song, float length, double musicTime);
    void DrawOverdrive(Player &player, Chart &curChart, float length, double musicTime);
    void DrawSolo(Player &player, Chart &curChart, float length, double musicTime);

    void DrawFill(Player &player, Chart &curChart, float length, double musicTime);
    void DrawCoda(float length, double musicTime, Player &player);

    void CheckPlasticNotes(
        Player &player,
        Chart &curChart,
        double curSongTime,
        int curNote
    );
    void CalculateSustainScore(PlayerGameplayStats *&stats);
    void RenderClassicNotes(Player &player, Chart &curChart, double curSongTime, float length);
    void DrawHitwindow(Player &player, float length);
    void RenderPDrumsNotes(Player &player, Chart &curChart, double curSongTime, float length);

    void nDrawDrumsHitEffects(Player &player, Note note, double curSongTime, float notePosX);
    void nDrawFiveLaneHitEffects(
        Player &player, Note note, double curSongTime, float notePosX, int lane
    );
    void
    nDrawPlasticNote(
        Note note, Color accentColor, Color noteColor, float notePosX, float noteTime
    );
    void nDrawPadNote(Note note, Color noteColor, float notePosX, float noteScrollPos);
    void nDrawSustain(
        Note note,
        Color noteColor,
        float notePosX,
        float length,
        float relTime,
        float relEnd
    );
    void nDrawCodaLanes(
        float length,
        double sTime,
        double cLen,
        double curTime,
        float NoteSpeed,
        int Difficulty
    );
    void nDrawFiveLaneUnderlay(float length, bool pad, Player &player);

    void nDrawSoloSides(
        float length,
        double sTime,
        double cLen,
        double curTime,
        float NoteSpeed,
        int Difficulty,
        bool Classic
    );

    void eDrawSides(
        float scrollPos,
        double time,
        double start,
        double end,
        float length,
        double radius,
        Color color,
        bool emh
    );

    double GetNoteOnScreenTime(
        double noteTime, double songTime, float noteSpeed, int Difficulty, float length
    );
    void DrawPerfectText(double noteTime, double songTime, Player &player);
    void DrawCombo(double noteTime, double songTime, Player &player);
    double HighwaySpeedDifficultyMultiplier(int Difficulty);
    float MaxHighwaySpeed = 1.25f;
    float MinHighwaySpeed = 0.5f;
    RenderTexture2D GameplayRenderTexture;
public:
    gameplayRenderer();
    ~gameplayRenderer();
    double CurrentTick = 0.0;
    float highwayLevel = 0;
    float smasherPos = 2.4f;
    float HitAnimDuration = 0.15f;
    double OverdriveAnimationDuration = 0.75f;
    bool highwayInAnimation = false;
    bool highwayInEndAnim = false;
    bool highwayOutAnimation = false;
    bool highwayOutEndAnim = false;
    float animDuration = 1.0f;
    bool overstrum = false;
    int selectedSongInt = 0;
    bool songPlaying = false;
    bool showHitwindow = false;

    bool streamsLoaded = false;
    bool midiLoaded = false;

    float Back4p = -12.0f;
    float Height4p = 10.0f;
    float Height = 7.25f;
    float Back = -10.0f;
    float Height1p = 7.0f;
    float Back1p = -11.0f;
    float FOV1p = 40.0f;
    float FOV = 45.0f;
    bool extendedSustainActive = false;
    float textureOffset = 0;
    float renderPos = 0;
    int cameraSel = 0;
    Mesh sustainPlane;
    Mesh dividerPlane;
    Mesh soloPlane;
    Texture2D invSoloTex;
    Texture2D dividerTex[3];
    std::vector<std::vector<Camera3D>> cameraVectors;



    /*
    gpr.camera.position = Vector3{ 0.0f, 7.0f, -10.0f };
    // 0.0f, 0.0f, 6.5f
    gpr.camera.target = Vector3{ 0.0f, 0.0f, 13.0f };
     */

    void
    RenderGameplay(Player &player, double curSongTime, Song song);
    enum ModelVectorEnum {
        mHOPO,
        mLIFT,
        mOPEN,
        mSTRUM,
        mTAP,
        mTOM
    };
    enum ModelPartEnum {
        mBASE,
        mCOLOR,
        mSIDES,
        mINSIDE
    };
    std::vector<std::filesystem::path> NoteFilenames {
        "base.obj",
        "color.obj",
        "sides.obj",
        "tInside.obj"
    };

    std::vector<Model> HopoParts;
    std::vector<Model> LiftParts;
    std::vector<Model> OpenParts;
    std::vector<Model> StrumParts;
    std::vector<Model> TapParts;
    std::vector<Model> DrumParts;
    std::vector<Model> CymbalParts;

    Model InnerKickSmasher;
    Model OuterKickSmasher;
    Model InnerTomSmasher;
    Model OuterTomSmasher;
    Texture2D InnerKickSmasherTex;
    Texture2D OuterKickSmasherTex;
    /*
     *  for (auto model : TapParts) {
     *      DrawModel(model, pos, 1.0, color);
     *  }
     *
     *  probably good to use a vector for note colors
     */
    void LoadGameplayAssets();
    bool upStrum = false;
    bool downStrum = false;
    bool FAS = false;
    bool processingStrum = false;
    bool Restart = false;

    void RaiseHighway();

    void LowerHighway();

    void NoteMultiplierEffect(double curSongTime, Player &player);
    void DrawRenderTexture();
    double multiplierEffectTime = 1.0;
};

extern gameplayRenderer TheGameRenderer;
