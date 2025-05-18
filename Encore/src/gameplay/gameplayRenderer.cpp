#include "menus/MenuManager.h"

#include <array>
//
// Created by marie on 09/06/2024.
//
float diffDistance(int diff, bool classic) {
    return diff == 3 || classic ? 2.0f : 1.5f;
};
float lineDistance = 1.5f;

#include "gameplayRenderer.h"

#include "assets.h"
#include "enctime.h"
#include "menus/gameMenu.h"
#include "raymath.h"
#include "menus/uiUnits.h"
#include "rlgl.h"
#include "easing/easing.h"
#include "song/audio.h"
#include "users/playerManager.h"
#include "util/enclog.h"
#include "util/raylib-3d-text.h"

Assets &gprAssets = Assets::getInstance();
Units &gprU = Units::getInstance();

// Color accentColor = {255,0,255,255};
float defaultHighwayLength = 11.5f;
Color OverdriveColor = { 255, 200, 0, 255 };

std::vector<Color> GRYBO = { GREEN, RED, YELLOW, BLUE, ORANGE };
std::vector<Color> TRANS = { SKYBLUE, PINK, WHITE, PINK, SKYBLUE };

void BeginBlendModeSeparate() {
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
}

enum DrumNotes {
    KICK,
    RED_DRUM,
    YELLOW_DRUM,
    BLUE_DRUM,
    GREEN_DRUM
};

double startTime = 0.0;
bool highwayRaiseFinish = false;
Camera3D CreateCamera(Vector3 position, Vector3 target, float FOV) {
    Camera3D camera;
    camera.position = position;
    camera.target = target;
    camera.up = Vector3 { 0.0f, 1.0f, 0.0f };
    camera.projection = CAMERA_PERSPECTIVE;
    camera.fovy = FOV;
    return camera;
}

double GetNotePos(double noteTime, double songTime, float noteSpeed, float length) {
    return ((noteTime - songTime) * (noteSpeed * (length / 2))) + 2.4f;
}

/* double TimeRangeToTickDelta(double timeStart, double timeEnd, BPM bpm) {
    double timeDelta = timeEnd - timeStart;
    double beatDelta = timeDelta * bpm.bpm / 60.0;
    return beatDelta * 480.0;
}
*/
unsigned char
TickToChar(int tick, int MinBrightness, int MaxBrightness, int QuarterNoteLength) {
    float TickModulo = tick % QuarterNoteLength;
    return Remap(
        TickModulo / float(QuarterNoteLength), 0, 1.0f, MaxBrightness, MinBrightness
    );
}

// this is kind of a. "replacement" for loading everything in Assets
// technically i should be putting this in another class but you know
// maybe i should make a struct for a "note model" so its easier to do this
// because goddamn is this gonna get tiring
// especially if each thing has *around* three models. THREE. god fucking damnnit
void gameplayRenderer::LoadGameplayAssets() {
    // PLEASE CLEAN UP THE HORRORS.
    // PLEASE CLEAN UP THE HORRORS.
    // PLEASE CLEAN UP THE HORRORS.
    // PLEASE CLEAN UP THE HORRORS.
    // PLEASE CLEAN UP THE HORRORS.
    // PLEASE CLEAN UP THE HORRORS.
    // PLEASE CLEAN UP THE HORRORS.

    std::filesystem::path noteModelPath = gprAssets.getDirectory();
    noteModelPath /= "Assets/noteredux";
    std::filesystem::path highwayModelPath = gprAssets.getDirectory();
    highwayModelPath /= "Assets/gameplay/highway";

    GameplayRenderTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    sustainPlane = GenMeshPlane(0.8f, 1.0f, 1, 1);
    dividerPlane = GenMeshPlane(1.0f, 1.0f, 1, 1);
    soloPlane = GenMeshPlane(1.0f, 1.0f, 1, 1);
    Image invSolo = LoadImageFromTexture(gprAssets.soloTexture);
    ImageFlipHorizontal(&invSolo);
    invSoloTex = LoadTextureFromImage(invSolo);
    SetTextureWrap(invSoloTex, TEXTURE_WRAP_CLAMP);
    SetTextureWrap(gprAssets.soloTexture, TEXTURE_WRAP_CLAMP);
    UnloadImage(invSolo);

    std::filesystem::path dividerPath = gprAssets.getDirectory();
    dividerPath /= "Assets/gameplay/highway/dividers";
    dividerTex[0] = LoadTexture((dividerPath / "left.png").string().c_str());
    dividerTex[1] = LoadTexture((dividerPath / "center.png").string().c_str());
    dividerTex[2] = LoadTexture((dividerPath / "right.png").string().c_str());

    Texture2D NoteSide = LoadTexture((noteModelPath / "NoteSide.png").string().c_str());
    Texture2D NoteColor = LoadTexture((noteModelPath / "NoteColor.png").string().c_str());
    Texture2D NoteBottom =
        LoadTexture((noteModelPath / "NoteBottom.png").string().c_str());
    Texture2D HopoSide = LoadTexture((noteModelPath / "HopoSides.png").string().c_str());
    Texture2D LiftSide = LoadTexture((noteModelPath / "LiftSides.png").string().c_str());
    Texture2D LiftBase = LoadTexture((noteModelPath / "LiftBase.png").string().c_str());
    // hopo
    Model BaseHopo = LoadModel((noteModelPath / "hopo/base.obj").string().c_str());
    BaseHopo.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteBottom;
    Model ColorHopo = LoadModel((noteModelPath / "hopo/color.obj").string().c_str());
    ColorHopo.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteColor;
    Model SidesHopo = LoadModel((noteModelPath / "hopo/sides.obj").string().c_str());
    SidesHopo.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = HopoSide;
    HopoParts.push_back(std::move(BaseHopo));
    HopoParts.push_back(std::move(ColorHopo));
    HopoParts.push_back(std::move(SidesHopo));

    // lift
    Model LiftSides = LoadModel((noteModelPath / "lift/sides.obj").string().c_str());
    LiftSides.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LiftSide;
    Model LiftColor = LoadModel((noteModelPath / "lift/color.obj").string().c_str());
    LiftColor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LiftBase;
    LiftParts.push_back(std::move(LiftSides));
    LiftParts.push_back(std::move(LiftColor));

    // open
    Model BaseOpen = LoadModel((noteModelPath / "open/base.obj").string().c_str());
    BaseOpen.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteBottom;
    Model ColorOpen = LoadModel((noteModelPath / "open/color.obj").string().c_str());
    ColorOpen.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteColor;
    Model SidesOpen = LoadModel((noteModelPath / "open/sides.obj").string().c_str());
    SidesOpen.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteSide;
    OpenParts.push_back(std::move(BaseOpen));
    OpenParts.push_back(std::move(ColorOpen));
    OpenParts.push_back(std::move(SidesOpen));

    // strum
    Model StrumBase = LoadModel((noteModelPath / "strum/base.obj").string().c_str());
    StrumBase.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteBottom;
    Model StrumColor = LoadModel((noteModelPath / "strum/color.obj").string().c_str());
    StrumColor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteColor;
    Model StrumSides = LoadModel((noteModelPath / "strum/sides.obj").string().c_str());
    StrumSides.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteSide;
    StrumParts.push_back(std::move(StrumBase));
    StrumParts.push_back(std::move(StrumColor));
    StrumParts.push_back(std::move(StrumSides));

    // Tap
    Model TapBase = LoadModel((noteModelPath / "tap/base.obj").string().c_str());
    TapBase.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteBottom;
    Model TapColor = LoadModel((noteModelPath / "tap/color.obj").string().c_str());
    TapColor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteColor;
    Model TapSides = LoadModel((noteModelPath / "tap/sides.obj").string().c_str());
    TapSides.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteSide;
    Model TapInside = LoadModel((noteModelPath / "tap/tInside.obj").string().c_str());
    TapParts.push_back(std::move(TapBase));
    TapParts.push_back(std::move(TapColor));
    TapParts.push_back(std::move(TapSides));
    TapParts.push_back(std::move(TapInside));

    // Tom
    Model TomBase = LoadModel((noteModelPath / "tom/base.obj").string().c_str());
    TomBase.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteBottom;
    Model TomColor = LoadModel((noteModelPath / "tom/color.obj").string().c_str());
    TomColor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteColor;
    Model TomSides = LoadModel((noteModelPath / "tom/sides.obj").string().c_str());
    TomSides.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = NoteSide;
    DrumParts.push_back(std::move(TomBase));
    DrumParts.push_back(std::move(TomColor));
    DrumParts.push_back(std::move(TomSides));

    // Crash
    Texture2D CymbalBaseTex =
        LoadTexture((noteModelPath / "CymbalBase.png").string().c_str());
    Texture2D CymbalColorTex =
        LoadTexture((noteModelPath / "CymbalColor.png").string().c_str());
    Texture2D CymbalSidesTex =
        LoadTexture((noteModelPath / "CymbalWhite.png").string().c_str());
    Model CymbalBase = LoadModel((noteModelPath / "cymbal/base.obj").string().c_str());
    CymbalBase.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = CymbalBaseTex;
    Model CymbalColor = LoadModel((noteModelPath / "cymbal/color.obj").string().c_str());
    CymbalColor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = CymbalColorTex;
    Model CymbalSides = LoadModel((noteModelPath / "cymbal/sides.obj").string().c_str());
    CymbalSides.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = CymbalSidesTex;
    CymbalParts.push_back(std::move(CymbalBase));
    CymbalParts.push_back(std::move(CymbalColor));
    CymbalParts.push_back(std::move(CymbalSides));

    InnerKickSmasher =
        LoadModel((highwayModelPath / "DrumSmasherInner.obj").string().c_str());
    OuterKickSmasher =
        LoadModel((highwayModelPath / "DrumSmasherOuter.obj").string().c_str());
    InnerTomSmasher =
        LoadModel((highwayModelPath / "DrumSmasherTomInner.obj").string().c_str());
    OuterTomSmasher =
        LoadModel((highwayModelPath / "DrumSmasherTomOuter.obj").string().c_str());
    InnerKickSmasherTex = Assets::LoadTextureFilter(
        highwayModelPath / "DrumSmasherInner.png", gprAssets.loadedAssets
    );
    OuterKickSmasherTex = Assets::LoadTextureFilter(
        highwayModelPath / "DrumSmasherOuter.png", gprAssets.loadedAssets
    );
    InnerKickSmasher.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = InnerKickSmasherTex;
    OuterKickSmasher.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = OuterKickSmasherTex;
    OuterTomSmasher.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
        gprAssets.smasherOuterTex;
    // Y UP!!!! REMEMBER!!!!!!
    //							  x,    y,     z
    //                         0.0f, 5.0f, -3.5f
    //								 6.5f

    // singleplayer
    // 0.0f, 0.0f, 6.5f
    /*
     * basically this is just. a vector for the vectors. its really nasty as is, but you
     * know. its much more compact than whatever was here before though
     */
    float CameraPos = 0.75f;
    std::vector<std::vector<Vector3> > CameraDisplacement = {
        { { 0, Height1p, Back1p } },

        { { 0.75f, Height, Back }, { -0.75f, Height, Back } },

        { { CameraPos, Height, Back }, { 0, Height, Back }, { -CameraPos, Height, Back } },

        { { 3.0f, Height4p, Back4p },
          { 1.0f, Height4p, Back4p },
          { -1.0f, Height4p, Back4p },
          { -3.0f, Height4p, Back4p } }
    };
    float TargetDistance = 20.0f;
    std::array<std::array<Vector3, 4>, 4> CameraTargetDisplacement {
        { { { { 0, 0, TargetDistance } } },

          { { { 0.75f, 0, TargetDistance }, { -0.75f, 0, TargetDistance } } },

          { { { CameraPos * 3, 0, TargetDistance },
              { 0, 0, TargetDistance },
              { -CameraPos * 3, 0, TargetDistance } } },

          { { { 3.0f, 0, TargetDistance },
              { 1.0f, 0, TargetDistance },
              { -1.0f, 0, TargetDistance },
              { -3.0f, 0, TargetDistance } } } }
    };
    std::array CameraFOV { FOV1p, FOV, FOV, FOV };
    for (int cam = 0; cam < 4; cam++) {
        std::vector<Camera3D> cameraVec;
        for (int pos = 0; pos <= cam; pos++) {
            cameraVec.push_back(CreateCamera(
                CameraDisplacement.at(cam).at(pos),
                CameraTargetDisplacement.at(cam).at(pos),
                CameraFOV.at(cam)
            ));
        }
        cameraVectors.push_back(cameraVec);
    }

    /*
    camera1pVector.push_back(CreateCamera(CameraDisplacement[0][0],
    CameraTargetDisplacement[0][0], FOV));

    // 2 player
    float SideDisplacement2p = 0.75f;

    camera2pVector.push_back(CreateCamera(CameraDisplacement[1][0],
    CameraTargetDisplacement[1][0], FOV));
    camera2pVector.push_back(CreateCamera(CameraDisplacement[1][1],
    CameraTargetDisplacement[1][1], FOV));

    // 3 player
    float SideDisplacement3p = 1.25f;
    camera3pVector.push_back(CreateCamera(CameraDisplacement[2][0],
    CameraTargetDisplacement[2][0], FOV));
    camera3pVector.push_back(CreateCamera(CameraDisplacement[2][1],
    CameraTargetDisplacement[2][1], FOV));
    camera3pVector.push_back(CreateCamera(CameraDisplacement[2][2],
    CameraTargetDisplacement[2][2], FOV));

    // 4 player
    camera4pVector.push_back(CreateCamera(CameraDisplacement[3][0],
    CameraTargetDisplacement[3][0], FOV));
    camera4pVector.push_back(CreateCamera(CameraDisplacement[3][1],
    CameraTargetDisplacement[3][1], FOV));
    camera4pVector.push_back(CreateCamera(CameraDisplacement[3][2],
    CameraTargetDisplacement[3][2], FOV));
    camera4pVector.push_back(CreateCamera(CameraDisplacement[3][3],
    CameraTargetDisplacement[3][3], FOV));

    float SideDisplacement4p = 3.0f;
    float SideDisplacement4p2 = 1.0f;
    float Back4p = -10.0f;
    float Height4p = 10.0f;

    cameraVectors.push_back(camera1pVector);
    cameraVectors.push_back(camera2pVector);
    cameraVectors.push_back(camera3pVector);
    cameraVectors.push_back(camera4pVector);
    */
}

void gameplayRenderer::RaiseHighway() {
    if (!highwayInAnimation) {
        startTime = GetTime();
        highwayRaiseFinish = false;
        highwayInAnimation = true;
    }
    if (GetTime() <= startTime + animDuration && highwayInAnimation) {
        double timeSinceStart = GetTime() - startTime;
        float target = Remap(
            1.0 - getEasingFunction(EaseInExpo)(timeSinceStart / animDuration),
            0,
            1.0,
            20,
            50
        );
        float position = Remap(
            1.0 - getEasingFunction(EaseInExpo)(timeSinceStart / animDuration),
            0,
            1.0,
            Back,
            Back + 30
        );
        float fov = Remap(1.0 - (timeSinceStart / animDuration), 0, 1.0, 45, 50);
        cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel].target.z = target;
        cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel].position.z =
            position;
        cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel].fovy = fov;

        highwayRaiseFinish = true;
    } else {
        cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel].target.z = 20;
        cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel].position.z = Back;
        cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel].fovy = 45;
    }
    if (GetTime() >= startTime + animDuration && highwayRaiseFinish) {
        highwayInEndAnim = true;
    }
};

void gameplayRenderer::LowerHighway() {
    /*
    if (!highwayOutAnimation) {
        startTime = GetTime();
        highwayOutAnimation = true;
    }
    if (GetTime() <= TheSongTime.GetStartTime() + animDuration && highwayOutAnimation) {
        double timeSinceStart = GetTime() - startTime;
        highwayLevel = Remap(
            1.0 - getEasingFunction(EaseInExpo)(timeSinceStart / animDuration),
            0,
            1.0,
            -GetScreenHeight(),
            0
        );
        highwayOutEndAnim = true;
    }
    */
};

Color MultiplierEffect(double curSongTime, Player &player) {
    double multiplierEffectTime = 1.0;
    Color RingDefault = ColorBrightness(player.AccentColor, -0.7);
    double TimeHit = player.stats->MultiplierEffectTime;
    bool miss = player.stats->Miss;
    Color ColorForEffect = miss ? RED : WHITE;
    Color ColorToReturn = ColorBrightness(player.AccentColor, -0.7);
    if (curSongTime < TimeHit + multiplierEffectTime && curSongTime > TimeHit) {
        double TimeSinceHit = curSongTime - TimeHit;
        ColorToReturn.r = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / multiplierEffectTime),
            0,
            1.0,
            ColorForEffect.r,
            RingDefault.r
        );
        ColorToReturn.g = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / multiplierEffectTime),
            0,
            1.0,
            ColorForEffect.g,
            RingDefault.g
        );
        ColorToReturn.b = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / multiplierEffectTime),
            0,
            1.0,
            ColorForEffect.b,
            RingDefault.b
        );
    }
    return ColorToReturn;
}

void gameplayRenderer::NoteMultiplierEffect(double curSongTime, Player &player) {
    double TimeHit = player.stats->MultiplierEffectTime;
    bool miss = player.stats->Miss;
    if (curSongTime < TimeHit + multiplierEffectTime && curSongTime > TimeHit) {
        Color missColor = { 200, 0, 0, 255 };
        Color comboColor = { 200, 200, 200, 255 };

        Color RingDefault = ColorBrightness(player.AccentColor, -0.7);
        unsigned char r = RingDefault.r;
        unsigned char g = RingDefault.g;
        unsigned char b = RingDefault.b;
        unsigned char a = 255;
        double TimeSinceHit = curSongTime - TimeHit;
        if (!miss) {
            if (player.stats->Combo <= player.stats->maxMultForMeter() * 10
                && player.stats->Combo % 10 == 0) {
                r = Remap(
                    getEasingFunction(EaseOutQuart)(TimeSinceHit / multiplierEffectTime),
                    0,
                    1.0,
                    comboColor.r,
                    RingDefault.r
                );
                g = Remap(
                    getEasingFunction(EaseOutQuart)(TimeSinceHit / multiplierEffectTime),
                    0,
                    1.0,
                    comboColor.g,
                    RingDefault.g
                );
                b = Remap(
                    getEasingFunction(EaseOutQuart)(TimeSinceHit / multiplierEffectTime),
                    0,
                    1.0,
                    comboColor.b,
                    RingDefault.b
                );
            }
        } else {
            if (player.stats->Combo != 0) {
                r = Remap(
                    getEasingFunction(EaseOutQuart)(TimeSinceHit / multiplierEffectTime),
                    0,
                    1.0,
                    missColor.r,
                    RingDefault.r
                );
                g = Remap(
                    getEasingFunction(EaseOutQuart)(TimeSinceHit / multiplierEffectTime),
                    0,
                    1.0,
                    missColor.g,
                    RingDefault.g
                );
                b = Remap(
                    getEasingFunction(EaseOutQuart)(TimeSinceHit / multiplierEffectTime),
                    0,
                    1.0,
                    missColor.b,
                    RingDefault.b
                );
            }
        }
        gprAssets.MultOuterFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].color = {
            r, g, b, a
        };
    }
}

void gameplayRenderer::DrawRenderTexture() {
    EndMode3D();
    EndBlendMode();
    EndTextureMode();
    SetTextureWrap(GameplayRenderTexture.texture, TEXTURE_WRAP_CLAMP);
    int height = (float)GetScreenHeight();
    int width = (float)GetScreenWidth();
    GameplayRenderTexture.texture.width = width;
    GameplayRenderTexture.texture.height = height;
    Rectangle source = { 0, 0, float(width), float(-height) };
    Rectangle res = { 0, 0, float(GetScreenWidth()), float(GetScreenHeight()) };
    Vector2 shaderResolution = { float(GetScreenWidth()), float(GetScreenHeight()) };

    BeginShaderMode(gprAssets.fxaa);
    // DrawTextureRec(GameplayRenderTexture.texture, res, {0}, WHITE);

    SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, GameplayRenderTexture.texture);
    SetShaderValue(
        gprAssets.fxaa, gprAssets.resLoc, &shaderResolution, SHADER_UNIFORM_VEC2
    );
    DrawTexturePro(
        GameplayRenderTexture.texture, source, res, { renderPos, highwayLevel }, 0, WHITE
    );

    EndShaderMode();
}

void EnableFadeShaderForSmallObjectsThatUseRaylibMeshFuncs() {
    int UseIn = 1;
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &UseIn, SHADER_UNIFORM_INT
    );
    BeginShaderMode(gprAssets.HighwayFade);
}
void DisableFadeShaderForSmallObjectsThatUseRaylibMeshFuncs() {
    int DontIn = 0;
    EndShaderMode();
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT
    );
}
void gameplayRenderer::ProcessSustainScoring(
    int lane,
    double beatsLen,
    double heldTime,
    double lenTime,
    bool perfect,
    PlayerGameplayStats *&stats
) {
    stats->SustainScoreBuffer[lane] = Clamp(
        float(heldTime / lenTime) * (12 * stats->multiplier() * (perfect + 1) * beatsLen),
        0,
        beatsLen * 12 * stats->multiplier() * (perfect + 1)
    );
}

void gameplayRenderer::AddSustainPoints(int lane, PlayerGameplayStats *&stats) {
    stats->Score += stats->SustainScoreBuffer[lane];
    stats->SustainScore += stats->SustainScoreBuffer[lane];
    ThePlayerManager.BandStats->Score += stats->SustainScoreBuffer[lane];
    stats->SustainScoreBuffer[lane] = 0;
}

float gameplayRenderer::GetNoteXPosition(Player &player, float diffDistance, int lane) {
    bool IsPlayerExpert = (player.Difficulty == 3);
    int NotePosAccountingForLefty = player.LeftyFlip ? (IsPlayerExpert + 3) - lane : lane;
    return diffDistance - (1.0f * NotePosAccountingForLefty);
}

void gameplayRenderer::RenderPadNotes(
    Player &player, Chart &curChart, double curSongTime, float length
) {
    float diffDistance = player.Difficulty == 3 ? 2.0f : 1.5f;
    StartRenderTexture();
    // glDisable(GL_CULL_FACE);

    for (int lane = 0; lane < (player.Difficulty == 3 ? 5 : 4); lane++) {
        int NotesToRender = 0;
        for (auto i : curChart.notes_perlane[lane]) {
            // for (int i = player.stats->curNoteIdx[lane] - 1;
            //      i < curChart.notes_perlane[lane].size();
            //      i++) {
            Note &curNote = curChart.notes[i];

            if (curNote.time > TheSongTime.GetSongLength())
                continue;

            if (TheSongList.curSong->BRE.IsNoteInCoda(curNote)) {
                player.stats->curNoteInt++;
                continue;
            }
            if (curNote.time + curNote.len < curSongTime - 2)
                continue;

            double HighwayEnd = length + (smasherPos * 4);
            double NoteStartPositionWorld =
                GetNotePos(curNote.time, curSongTime, player.NoteSpeed, HighwayEnd);
            double NoteEndPositionWorld = GetNotePos(
                curNote.time + curNote.len, curSongTime, player.NoteSpeed, HighwayEnd
            );
            if (NoteStartPositionWorld >= HighwayEnd)
                break;
            NotesToRender++;
            // if (NotesToRender > 60) break;

            Color NoteColor = player.AccentColor;
            // TheGameMenu.hehe && player.Difficulty == 3
            //? TRANS[lane]
            //: player.AccentColor;

            // if (curNote.hit) {
            //	player.stats->totalOffset += curNote.HitOffset;
            // }

            if (!curNote.hit && !curNote.accounted
                && curNote.time + goodBackend < curSongTime
                && !TheSongTime.SongComplete()) {
                curNote.miss = true;
                player.stats->MissNote();
                player.stats->Combo = 0;
                curNote.accounted = true;
                if (player.stats->curNoteIdx[lane]
                    < curChart.notes_perlane[lane].size() - 1)
                    player.stats->curNoteIdx[lane]++;
                Encore::EncoreLog(
                    LOG_INFO,
                    TextFormat(
                        "Missed note at %f, note %01i",
                        curSongTime,
                        player.stats->curNoteInt
                    )
                );
            } else if (player.Bot) {
                if (!curNote.hit && !curNote.accounted && curNote.time < curSongTime
                    && player.stats->curNoteInt < curChart.notes.size()
                    && !TheSongTime.SongComplete()) {
                    curNote.hit = true;
                    player.stats->LastPerfectTime = curNote.hitTime;
                    player.stats->HitNote(false);
                    if (ThePlayerManager.BandStats->Multiplayer) {
                        ThePlayerManager.BandStats->AddNotePoint(
                            curNote.perfect, player.stats->noODmultiplier()
                        );
                    }
                    if (curNote.len > 0)
                        curNote.held = true;
                    curNote.accounted = true;
                    // player.stats->Notes += 1;
                    // player.stats->Combo++;
                    curNote.accounted = true;
                    curNote.hitTime = curSongTime;
                }
            }

            curChart.solos.UpdateEventViaNote(curNote, player.stats->curSolo);
            curChart.sections.UpdateEventViaNote(curNote, player.stats->curSection);
            curChart.fills.UpdateEventViaNote(curNote, player.stats->curFill);
            curChart.overdrive.UpdateEventViaNote(curNote, player.stats->curODPhrase);

            if (curNote.hit && curChart.overdrive.Perfect(player.stats->curODPhrase)) {
                player.stats->overdriveFill +=
                    curChart.overdrive.AddOverdrive(player.stats->curODPhrase);
                if (player.stats->overdriveFill > 1.0f)
                    player.stats->overdriveFill = 1.0f;
            }

            float notePosX = GetNoteXPosition(player, diffDistance, curNote.lane);

            if (NoteStartPositionWorld > HighwayEnd) {
                break;
            }
            if (NoteEndPositionWorld > HighwayEnd)
                NoteEndPositionWorld = HighwayEnd;

            bool SkipShit = false;
            if (NoteEndPositionWorld < -1)
                SkipShit = true;

            if (!SkipShit) {
                nDrawPadNote(curNote, NoteColor, notePosX, NoteStartPositionWorld);
            }
            PlayerGameplayStats *&stats = player.stats;
            if ((curNote.len) > 0 && !SkipShit) {
                if (curNote.held) {
                    NoteStartPositionWorld = smasherPos;
                    curNote.heldTime = curSongTime - curNote.time;
                    if (curNote.heldTime >= curNote.len)
                        curNote.heldTime = curNote.len;
                    CalculateSustainScore(stats);
                    /*
                    ProcessSustainScoring(
                        lane,
                        curNote.beatsLen,
                        curNote.heldTime,
                        curNote.len,
                        curNote.perfect,
                        stats
                    );*/
                    if (!stats->HeldFrets[lane] && !stats->HeldFretsAlt[lane]) {
                        curNote.held = false;
                    }
                    if (curNote.len <= curNote.heldTime) {
                        curNote.held = false;
                    }
                }

                nDrawSustain(
                    curNote,
                    NoteColor,
                    notePosX,
                    length,
                    NoteStartPositionWorld,
                    NoteEndPositionWorld
                );
            }

            nDrawFiveLaneHitEffects(player, curNote, curSongTime, notePosX, lane);
        }
    }
    EndMode3D();

    DrawRenderTexture();
}

double relNow = 0.0;

bool isNoteMatch(const Note &curNote, PlayerGameplayStats *&stats) {
    bool maskGreater = stats->PressedMask >= curNote.mask;
    bool maskEqual = stats->PressedMask == curNote.mask;
    bool maskLess = stats->PressedMask < (curNote.mask * 2);
    if (!curNote.pOpen) {
        bool chordMatch = stats->extendedSustainActive ? maskGreater : maskEqual;
        bool singleMatch =
            stats->extendedSustainActive ? maskGreater : maskGreater && maskLess;
        if (curNote.chord) {
            return chordMatch;
        }
        return singleMatch;
    }
    if (stats->PressedMask > 0) {
        return false;
    }
    return true;
}

void gameplayRenderer::CheckPlasticNotes(
    Player &player, Chart &curChart, double curSongTime, int curNoteInt
) {
    PlayerGameplayStats *&stats = player.stats;
    Note &curNote = curChart.notes.at(curNoteInt);
    if (curNote.hitInFrontend && curNote.isGood(curSongTime, player.InputCalibration)
        && !curNote.miss && !curNote.hit && !curNote.accounted
        && isNoteMatch(curNote, stats)) {
        curNote.cHitNote(curSongTime, player.InputCalibration);
        // TODO: fix for plastic
        stats->HitPlasticNote(curNote);
        ThePlayerManager.BandStats->AddClassicNotePoint(
            curNote.perfect, stats->noODmultiplier(), curNote.chordSize
        );
        if (stats->Combo <= stats->maxMultForMeter() * 10 && stats->Combo != 0
            && stats->Combo % 10 == 0) {
            stats->MultiplierEffectTime = curSongTime;
        }
    }
    if (!curNote.hit && !curNote.accounted
        && curNote.time + goodBackend + player.InputCalibration < curSongTime
        && !TheSongTime.SongComplete() && stats->curNoteInt < curChart.notes.size()
        && !player.Bot) {
        Encore::EncoreLog(
            LOG_INFO,
            TextFormat(
                "Missed note at %f, note %01i", curSongTime, player.stats->curNoteInt
            )
        );
        stats->MissNote();
        curNote.miss = true;
        curNote.accounted = true;
        stats->Miss = true;
        player.stats->MultiplierEffectTime = curSongTime;
    } else if (player.Bot) {
        if (!curNote.hit && !curNote.accounted
            && curNote.time + player.InputCalibration < curSongTime
            && stats->curNoteInt < curChart.notes.size() && !TheSongTime.SongComplete()) {
            curNote.cHitNote(curSongTime, 0);
            ThePlayerManager.BandStats->AddClassicNotePoint(
                curNote.perfect, stats->noODmultiplier(), curNote.chordSize
            );

            stats->HitPlasticNote(curNote);
        }
    }

    curChart.solos.UpdateEventViaNote(curNote, stats->curSolo);
    curChart.sections.UpdateEventViaNote(curNote, stats->curSection);
    curChart.fills.UpdateEventViaNote(curNote, stats->curFill);
    curChart.overdrive.UpdateEventViaNote(curNote, stats->curODPhrase);
    if (curChart.overdrive.Perfect(stats->curODPhrase)) {
        if (!curChart.overdrive.events[stats->curODPhrase].added)
            player.stats->overdriveHitTime = curSongTime;
        player.stats->overdriveFill +=
            curChart.overdrive.AddOverdrive(stats->curODPhrase);

        if (player.stats->overdriveFill > 1.0f)
            player.stats->overdriveFill = 1.0f;
    }
    /*if (curNote.hit && curChart.overdrive.Perfect(stats->curODPhrase)) {
        if (!curChart.overdrive.events[stats->curODPhrase].added)
            player.stats->overdriveHitTime = curSongTime;
        player.stats->overdriveFill +=
            curChart.overdrive.AddOverdrive(stats->curODPhrase);

        if (player.stats->overdriveFill > 1.0f)
            player.stats->overdriveFill = 1.0f;
    }*/
}
void gameplayRenderer::CalculateSustainScore(PlayerGameplayStats *&stats) {
    double PointsPerTick = double(SUSTAIN_POINTS_PER_BEAT) / 480.0;
    stats->SustainScore += (CurrentTick - stats->LastTick) * PointsPerTick;
    ThePlayerManager.BandStats->SustainScore +=
        (CurrentTick - stats->LastTick) * PointsPerTick;
    stats->Score += (CurrentTick - stats->LastTick) * PointsPerTick;
    ThePlayerManager.BandStats->Score += (CurrentTick - stats->LastTick) * PointsPerTick;
}

float HealthToBrutalPosition(float health, float highwayLength) {
    return Remap(Clamp(health, 0.1f, 0.9f), 0.0f, 1.0f, 0, highwayLength * 0.7);
}

void gameplayRenderer::RenderClassicNotes(
    Player &player, Chart &curChart, double curSongTime, float length
) {
    StartRenderTexture();
    // glDisable(GL_CULL_FACE);
    PlayerGameplayStats *&stats = player.stats;

    double HighwayEnd = length + (smasherPos * 4);
    if (player.BrutalMode) {
        Vector3 BeatlinePos =
            Vector3 { 0, 0, HealthToBrutalPosition(stats->Health, HighwayEnd) };
        DrawModelEx(gprAssets.beatline, BeatlinePos, { 0 }, 0, { 1, 1, 4 }, WHITE);
    }

    for (int n = 0; n < curChart.notes.size(); ++n) {
        // if (curNote.time < TheSongTime.GetFakeStartTime()) {
        //     player.stats->curNoteInt++;
        //     continue;
        // }
        Note &curNote = curChart.notes.at(n);
        if (curNote.time > TheSongTime.GetSongLength())
            continue;

        if (TheSongList.curSong->BRE.IsNoteInCoda(curNote)) {
            stats->curNoteInt++;
            continue;
        }
        if (curNote.time + curNote.len < curSongTime - 2)
            continue;

        double NoteStartPositionWorld =
            GetNotePos(curNote.time, curSongTime, player.NoteSpeed, HighwayEnd);
        double NoteEndPositionWorld = GetNotePos(
            curNote.time + curNote.len, curSongTime, player.NoteSpeed, HighwayEnd
        );

        if (NoteStartPositionWorld > HighwayEnd) {
            break;
        }

        // CheckPlasticNotes(player, curChart, curSongTime, curNote);
        // this is SPECIFICALLY just in case the fucking frontend fails.
        // or something stupid prevents the count from incrementing
        // if (stats->curNoteInt <= n && curNote.miss)
        //    stats->curNoteInt = n + 1;

        if (NoteEndPositionWorld > HighwayEnd)
            NoteEndPositionWorld = HighwayEnd;

        bool SkipShit = false;
        if (NoteEndPositionWorld < -1)
            SkipShit = true;

        bool BrutalSkip = false;
        if (NoteStartPositionWorld < HealthToBrutalPosition(stats->Health, HighwayEnd)
            && player.BrutalMode) {
            BrutalSkip = true;
        }

        if (!curNote.miss && !curChart.overdrive.events.at(stats->curODPhrase).missed
            && (curNote.time >= curChart.overdrive.events.at(stats->curODPhrase).StartSec
                && curNote.time < curChart.overdrive.events.at(stats->curODPhrase).EndSec
            )) {
            curNote.renderAsOD = true;
        }
        for (ClassicLane cLane : curNote.pLanes) {
            int lane = cLane.lane;
            int noteLane = player.LeftyFlip ? 4 - lane : lane;

            Color NoteColor = GRYBO[lane];
            // TheGameMenu.hehe ? TRANS[lane] : GRYBO[lane];
            if (curNote.pOpen)
                NoteColor = PURPLE;

            float notePosX = 2.0f - (1.0f * noteLane);

            // float NoteScroll = smasherPos + (length * (float)relTime);

            // note: WE'RE GETTING CLOSE CHAT. HOLY SHIT CHAT WE'RE GETTING THERE
            // HOLY FUCK CHAT IT MIGHT HAPPEN
            // WE MIGHT DO IT
            // ITS GONNA HAPPEN I FEEL
            // JUST GIVE IT SOME TIME
            // WE MIGHT GET IT
            // this is copium i think

            // todo: REMOVE SUSTAIN CHECK FROM RENDERER
            // todo: PLEASE REMOVE SUSTAIN CHECK FROM RENDERER
            // todo: PLEASE REMOVE LOGIC FROM RENDERER PLEASE :sob:
            if (curNote.len > 0 && !SkipShit) {
                // this is stupid sustain bullshit because you know
                // cant separate the renderer and logic i guess /shrug
                //

                //         __            __
                //        /  \          /  \
                //        |  |          |  |
                //        \__/          \__/
                //
                //    /\                      /\
                //    \ \                    / /
                //     \ \                  / /
                //      \ \________________/ /
                //       \__________________/
                //

                if (curNote.held) {
                    NoteStartPositionWorld = smasherPos;
                }

                if (!SkipShit) {
                    if (BrutalSkip) {
                        NoteStartPositionWorld =
                            HealthToBrutalPosition(stats->Health, HighwayEnd);
                    }
                    nDrawSustain(
                        curNote,
                        NoteColor,
                        notePosX,
                        length,
                        NoteStartPositionWorld,
                        NoteEndPositionWorld
                    );
                }
            }
            if (!SkipShit && !BrutalSkip) {
                nDrawPlasticNote(
                    curNote,
                    player.AccentColor,
                    NoteColor,
                    notePosX,
                    NoteStartPositionWorld
                );
            }
            nDrawFiveLaneHitEffects(player, curNote, curSongTime, notePosX, lane);
        }
    }
    EndMode3D();

    DrawRenderTexture();
}

void gameplayRenderer::DrawHitwindow(Player &player, float length) {
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);

    float lineDistance = player.Difficulty == 3 ? 1.5f : 1.0f;
    float Diff = HighwaySpeedDifficultyMultiplier(player.Difficulty);
    double hitwindowFront = ((goodFrontend - player.InputCalibration))
        * (player.NoteSpeed * Diff) * (11.5f / length);
    double hitwindowBack = ((-goodBackend - player.InputCalibration))
        * (player.NoteSpeed * Diff) * (11.5f / length);

    float Front = (float)(smasherPos + (length * hitwindowFront));
    float Back = (float)(smasherPos + (length * hitwindowBack));
    Color GoodColor = Color { 32, 32, 32, 255 };
    Color PerfectColor = Color { 64, 64, 64, 128 };
    float GoodY = 0;
    float PerfectY = 0.001f;
    if (false) {
        GoodColor = { 0 };
        PerfectColor = { 0 };
    }
    float WidthOfHitThing = 0.25f;
    float SideOfHitTHing = 0 - lineDistance - (1.1f + WidthOfHitThing);
    float OtherSideOfHitThing = 0 - lineDistance - 1.1f;
    DrawTriangle3D(
        { SideOfHitTHing, GoodY, Back },
        { SideOfHitTHing, GoodY, Front },
        { OtherSideOfHitThing, GoodY, Back },
        GoodColor
    );

    DrawTriangle3D(
        { OtherSideOfHitThing, GoodY, Front },
        { OtherSideOfHitThing, GoodY, Back },
        { SideOfHitTHing, GoodY, Front },
        GoodColor
    );

    double perfectFront = ((perfectFrontend - player.InputCalibration))
        * (player.NoteSpeed * Diff) * (11.5f / length);
    double perfectBack = ((-perfectBackend - player.InputCalibration))
        * (player.NoteSpeed * Diff) * (11.5f / length);

    float pFront = (float)(smasherPos + (length * perfectFront));
    float pBack = (float)(smasherPos + (length * perfectBack));

    DrawTriangle3D(
        { SideOfHitTHing, PerfectY, pBack },
        { SideOfHitTHing, PerfectY, pFront },
        { OtherSideOfHitThing, PerfectY, pBack },
        PerfectColor
    );

    DrawTriangle3D(
        { OtherSideOfHitThing, PerfectY, pFront },
        { OtherSideOfHitThing, PerfectY, pBack },
        { SideOfHitTHing, PerfectY, pFront },
        PerfectColor
    );

    float HitwindowCenter = smasherPos
        + (((-player.InputCalibration) * (player.NoteSpeed * Diff) * (11.5f / length))
           * length);
    DrawCylinderEx(
        { OtherSideOfHitThing, 0.002f, HitwindowCenter },
        { SideOfHitTHing, 0.002f, HitwindowCenter },
        0.01f,
        0.01f,
        3,
        WHITE
    );
    float TotalOffset = 0;
    int TotalNotes = 0;
    for (auto it = player.stats->HitwindowNoteHitOffset.begin();
         it != player.stats->HitwindowNoteHitOffset.end();
         ++it) {
        float HitCenter = smasherPos
            + (((it->first - player.InputCalibration) * (player.NoteSpeed * Diff)
                * (11.5f / length))
               * length);

        Color BarColor = (it->second == ALTERNATIVE) ? Color { 255, 0, 255, 32 }
                                                     : Color { 0, 255, 0, 32 };
        DrawCylinderEx(
            { OtherSideOfHitThing, 0.002f, HitCenter },
            { SideOfHitTHing, 0.002f, HitCenter },
            0.015f,
            0.015f,
            3,
            BarColor
        );
        TotalOffset += it->first;
        TotalNotes++;
    }
    float AvgOffset = TotalOffset / TotalNotes;
    float HitCenter = smasherPos
        + (((AvgOffset - player.InputCalibration) * (player.NoteSpeed * Diff)
            * (11.5f / length))
           * length);
    DrawCube(
        { SideOfHitTHing, 0.006f, HitCenter }, 0.2f, 0.002f, 0.2f, { 0, 255, 255, 196 }
    );
    /*
    DrawCylinderEx(
        { SideOfHitTHing, 0.002f, HitCenter },
        { SideOfHitTHing, 0.002f, HitCenter },
        0.1f,
        0.1f,
        4,
        BLUE
    );*/
}

void gameplayRenderer::RenderHud(Player &player, float length) {
    StartRenderTexture();
    BeginBlendModeSeparate();
    DrawHitwindow(player, length);
    DrawModelEx(
        gprAssets.odFrame, Vector3 { 0, 0.0f, 0.8f }, { 0 }, 0, { 1.0, 1.0, 1.4 }, WHITE
    );
    DrawModelEx(
        gprAssets.odBar, Vector3 { 0, 0.0f, 0.8f }, { 0 }, 0, { 1.0, 1.0, 1.4 }, WHITE
    );

    float FillPct = player.stats->comboFillCalc();
    Vector4 MultFillColor { 0.8, 0.8, 0.8, 1 };
    Vector4 GoldMultiplier { OverdriveColor.r / 255.0f,
                             OverdriveColor.g / 255.0f,
                             OverdriveColor.b / 255.0f,
                             OverdriveColor.a / 255.0f };
    Vector4 BlueMultiplier;
    if (player.BrutalMode)
        BlueMultiplier = { 1, 0, 0, 1 };
    else
        BlueMultiplier = { 0.2, 0.6, 1, 1 };

    if (player.stats->IsBassOrVox()) {
        if (player.stats->noODmultiplier() >= 6) {
            MultFillColor = player.stats->Overdrive ? GoldMultiplier : BlueMultiplier;
        }
    } else {
        if (player.stats->noODmultiplier() >= 4) {
            MultFillColor = player.stats->Overdrive ? GoldMultiplier : BlueMultiplier;
        }
    }

    SetShaderValue(
        gprAssets.MultiplierFill,
        gprAssets.FillPercentageLoc,
        &FillPct,
        SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.MultiplierFill,
        gprAssets.MultiplierColorLoc,
        &MultFillColor,
        SHADER_UNIFORM_VEC4
    );
    SetShaderValueTexture(
        gprAssets.MultiplierFill, gprAssets.MultTextureLoc, gprAssets.MultFillBase
    );

    gprAssets.MultInnerDot.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
        ColorBrightness(player.AccentColor, -0.5);
    gprAssets.MultInnerFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
        ColorBrightness(player.AccentColor, -0.4);

    Vector4 FColor = { 0.5f, 0.4f, 0.1, 1.0f };
    float ForFCTime = GetTime();
    int FCING = player.stats->FC ? 1 : 0;
    SetTextureWrap(gprAssets.MultFCTex1, TEXTURE_WRAP_REPEAT);
    SetTextureWrap(gprAssets.MultFCTex2, TEXTURE_WRAP_REPEAT);
    SetTextureWrap(gprAssets.MultFCTex3, TEXTURE_WRAP_REPEAT);
    Color basicColor = player.Bot ? ColorBrightness(SKYBLUE, 0.2)
                                  : ColorBrightness(player.AccentColor, -0.4);
    Vector4 basicColorVec = { basicColor.r / 255.0f,
                              basicColor.g / 255.0f,
                              basicColor.b / 255.0f,
                              basicColor.a / 255.0f };
    SetShaderValue(
        gprAssets.FullComboIndicator, gprAssets.FCIndLoc, &FCING, SHADER_UNIFORM_INT
    );
    SetShaderValue(
        gprAssets.FullComboIndicator, gprAssets.TimeLoc, &ForFCTime, SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.FullComboIndicator,
        gprAssets.BasicColorLoc,
        &basicColorVec,
        SHADER_UNIFORM_VEC4
    );
    SetShaderValue(
        gprAssets.FullComboIndicator, gprAssets.FCColorLoc, &FColor, SHADER_UNIFORM_VEC4
    );
    SetShaderValueTexture(
        gprAssets.FullComboIndicator, gprAssets.BottomTextureLoc, gprAssets.MultFCTex3
    );
    SetShaderValueTexture(
        gprAssets.FullComboIndicator, gprAssets.MiddleTextureLoc, gprAssets.MultFCTex1
    );
    SetShaderValueTexture(
        gprAssets.FullComboIndicator, gprAssets.TopTextureLoc, gprAssets.MultFCTex2
    );

    // DrawModel(gprAssets.MultInnerDot, Vector3 { 0, 0.0f, 1.1f }, 1.1, WHITE);
    DrawModel(gprAssets.MultFill, Vector3 { 0, 0.0f, 1.1f }, 1.0, WHITE);
    DrawModel(gprAssets.MultOuterFrame, Vector3 { 0, 0.0f, 1.1f }, 1.0, WHITE);
    DrawModel(
        gprAssets.MultInnerFrame,
        Vector3 { 0, 0.0f, 1.1f },
        1.0,
        ColorBrightness(player.AccentColor, -0.4)
    );
    DrawModelEx(
        gprAssets.multNumber,
        Vector3 { 0, 0.0f, 1.075f },
        { 0 },
        0,
        { 1.0, 1.15, 1.15 },
        WHITE
    );
    DrawRenderTexture();
}

void gameplayRenderer::RenderGameplay(Player &player, double curSongTime, Song song) {
    if (GameplayRenderTexture.texture.width != GetScreenWidth()
        || GameplayRenderTexture.texture.height != GetScreenHeight()
        || IsWindowResized()) {
        GameplayRenderTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
        // hud_tex.texture.width = width;
        // hud_tex.texture.height = height;
        GenTextureMipmaps(&GameplayRenderTexture.texture);
        SetTextureFilter(GameplayRenderTexture.texture, TEXTURE_FILTER_BILINEAR);
    }
    PlayerGameplayStats *&stats = player.stats;

    Chart &curChart =
        player.stats
            ->CurPlayingChart; // song.parts[player.Instrument]->charts[player.Difficulty];
    float highwayLength = 17.25 * player.HighwayLength;
    player.stats->Difficulty = player.Difficulty;
    player.stats->Instrument = player.Instrument;
    // float multFill = (!player.stats->Overdrive ? (float)(player.stats->multiplier() -
    // 1) : (!player.stats->Multiplayer ? ((float)(player.stats->multiplier() / 2) - 1)
    // / (float)player.stats->maxMultForMeter() : (float)(player.stats->multiplier() -
    // 1)));

    SetShaderValue(
        gprAssets.multNumberShader,
        gprAssets.uvOffsetXLoc,
        &stats->uvOffsetX,
        SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.multNumberShader,
        gprAssets.uvOffsetYLoc,
        &stats->uvOffsetY,
        SHADER_UNIFORM_FLOAT
    );

    float multFill =
        (!player.stats->Overdrive ? (float)(player.stats->multiplier() - 1)
                                  : ((float)(player.stats->multiplier() / 2) - 1))
        / (float)player.stats->maxMultForMeter();

    // float multFill = 0.0;
    float DiffMultiplier =
        Remap(player.Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    // float relTime = (curSongTime) / (highwayLength/4); // 1.75;
    // float highwaySpeedTime = (relTime - smasherPos);
    // float highwaySpeedTime = GetNoteOnScreenTime(0, curSongTime, player.NoteSpeed,
    // player.Difficulty, -player.HighwayLength);
    float highwaySpeedTime =
        (curSongTime * player.NoteSpeed) / (2.222f * player.HighwayLength);
    int PlayerComboMax = (player.Instrument == PartBass || player.Instrument == PartVocals
                          || player.Instrument == PlasticBass)
        ? 50
        : 30;
    Color highwayColor = ColorContrast(
        player.AccentColor,
        Clamp(Remap(player.stats->Combo, 0, PlayerComboMax, -0.6f, 0.0f), -0.6, 0.0f)
    );

    Vector4 ColorForHighway = Vector4 { highwayColor.r / 255.0f,
                                        highwayColor.g / 255.0f,
                                        highwayColor.b / 255.0f,
                                        highwayColor.a / 255.0f };
    stats->MultiplierUVCalculation();
    // NoteMultiplierEffect(curSongTime, player);
    SetShaderValue(
        gprAssets.Highway,
        gprAssets.HighwayTimeShaderLoc,
        &highwaySpeedTime,
        SHADER_UNIFORM_FLOAT
    );

    // SetShaderValue(
    //     gprAssets.Highway,
    //     gprAssets.HighwayColorShaderLoc,
    //     &ColorForHighway,
    //     SHADER_UNIFORM_VEC4
    //);
    SetShaderValue(
        gprAssets.odMultShader, gprAssets.multLoc, &multFill, SHADER_UNIFORM_FLOAT
    );

    SetShaderValue(
        gprAssets.multNumberShader,
        gprAssets.uvOffsetXLoc,
        &player.stats->uvOffsetX,
        SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.multNumberShader,
        gprAssets.uvOffsetYLoc,
        &player.stats->uvOffsetY,
        SHADER_UNIFORM_FLOAT
    );
    float comboFill = player.stats->comboFillCalc();
    int isBassOrVocal = 0;
    if (player.Instrument == PartBass || player.Instrument == PartVocals
        || player.Instrument == PlasticBass) {
        isBassOrVocal = 1;
    }
    SetShaderValue(
        gprAssets.odMultShader,
        gprAssets.isBassOrVocalLoc,
        &isBassOrVocal,
        SHADER_UNIFORM_INT
    );
    SetShaderValue(
        gprAssets.odMultShader, gprAssets.comboCounterLoc, &comboFill, SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.odMultShader,
        gprAssets.odLoc,
        &player.stats->overdriveFill,
        SHADER_UNIFORM_FLOAT
    );

    float HighwayFadeStart = highwayLength + (smasherPos * 2);
    float HighwayEnd = highwayLength + (smasherPos * 4);
    SetShaderValue(
        gprAssets.Highway,
        gprAssets.HighwayScrollFadeStartLoc,
        &HighwayFadeStart,
        SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.Highway,
        gprAssets.HighwayScrollFadeEndLoc,
        &HighwayEnd,
        SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.HighwayFade,
        gprAssets.HighwayFadeStartLoc,
        &HighwayFadeStart,
        SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.HighwayFade,
        gprAssets.HighwayFadeEndLoc,
        &HighwayEnd,
        SHADER_UNIFORM_FLOAT
    );

    gprAssets.noteTopModel.materials->shader = gprAssets.HighwayFade;
    gprAssets.noteBottomModel.materials->shader = gprAssets.HighwayFade;
    gprAssets.liftModel.materials->shader = gprAssets.HighwayFade;
    gprAssets.liftModelOD.materials->shader = gprAssets.HighwayFade;
    gprAssets.noteTopModelHP.materials->shader = gprAssets.HighwayFade;
    gprAssets.noteBottomModelHP.materials->shader = gprAssets.HighwayFade;
    gprAssets.noteTopModelOD.materials->shader = gprAssets.HighwayFade;
    gprAssets.noteBottomModelOD.materials->shader = gprAssets.HighwayFade;
    gprAssets.CymbalInner.materials->shader = gprAssets.HighwayFade;
    gprAssets.CymbalOuter.materials->shader = gprAssets.HighwayFade;
    gprAssets.CymbalBottom.materials->shader = gprAssets.HighwayFade;
    gprAssets.smasherBoard.materials->shader = gprAssets.HighwayFade;
    gprAssets.smasherBoardEMH.materials->shader = gprAssets.HighwayFade;
    gprAssets.smasherPressed.materials->shader = gprAssets.HighwayFade;
    InnerKickSmasher.materials->shader = gprAssets.HighwayFade;
    OuterKickSmasher.materials->shader = gprAssets.HighwayFade;
    InnerTomSmasher.materials->shader = gprAssets.HighwayFade;
    OuterTomSmasher.materials->shader = gprAssets.HighwayFade;
    // gprAssets.smasherReg.materials->shader = gprAssets.HighwayFade;
    gprAssets.smasherInner.materials->shader = gprAssets.HighwayFade;
    gprAssets.smasherOuter.materials->shader = gprAssets.HighwayFade;
    gprAssets.beatline.materials->shader = gprAssets.HighwayFade;
    gprAssets.DarkerHighwayThing.materials->shader = gprAssets.HighwayFade;
    gprAssets.odFrame.materials->shader = gprAssets.HighwayFade;

    gprAssets.expertHighwaySides.materials->shader = gprAssets.HighwayFade;

    gprAssets.odHighwayX.materials[0].maps[MATERIAL_MAP_ALBEDO].color = OverdriveColor;
    gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;
    gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;
    gprAssets.smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
        ColorBrightness(highwayColor, -0.35);
    gprAssets.smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
        ColorBrightness(highwayColor, -0.35);

    gprAssets.MultOuterFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
        MultiplierEffect(curSongTime, player);
    gprAssets.expertHighway.meshes[0].colors = (unsigned char *)ColorToInt(highwayColor);
    gprAssets.emhHighway.meshes[0].colors = (unsigned char *)ColorToInt(highwayColor);
    gprAssets.smasherBoard.meshes[0].colors =
        (unsigned char *)ColorToInt(ColorBrightness(highwayColor, -0.5));
    gprAssets.smasherBoardEMH.meshes[0].colors =
        (unsigned char *)ColorToInt(highwayColor);

    gprAssets.emhHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
        player.AccentColor;

    if (player.Bot)
        player.stats->FC = false;

    // worlds most useful code

    // if (player.Bot) player.Bot = true;
    // else player.Bot = false;

    // RaiseHighway();
    if (!songPlaying) {
        if (Restart) {
            TheAudioManager.restartStreams();
            Restart = false;
        } else
            TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[0].handle);
        songPlaying = true;
        double songEnd =
            floor(TheAudioManager.GetMusicTimeLength()) >= (song.end <= 0 ? 0 : song.end)
            ? floor(TheAudioManager.GetMusicTimeLength())
            : song.end - 0.01;
        highwayInEndAnim = false;
        TheSongTime.Start(songEnd);
    }

    // BeginShaderMode(gprAssets.HighwayFade);

    if (player.Instrument == PlasticDrums) {
        RenderPDrumsHighway(player, song, curSongTime);
    } else if (player.Difficulty == 3
               || (player.ClassicMode && player.Instrument != PlasticDrums)) {
        RenderExpertHighway(player, song, curSongTime);
    } else {
        RenderEmhHighway(player, song, curSongTime);
    }
    if (player.ClassicMode) {
        if (player.Instrument == PlasticDrums) {
            RenderPDrumsNotes(player, curChart, curSongTime, highwayLength);
        } else {
            RenderClassicNotes(player, curChart, curSongTime, highwayLength);
        }
    } else {
        RenderPadNotes(player, curChart, curSongTime, highwayLength);
    }

    if (player.stats->LastPerfectTime != -2.0) {
        DrawPerfectText(player.stats->LastPerfectTime, curSongTime, player);
    };
    //
    // if (player.stats->Combo >= 25) {
    //     DrawPerfectText(player.stats->Combo, curSongTime, player);
    // };

    if (!song.BRE.IsCodaActive(curSongTime)) {
        RenderHud(player, highwayLength);
    }

    float PlayerCombinedHealth = 0;

    for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
        PlayerCombinedHealth += ThePlayerManager.GetActivePlayer(i).stats->Health;
    }

    ThePlayerManager.BandStats->Health =
        PlayerCombinedHealth / (ThePlayerManager.PlayersActive);

    float TempHealthBarHeight =
        Remap(ThePlayerManager.BandStats->Health, 0, 100.0f, 0, GetScreenHeight());

    DrawRectangle(
        gprU.hpct(0.0f), 0, 10, gprU.hinpct(ThePlayerManager.BandStats->Health), GREEN
    );

    DrawTextEx(
        gprAssets.rubik,
        TextFormat("%4.2f", ThePlayerManager.BandStats->Health),
        { 0, 0 },
        32,
        0,
        WHITE
    );
}

void gameplayRenderer::DrawHighwayMesh(
    float LengthMultiplier, bool Overdrive, float ActiveTime, float SongTime, bool EMH
) {
    Vector3 HighwayPos { 0, 0, -0.2f };
    Vector3 HighwayScale { 1, 1, 1.5f * LengthMultiplier };
    Model Highway;
    Model OverdriveOverlay;
    Model HighwaySides;
    Model SmasherBoard;
    if (EMH) {
        Highway = gprAssets.emhHighway;
        OverdriveOverlay = gprAssets.odHighwayEMH;
        HighwaySides = gprAssets.emhHighwaySides;
        SmasherBoard = gprAssets.smasherBoardEMH;
    } else {
        Highway = gprAssets.expertHighway;
        OverdriveOverlay = gprAssets.odHighwayX;
        HighwaySides = gprAssets.expertHighwaySides;
        SmasherBoard = gprAssets.smasherBoard;
    }
    Highway.materials[0].maps->texture.height /= 2.0f;
    OverdriveOverlay.materials[0].maps->texture.height /= 2.0f;
    DrawModelEx(HighwaySides, HighwayPos, { 0 }, 0, HighwayScale, WHITE);
    DrawModelEx(Highway, HighwayPos, { 0 }, 0, HighwayScale, WHITE);
    unsigned char OverdriveAlpha = 255;
    double OverdriveAnimDuration = 0.25f;

    DrawModel(SmasherBoard, Vector3 { 0, 0.0015f, -0.25f }, 1.0f, WHITE);
    if (SongTime <= ActiveTime + OverdriveAnimDuration) {
        double TimeSinceOverdriveActivate = SongTime - ActiveTime;
        OverdriveAlpha = Remap(
            getEasingFunction(EaseOutQuint)(
                TimeSinceOverdriveActivate / OverdriveAnimDuration
            ),
            0,
            1.0,
            0,
            255
        );
    } else
        OverdriveAlpha = 255;

    if (SongTime <= ActiveTime + OverdriveAnimDuration
        && SongTime > OverdriveAnimDuration) {
        double TimeSinceOverdriveActivate = SongTime - ActiveTime;
        OverdriveAlpha = Remap(
            getEasingFunction(EaseOutQuint)(
                TimeSinceOverdriveActivate / OverdriveAnimDuration
            ),
            0,
            1.0,
            255,
            0
        );
    } else if (!Overdrive)
        OverdriveAlpha = 0;

    if (Overdrive || SongTime <= ActiveTime + OverdriveAnimDuration) {
        DrawModelEx(
            OverdriveOverlay,
            Vector3 { 0, 0.005f, -0.2 },
            { 0 },
            0,
            HighwayScale,
            Color { 255, 255, 255, OverdriveAlpha }
        );
    }
};

void gameplayRenderer::StartRenderTexture() {
    BeginTextureMode(GameplayRenderTexture);
    ClearBackground({ 0, 0, 0, 0 });
    BeginMode3D(cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel]);
}
void gameplayRenderer::DrawSmashers(Player &player) {
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);

    for (int i = 0; i < (player.Difficulty == 3 || player.ClassicMode ? 5 : 4); i++) {
        Color NoteColor; // = TheGameMenu.hehe && player.Difficulty == 3 ? i == 0 || i ==
        // 4 ? SKYBLUE : i == 1 || i == 3 ? PINK : WHITE : accentColor;
        int noteColor = player.LeftyFlip
            ? (player.Difficulty || player.ClassicMode == 3 ? 4 : 3) - i
            : i;
        if (player.ClassicMode) {
            NoteColor = GRYBO[i];
            // TheGameMenu.hehe ? TRANS[i] : GRYBO[i];
        } else {
            NoteColor = player.AccentColor;
            // TheGameMenu.hehe ? TRANS[i] : player.AccentColor;
        }

        gprAssets.smasherOuter.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
        float RandomRotationToSelect = GetRandomValue(-2, 2) / 10.0f;
        // constexpr float thing[4] {0.15, 0.1, -0.1, -0.15};
        Vector3 RotationAxis { 1, 0, RandomRotationToSelect };
        // if (player.stats->HeldFrets[noteColor]
        //     || player.stats->HeldFretsAlt[noteColor]) {
        gprAssets.smasherInner.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        if (player.stats->HeldFrets[noteColor] || player.stats->HeldFretsAlt[noteColor]) {
            gprAssets.smasherInner.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
                gprAssets.smasherTopPressedTex;
        } else {
            gprAssets.smasherInner.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
                gprAssets.smasherTopUnpressedTex;
        }
        DrawModelEx(
            gprAssets.smasherInner,
            Vector3 { diffDistance(player.Difficulty, player.ClassicMode) - (i),
                      player.stats->fiveLaneSmasherHeights[i],
                      smasherPos },
            { 0 },
            0,
            { 1.0f, 1.0f, 1.0f },
            WHITE
        );
        DrawModelEx(
            gprAssets.smasherOuter,
            Vector3 { diffDistance(player.Difficulty, player.ClassicMode) - (i),
                      0,
                      smasherPos },
            { 0, 0, 0 },
            0,
            { 1.0f, 1.0f, 1.0f },
            ColorBrightness(ColorContrast(player.AccentColor, -0.5), 0.25)
        );
    }
}
void gameplayRenderer::RenderExpertHighway(Player &player, Song song, double curSongTime) {
    StartRenderTexture();
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
    PlayerGameplayStats *&stats = player.stats;

    float highwayLength =
        (defaultHighwayLength * 1.5f) * player.HighwayLength; //* player.HighwayLength;
    float highwayPosShit = ((20) * (1 - player.HighwayLength));

    DrawHighwayMesh(
        player.HighwayLength,
        player.stats->Overdrive,
        player.stats->overdriveActiveTime,
        curSongTime,
        false
    );
    int UseIn = 1;
    int DontIn = 0;
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT
    );

    DrawRenderTexture();

    // StartRenderTexture();
    // BeginBlendModeSeparate();

    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &UseIn, SHADER_UNIFORM_INT
    );
    // BeginShaderMode(gprAssets.HighwayFade);

    // DrawHitwindow(player, highwayLength);
    // DrawRenderTexture();

    StartRenderTexture();
    BeginShaderMode(gprAssets.HighwayFade);
    if (!song.beatLines.empty()) {
        DrawBeatlines(player, song, highwayLength, curSongTime);
    }
    DrawRenderTexture();
    /*
        StartRenderTexture();
        BeginShaderMode(gprAssets.HighwayFade);
        if
       (!song.parts[player.Instrument]->charts[player.Difficulty].overdrive.events.empty(
            )) {
            DrawOverdrive(
                player,
                song.parts[player.Instrument]->charts[player.Difficulty],
                highwayLength,
                curSongTime
            );
        }
        float darkYPos = 0.015f;
        // BeginBlendMode(BLEND_ALPHA);
        DrawRenderTexture();
    */
    StartRenderTexture();
    BeginShaderMode(gprAssets.HighwayFade);
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT
    );
    if (!song.parts[player.Instrument]->charts[player.Difficulty].solos.events.empty()) {
        DrawSolo(
            player,
            song.parts[player.Instrument]->charts[player.Difficulty],
            highwayLength,
            curSongTime
        );
    }
    DrawRenderTexture();

    StartRenderTexture();
    BeginShaderMode(gprAssets.HighwayFade);
    if (song.BRE.exists) {
        DrawCoda(highwayLength, curSongTime, player);
    }
    DrawRenderTexture();

    StartRenderTexture();
    BeginShaderMode(gprAssets.HighwayFade);
    nDrawFiveLaneUnderlay(highwayLength, !player.ClassicMode, player);
    DrawRenderTexture();

    StartRenderTexture();
    DrawSmashers(player);
    DrawRenderTexture();
}

void gameplayRenderer::RenderEmhHighway(Player &player, Song song, double curSongTime) {
    StartRenderTexture();
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
    PlayerGameplayStats *&stats = player.stats;

    float highwayLength =
        (defaultHighwayLength * 1.5f) * player.HighwayLength; //* player.HighwayLength;
    float highwayPosShit = ((20) * (1 - player.HighwayLength));

    DrawHighwayMesh(
        player.HighwayLength,
        player.stats->Overdrive,
        player.stats->overdriveActiveTime,
        curSongTime,
        true
    );
    int UseIn = 1;
    int DontIn = 0;
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT
    );

    DrawRenderTexture();

    // StartRenderTexture();
    // BeginBlendModeSeparate();

    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &UseIn, SHADER_UNIFORM_INT
    );
    // BeginShaderMode(gprAssets.HighwayFade);

    // DrawHitwindow(player, highwayLength);
    // DrawRenderTexture();

    StartRenderTexture();
    BeginShaderMode(gprAssets.HighwayFade);
    if (!song.beatLines.empty()) {
        DrawBeatlines(player, song, highwayLength, curSongTime);
    }
    DrawRenderTexture();

    StartRenderTexture();
    BeginShaderMode(gprAssets.HighwayFade);
    if (!song.parts[player.Instrument]->charts[player.Difficulty].overdrive.events.empty(
        )) {
        DrawOverdrive(
            player,
            song.parts[player.Instrument]->charts[player.Difficulty],
            highwayLength,
            curSongTime
        );
    }
    float darkYPos = 0.015f;
    // BeginBlendMode(BLEND_ALPHA);
    DrawRenderTexture();

    StartRenderTexture();
    BeginShaderMode(gprAssets.HighwayFade);
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT
    );
    if (!song.parts[player.Instrument]->charts[player.Difficulty].solos.events.empty()) {
        DrawSolo(
            player,
            song.parts[player.Instrument]->charts[player.Difficulty],
            highwayLength,
            curSongTime
        );
    }
    DrawRenderTexture();

    StartRenderTexture();
    BeginShaderMode(gprAssets.HighwayFade);
    if (song.BRE.exists) {
        DrawCoda(highwayLength, curSongTime, player);
    }
    DrawRenderTexture();

    StartRenderTexture();
    BeginShaderMode(gprAssets.HighwayFade);
    nDrawFiveLaneUnderlay(highwayLength, !player.ClassicMode, player);
    DrawRenderTexture();

    StartRenderTexture();
    DrawSmashers(player);
    DrawRenderTexture();
}

void gameplayRenderer::DrawBeatlines(
    Player &player, Song &song, float length, double musicTime
) {
    Model beatline = gprAssets.beatline;
    beatline.materials[0].shader = gprAssets.HighwayFade;
    float yPos = 0.0f;
    float AddToFrontPos = 0.0f;
    double HighwayEnd = length + (smasherPos * 4);
    if (song.beatLines.size() > 0) {
        for (int i = 0; i < song.beatLines.size(); i++) {
            Color BeatLineColor = { 255, 255, 255, 255 };
            float NotePos = GetNotePos(
                song.beatLines[i].Time, musicTime, player.NoteSpeed, HighwayEnd
            );
            if (NotePos < 0)
                continue;
            // disabled due to too much visual clutter
            if (i > 0 && false) {
                float minorPos = GetNotePos(
                    ((song.beatLines[i - 1].Time + song.beatLines[i].Time) / 2),
                    musicTime,
                    player.NoteSpeed,
                    HighwayEnd
                );

                if (minorPos > HighwayEnd)
                    break;
                if (NotePos - minorPos > 3) {
                    Color BeatLineColor2 = { 255, 255, 255, 255 };
                    Vector3 SecondaryBeatlinePos =
                        Vector3 { 0, yPos, minorPos - AddToFrontPos };
                    beatline.materials[0].maps[MATERIAL_MAP_DIFFUSE].color =
                        BeatLineColor2;
                    beatline.meshes[0].colors =
                        (unsigned char *)ColorToInt(BeatLineColor2);
                    DrawModelEx(
                        beatline,
                        SecondaryBeatlinePos,
                        { 0 },
                        0,
                        { 1, 1, 0.5 },
                        BeatLineColor2
                    );
                }
            }

            if (NotePos > HighwayEnd)
                break;

            float radius = song.beatLines[i].Major ? 1.5f : 0.95f;

            Color BeatLineColorA = (song.beatLines[i].Major)
                ? Color { 255, 255, 255, 255 }
                : Color { 255, 255, 255, 255 };
            beatline.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BeatLineColorA;
            beatline.meshes[0].colors = (unsigned char *)ColorToInt(BeatLineColorA);
            Vector3 BeatlinePos = Vector3 { 0, yPos, NotePos - AddToFrontPos };
            DrawModelEx(beatline, BeatlinePos, { 0 }, 0, { 1, 1, radius }, BeatLineColorA);
            /*DrawCylinderEx(Vector3{ -diffDistance - 0.5f,yPos,smasherPos + (length *
               (float)relTime) + (radius * 2) }, Vector3{ diffDistance +
               0.5f,yPos,smasherPos + (length * (float)relTime) + (radius * 2) },
                           radius,
                           radius,
                           4,
                           BeatLineColor);*/

            // if (relTime < -1) break;
        }
    }
}

void gameplayRenderer::DrawOverdrive(
    Player &player, Chart &curChart, float length, double musicTime
) {
    float DiffMultiplier =
        Remap(player.Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    float start = curChart.overdrive[player.stats->curODPhrase].StartSec;
    float end = curChart.overdrive[player.stats->curODPhrase].EndSec;
    /* for unisons
        eDrawSides((player.NoteSpeed * DiffMultiplier), musicTime, start, end, length,
       0.1, WHITE);
    */
}

void gameplayRenderer::DrawSolo(
    Player &player, Chart &curChart, float length, double musicTime
) {
    float DiffMultiplier =
        Remap(player.Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    float start = curChart.solos[player.stats->curSolo].StartSec;
    float end = curChart.solos[player.stats->curSolo].EndSec;
    nDrawSoloSides(
        length,
        start,
        end,
        musicTime,
        player.NoteSpeed,
        player.Difficulty,
        player.ClassicMode
    );
    int DoIn = 1;
    int DontIn = 0;
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DoIn, SHADER_UNIFORM_INT
    );
    eDrawSides(
        (player.NoteSpeed * DiffMultiplier),
        musicTime,
        start,
        end,
        length,
        0.09,
        SKYBLUE,
        (player.Difficulty != 3 && !player.ClassicMode)
    );
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT
    );
    if (!curChart.solos.events.empty()
        && musicTime >= curChart.solos[player.stats->curSolo].StartSec - 1
        && musicTime <= curChart.solos[player.stats->curSolo].EndSec + 2.5) {
        int solopctnum = Remap(
            curChart.solos[player.stats->curSolo].NotesHit,
            0,
            curChart.solos[player.stats->curSolo].NoteCount,
            0,
            100
        );
        Color accColor = solopctnum == 100 ? GOLD : WHITE;
        const char *soloPct = TextFormat("%i%%", solopctnum);
        float soloPercentLength =
            MeasureTextEx(gprAssets.rubikBold, soloPct, gprU.hinpct(0.09f), 0).x;

        Vector2 SoloBoxPos = { (GetScreenWidth() / 2) - (soloPercentLength / 2),
                               gprU.hpct(0.2f) };

        // DrawTextEx(gprAssets.rubikBold, soloPct, SoloBoxPos, gprU.hinpct(0.09f), 0,
        // accColor);

        const char *soloHit = TextFormat(
            "%i/%i",
            curChart.solos[player.stats->curSolo].NotesHit,
            curChart.solos[player.stats->curSolo].NoteCount
        );

        // DrawTextEx(gprAssets.josefinSansItalic, soloHit, SoloHitPos,
        // gprU.hinpct(0.04f), 0, accColor);

        float posY = -25;

        float height = -2;
        float pctDist = 1.2;
        float praiseDist = 0;
        float backgroundHeight = height - 1.2;
        float soloScale = 1.15;

        float fontSizee = 160;
        float fontSize = 75;

        gprAssets.SoloBox.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
            player.AccentColor;
        rlPushMatrix();
        rlRotatef(180, 0, 1, 0);
        rlRotatef(90, 1, 0, 0);
        // rlRotatef(90.0f, 0.0f, 0.0f, 1.0f);								//0, 1, 2.4
        float soloWidth =
            MeasureText3D(gprAssets.rubikBold, soloPct, fontSizee, 0, 0).x / 2;
        float hitWidth =
            MeasureText3D(gprAssets.josefinSansItalic, soloHit, fontSize, 0, 0).x / 2;
        DrawModel(
            gprAssets.SoloBox, Vector3 { 0, posY - 1, backgroundHeight }, soloScale, WHITE
        );
        DrawText3D(
            gprAssets.rubikBold,
            soloPct,
            Vector3 { 0 - soloWidth, posY, height - pctDist },
            fontSizee,
            0,
            0,
            1,
            accColor
        );
        if (musicTime <= curChart.solos[player.stats->curSolo].EndSec)
            DrawText3D(
                gprAssets.josefinSansItalic,
                soloHit,
                Vector3 { 0 - hitWidth, posY, height },
                fontSize,
                0,
                0,
                1,
                accColor
            );
        rlPopMatrix();
        if (musicTime >= curChart.solos[player.stats->curSolo].EndSec
            && musicTime <= curChart.solos[player.stats->curSolo].EndSec + 2.5) {
            const char *PraiseText = "";
            if (solopctnum == 100) {
                PraiseText = "Perfect Solo!";
            } else if (solopctnum == 99) {
                PraiseText = "Awesome Choke!";
            } else if (solopctnum > 90) {
                PraiseText = "Awesome solo!";
            } else if (solopctnum > 80) {
                PraiseText = "Great solo!";
            } else if (solopctnum > 75) {
                PraiseText = "Decent solo";
            } else if (solopctnum > 50) {
                PraiseText = "OK solo";
            } else if (solopctnum > 0) {
                PraiseText = "Bad solo";
            }
            rlPushMatrix();
            rlRotatef(180, 0, 1, 0);
            rlRotatef(90, 1, 0, 0);
            // rlRotatef(90.0f, 0.0f, 0.0f, 1.0f);								//0,
            // 1, 2.4
            float praiseWidth =
                MeasureText3D(gprAssets.josefinSansItalic, PraiseText, fontSize, 0, 0).x
                / 2;
            DrawText3D(
                gprAssets.josefinSansItalic,
                PraiseText,
                Vector3 { 0 - praiseWidth, posY, height - praiseDist },
                fontSize,
                0,
                0,
                1,
                accColor
            );
            rlPopMatrix();
            // DrawTextEx(gprAssets.josefinSansItalic, PraiseText, PraisePos,
            // gprU.hinpct(0.05f), 0, accColor);
        }
    }
}

void gameplayRenderer::DrawFill(
    Player &player, Chart &curChart, float length, double musicTime
) {
    float DiffMultiplier =
        Remap(player.Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);

    float start = curChart.fills[player.stats->curFill].StartSec;
    float end = curChart.fills[player.stats->curFill].EndSec;
    eDrawSides(
        (player.NoteSpeed * DiffMultiplier),
        musicTime,
        start,
        end,
        length,
        0.075,
        GREEN,
        (player.Difficulty != 3 && !player.ClassicMode)
    );
}

void gameplayRenderer::DrawCoda(float length, double musicTime, Player &player) {
    float DiffMultiplier =
        Remap(player.Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);

    float start = TheSongList.curSong->BRE.StartSec;
    float end = TheSongList.curSong->BRE.EndSec;
    nDrawCodaLanes(length, start, end, musicTime, player.NoteSpeed, player.Difficulty);
    eDrawSides(
        (player.NoteSpeed * DiffMultiplier),
        musicTime,
        start,
        end,
        length,
        0.075,
        PURPLE,
        (player.Difficulty != 3 && !player.ClassicMode)
    );
}

void gameplayRenderer::eDrawSides(
    float scrollPos,
    double curSongTime,
    double start,
    double end,
    float length,
    double radius,
    Color color,
    bool emh
) {
    float soloStart = (float)((start - curSongTime)) * scrollPos * (11.5f / length);
    float soloEnd = (float)((end - curSongTime)) * scrollPos * (11.5f / length);

    // horrifying.
    // main calc
    bool Beginning =
        (float)(smasherPos + (length * soloStart)) >= (length * 1.5f) + smasherPos;
    bool Ending =
        (float)(smasherPos + (length * soloEnd)) >= (length * 1.5f) + smasherPos;
    float HighwayEnd = (length * 1.5f) + smasherPos;

    // right calc
    float RightSideX = emh ? 2.2f : 2.7f;
    float StartPos = Beginning ? HighwayEnd : (float)(smasherPos + (length * soloStart));
    float EndPos = Ending ? HighwayEnd : (float)(smasherPos + (length * soloEnd));
    Vector3 RightSideStart = { RightSideX, -0.0025, StartPos };
    Vector3 RightSideEnd = { RightSideX, -0.0025, EndPos };

    // left calc
    float LeftSideX = emh ? -2.2f : -2.7f;

    Vector3 LeftSideStart = { LeftSideX, -0.0025, StartPos };
    Vector3 LeftSideEnd = { LeftSideX, -0.0025, EndPos };

    // draw
    // if (soloEnd < -1) break;
    if (soloEnd > -1) {
        DrawCylinderEx(RightSideStart, RightSideEnd, radius, radius, 10, color);
        DrawCylinderEx(LeftSideStart, LeftSideEnd, radius, radius, 10, color);
    }
}

double gameplayRenderer::GetNoteOnScreenTime(
    double noteTime, double songTime, float noteSpeed, int Difficulty, float length
) {
    return ((noteTime - songTime)) * (noteSpeed * MaxHighwaySpeed) * (11.5f / length);
}
void gameplayRenderer::DrawPerfectText(double noteTime, double songTime, Player &player) {
    double PerfectHitAnimDuration = 0.75f;
    if (songTime > PerfectHitAnimDuration + noteTime)
        return;

    Units u = Units::getInstance();
    Vector3 WorldPerfectPos = { 0.4f, 0, 1.275f };
    Camera3D WorldCamera = cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel];
    Vector2 PerfectTextScreenPos = GetWorldToScreen(WorldPerfectPos, WorldCamera);
    double TimeSinceHit = songTime - noteTime;
    unsigned char HitAlpha = Remap(
        getEasingFunction(EaseOutQuad)(TimeSinceHit / PerfectHitAnimDuration),
        0,
        1.0,
        255,
        0
    );
    float ScreenYPos = Remap(
        getEasingFunction(EaseInOutBack)(TimeSinceHit / PerfectHitAnimDuration),
        0,
        1.0,
        0,
        u.winpct(0.02)
    );
    /*
    float HitPosLeft = Remap(
        getEasingFunction(EaseInOutBack)(TimeSinceHit / PerfectHitAnimDuration),
        0,
        1.0,
        3.4,
        3.0
    );*/

    Color InnerBoxColor = { 255, 255, 255, static_cast<unsigned char>(HitAlpha / 2) };
    Color OuterBoxColor = { 255, 255, 255, HitAlpha };
    Color BackgroundColor = { GOLD.r, GOLD.g, GOLD.b, HitAlpha };
    float Width = 1.0f;
    float Height = 0.01f;
    float Offset = u.hinpct(0.0025f);
    float FontSize = u.hinpct(0.025f);
    Vector2 PerfectTextPos = { (PerfectTextScreenPos.x + ScreenYPos) - renderPos,
                               PerfectTextScreenPos.y };
    Vector3 CenterlineWorld = { 0.0f, 0, 1.275f };
    Vector2 CenterlinePos = GetWorldToScreen(CenterlineWorld, WorldCamera);
    CenterlinePos.x -= renderPos;
    float TextWidth = MeasureTextEx(gprAssets.rubikBold, "PERFECT", FontSize, 0).x;
    float TextHeight = MeasureTextEx(gprAssets.rubikBold, "PERFECT", FontSize, 0).y;

    float BoxLeft = (PerfectTextPos.x - TextWidth);
    float BoxWidth = (CenterlinePos.x - BoxLeft);
    float BoxLeftTwo = CenterlinePos.x - BoxWidth;
    DrawRectangleGradientH(
        BoxLeft, PerfectTextPos.y, BoxWidth, TextHeight, { 0 }, BackgroundColor
    );
    // GameMenu::mhDrawText(
    //     gprAssets.rubikBold,
    //     "PERFECT",
    //     { PerfectTextPos.x + Offset, PerfectTextPos.y + Offset },
    //     FontSize,
    //     InnerBoxColor,
    //     gprAssets.sdfShader,
    //     RIGHT
    // );
    GameMenu::mhDrawText(
        gprAssets.rubikBold,
        "PERFECT",
        PerfectTextPos,
        FontSize,
        OuterBoxColor,
        gprAssets.sdfShader,
        RIGHT
    );

    /*
    DrawCube(
        Vector3 { HitPosLeft, -0.1f, smasherPos }, Width, Height, 0.5f, InnerBoxColor
    );
    DrawCube(
        Vector3 { HitPosLeft, -0.11f, smasherPos }, Width, Height, 1.0f, OuterBoxColor
    );*/
}

void gameplayRenderer::DrawCombo(double noteTime, double songTime, Player &player) {
    double PerfectHitAnimDuration = 0.75f;
    if (songTime > PerfectHitAnimDuration + noteTime)
        return;

    Units u = Units::getInstance();
    Vector3 WorldPerfectPos = { 0.4f, 0, 1.275f };
    Camera3D WorldCamera = cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel];
    Vector2 PerfectTextScreenPos = GetWorldToScreen(WorldPerfectPos, WorldCamera);
    double TimeSinceHit = songTime - noteTime;
    unsigned char HitAlpha = Remap(
        getEasingFunction(EaseOutQuad)(TimeSinceHit / PerfectHitAnimDuration),
        0,
        1.0,
        255,
        0
    );
    float ScreenYPos = Remap(
        getEasingFunction(EaseInOutBack)(TimeSinceHit / PerfectHitAnimDuration),
        0,
        1.0,
        0,
        u.winpct(0.02)
    );
    /*
    float HitPosLeft = Remap(
        getEasingFunction(EaseInOutBack)(TimeSinceHit / PerfectHitAnimDuration),
        0,
        1.0,
        3.4,
        3.0
    );*/

    Color InnerBoxColor = { 255, 255, 255, static_cast<unsigned char>(HitAlpha / 2) };
    Color OuterBoxColor = { 255, 255, 255, HitAlpha };
    Color BackgroundColor = { GOLD.r, GOLD.g, GOLD.b, HitAlpha };
    float Width = 1.0f;
    float Height = 0.01f;
    float Offset = u.hinpct(0.0025f);
    float FontSize = u.hinpct(0.025f);
    Vector2 PerfectTextPos = { (PerfectTextScreenPos.x + ScreenYPos) - renderPos,
                               PerfectTextScreenPos.y };
    Vector3 CenterlineWorld = { 0.0f, 0, 1.275f };
    Vector2 CenterlinePos = GetWorldToScreen(CenterlineWorld, WorldCamera);
    CenterlinePos.x -= renderPos;
    float TextWidth = MeasureTextEx(gprAssets.rubikBold, "PERFECT", FontSize, 0).x;
    float TextHeight = MeasureTextEx(gprAssets.rubikBold, "PERFECT", FontSize, 0).y;

    float BoxLeft = (PerfectTextPos.x - TextWidth);
    float BoxWidth = (CenterlinePos.x - BoxLeft);
    float BoxLeftTwo = CenterlinePos.x - BoxWidth;
    DrawRectangleGradientH(
        BoxLeft, PerfectTextPos.y, BoxWidth, TextHeight, { 0 }, BackgroundColor
    );
    // GameMenu::mhDrawText(
    //     gprAssets.rubikBold,
    //     "PERFECT",
    //     { PerfectTextPos.x + Offset, PerfectTextPos.y + Offset },
    //     FontSize,
    //     InnerBoxColor,
    //     gprAssets.sdfShader,
    //     RIGHT
    // );
    GameMenu::mhDrawText(
        gprAssets.rubikBold,
        "PERFECT",
        PerfectTextPos,
        FontSize,
        OuterBoxColor,
        gprAssets.sdfShader,
        RIGHT
    );

    /*
    DrawCube(
        Vector3 { HitPosLeft, -0.1f, smasherPos }, Width, Height, 0.5f, InnerBoxColor
    );
    DrawCube(
        Vector3 { HitPosLeft, -0.11f, smasherPos }, Width, Height, 1.0f, OuterBoxColor
    );*/
}

double gameplayRenderer::HighwaySpeedDifficultyMultiplier(int Difficulty) {
    return Remap(Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
}
gameplayRenderer::gameplayRenderer() {}
gameplayRenderer::~gameplayRenderer() {
    UnloadRenderTexture(GameplayRenderTexture);
}
// classic drums
void gameplayRenderer::RenderPDrumsNotes(
    Player &player, Chart &curChart, double curSongTime, float length
) {
    float DiffMultiplier =
        Remap(player.Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    StartRenderTexture();
    // glDisable(GL_CULL_FACE);
    PlayerGameplayStats *&stats = player.stats;

    for (auto &curNote : curChart.notes) {
        double HighwayEnd = length + (smasherPos * 4);
        double NoteStartPositionWorld =
            GetNotePos(curNote.time, curSongTime, player.NoteSpeed, HighwayEnd);
        double NoteEndPositionWorld = GetNotePos(
            curNote.time + curNote.len, curSongTime, player.NoteSpeed, HighwayEnd
        );
        if (NoteStartPositionWorld >= HighwayEnd)
            break;
        // NotesToRender++;
        if (!curNote.hit && !curNote.accounted
            && curNote.time + goodBackend + player.InputCalibration < curSongTime
            && !TheSongTime.SongComplete() && stats->curNoteInt < curChart.notes.size()
            && !TheSongTime.SongComplete() && !player.Bot) {
            Encore::EncoreLog(
                LOG_INFO,
                TextFormat("Missed note at %f, note %01i", curSongTime, stats->curNoteInt)
            );
            curNote.miss = true;
            FAS = false;
            stats->MissNote();
            stats->Combo = 0;
            curNote.accounted = true;
            stats->curNoteInt++;
        } else if (player.Bot) {
            if (!curNote.hit && !curNote.accounted && curNote.time < curSongTime
                && stats->curNoteInt < curChart.notes.size()
                && !TheSongTime.SongComplete()) {
                curNote.hit = true;
                player.stats->HitDrumsNote(true, !curNote.pDrumTom);
                ThePlayerManager.BandStats->DrumNotePoint(
                    true, player.stats->noODmultiplier(), !curNote.pDrumTom
                );

                stats->LastPerfectTime = curNote.hitTime;
                if (curNote.len > 0)
                    curNote.held = true;
                curNote.accounted = true;
                // stats->Combo++;
                curNote.accounted = true;
                curNote.hitTime = curSongTime;
                stats->curNoteInt++;
                if (player.stats->overdriveFill >= 0.25 && curNote.pDrumAct
                    && !player.stats->Overdrive) {
                    stats->overdriveActiveTime = curSongTime;
                    stats->overdriveActiveFill = stats->overdriveFill;
                    stats->Overdrive = true;
                    stats->overdriveHitAvailable = true;
                    stats->overdriveHitTime = curSongTime;
                    ThePlayerManager.BandStats->PlayersInOverdrive += 1;
                    ThePlayerManager.BandStats->Overdrive = true;
                }
            }
        }

        if (stats->StrumNoFretTime > curSongTime + fretAfterStrumTime && stats->FAS) {
            stats->FAS = false;
        }

        if (player.stats->Overdrive) {
            player.stats->overdriveActiveFill +=
                curChart.overdrive.AddOverdrive(player.stats->curODPhrase);
            if (player.stats->overdriveActiveFill > 1.0f)
                player.stats->overdriveActiveFill = 1.0f;
        } else {
            player.stats->overdriveFill +=
                curChart.overdrive.AddOverdrive(player.stats->curODPhrase);
            if (player.stats->overdriveFill > 1.0f)
                player.stats->overdriveFill = 1.0f;
        }

        curChart.solos.UpdateEventViaNote(curNote, player.stats->curSolo);
        curChart.overdrive.UpdateEventViaNote(curNote, player.stats->curODPhrase);
        curChart.sections.UpdateEventViaNote(curNote, player.stats->curSection);
        curChart.fills.UpdateEventViaNote(curNote, player.stats->curFill);

        if (NoteEndPositionWorld > HighwayEnd)
            NoteEndPositionWorld = HighwayEnd;

        bool SkipShit = false;
        if (NoteEndPositionWorld < -1)
            SkipShit = true;

        std::vector<Color> DRUMS = { ORANGE, RED, YELLOW, BLUE, GREEN };
        Color NoteColor = DRUMS[curNote.lane];

        float notePosX = curNote.lane == KICK
            ? 0
            : (diffDistance(player.Difficulty, true) - (1.25f * (curNote.lane - 1)))
                - 0.125f;
        float notePosY = 0;

        Vector3 NoteScale = { 1.0f, 1.0f, 1.0f };
        Vector3 NotePos = { notePosX, notePosY, float(NoteStartPositionWorld) };
        float Factor = 1.0f;
        if (!curNote.pDrumTom && !curNote.pSnare && !curNote.hit && curNote.lane != KICK
            && player.ProDrums) { // render cymbals
            Color BaseColor = WHITE;
            Color ColorColor = NoteColor;
            Color WhiteColor = ColorBrightness(NoteColor, Factor);
            if (curNote.renderAsOD) {
                BaseColor = GOLD;
                ColorColor = WHITE;
                WhiteColor = GOLD;
            } else if (curNote.miss) {
                BaseColor = RED;
                ColorColor = RED;
                WhiteColor = ColorBrightness(RED, Factor);
            }
            if (curNote.pDrumAct && player.stats->overdriveFill >= 0.25
                && !player.stats->Overdrive) {
                NoteScale.z = 2.0f;
                ColorColor = GREEN;
                WhiteColor = ColorBrightness(GREEN, Factor);
            }
            CymbalParts[mBASE].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BaseColor;
            CymbalParts[mCOLOR].materials[0].maps[MATERIAL_MAP_DIFFUSE].color =
                ColorColor;
            CymbalParts[mSIDES].materials[0].maps[MATERIAL_MAP_DIFFUSE].color =
                WhiteColor;
            CymbalParts[mBASE].materials[0].shader = gprAssets.HighwayFade;
            CymbalParts[mCOLOR].materials[0].shader = gprAssets.HighwayFade;
            CymbalParts[mSIDES].materials[0].shader = gprAssets.HighwayFade;

            DrawModelEx(CymbalParts[mBASE], NotePos, { 0 }, 0, NoteScale, BaseColor);
            DrawModelEx(CymbalParts[mCOLOR], NotePos, { 0 }, 0, NoteScale, ColorColor);
            DrawModelEx(CymbalParts[mSIDES], NotePos, { 0 }, 0, NoteScale, WhiteColor);
        } else if (!curNote.hit && curNote.lane == KICK) {
            Model TopModel = gprAssets.KickBottomModel;
            Model BottomModel = gprAssets.KickSideModel;

            Color TopColor = NoteColor;
            Color BottomColor = WHITE;

            if (curNote.miss) {
                TopColor = RED;
                BottomColor = RED;
            }
            if (curNote.renderAsOD) {
                TopColor = WHITE;
                BottomColor = GOLD;
            }

            TopModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = TopColor;
            BottomModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BottomColor;
            TopModel.materials[0].shader = gprAssets.HighwayFade;
            BottomModel.materials[0].shader = gprAssets.HighwayFade;
            DrawModelEx(TopModel, NotePos, { 0 }, 0, NoteScale, TopColor);
            DrawModelEx(BottomModel, NotePos, { 0 }, 0, NoteScale, BottomColor);
        } else if (!curNote.hit) {
            NoteScale = { 1.0f, 1.0f, 0.5f };
            Color InnerColor = NoteColor;
            Color BaseColor = WHITE;
            Color SideColor = WHITE;

            if (curNote.renderAsOD) {
                InnerColor = WHITE;
                BaseColor = WHITE;
                SideColor = GOLD;
            }
            if (curNote.miss) {
                InnerColor = RED;
                BaseColor = RED;
                SideColor = RED;
            }
            if (curNote.pDrumAct && player.stats->overdriveFill >= 0.25
                && !player.stats->Overdrive) {
                NoteScale.y = 2.0f;
                InnerColor = GREEN;
                BaseColor = RED;
                SideColor = RED;
            }
            DrumParts[mBASE].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BaseColor;
            DrumParts[mCOLOR].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = InnerColor;
            DrumParts[mSIDES].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = SideColor;
            DrumParts[mBASE].materials[0].shader = gprAssets.HighwayFade;
            DrumParts[mCOLOR].materials[0].shader = gprAssets.HighwayFade;
            DrumParts[mSIDES].materials[0].shader = gprAssets.HighwayFade;

            DrawModelEx(DrumParts[mBASE], NotePos, { 0 }, 0, NoteScale, BaseColor);
            DrawModelEx(DrumParts[mCOLOR], NotePos, { 0 }, 0, NoteScale, InnerColor);
            DrawModelEx(DrumParts[mSIDES], NotePos, { 0 }, 0, NoteScale, SideColor);
        }

        nDrawDrumsHitEffects(player, curNote, curSongTime, notePosX);
    }
    EndMode3D();

    DrawRenderTexture();
}

void gameplayRenderer::RenderPDrumsHighway(Player &player, Song song, double curSongTime) {
    StartRenderTexture();
    BeginBlendModeSeparate();

    PlayerGameplayStats *&stats = player.stats;

    float highwayLength = (defaultHighwayLength * 1.5f) * player.HighwayLength;
    float highwayPosShit = ((20) * (1 - player.HighwayLength));

    DrawHighwayMesh(
        player.HighwayLength,
        player.stats->Overdrive,
        player.stats->overdriveActiveTime,
        curSongTime,
        false
    );

    DrawRenderTexture();

    StartRenderTexture();
    BeginBlendModeSeparate();

    int DoIn = 1;
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DoIn, SHADER_UNIFORM_INT
    );
    BeginShaderMode(gprAssets.HighwayFade);
    if (!song.beatLines.empty()) {
        DrawBeatlines(player, song, highwayLength, curSongTime);
    }

    if (!song.parts[player.Instrument]->charts[player.Difficulty].overdrive.events.empty(
        )) {
        DrawOverdrive(
            player,
            song.parts[player.Instrument]->charts[player.Difficulty],
            highwayLength,
            curSongTime
        );
    }
    if (!song.parts[player.Instrument]->charts[player.Difficulty].solos.events.empty()) {
        DrawSolo(
            player,
            song.parts[player.Instrument]->charts[player.Difficulty],
            highwayLength,
            curSongTime
        );
    }
    if (!song.parts[player.Instrument]->charts[player.Difficulty].fills.events.empty()
        && player.stats->overdriveFill >= 0.25 && !player.stats->Overdrive) {
        DrawFill(
            player,
            song.parts[player.Instrument]->charts[player.Difficulty],
            highwayLength,
            curSongTime
        );
    }
    float darkYPos = 0.015f;
    float HighwayFadeStart = highwayLength + (smasherPos * 2);
    float HighwayEnd = highwayLength + (smasherPos * 3);
    SetShaderValue(
        gprAssets.HighwayFade,
        gprAssets.HighwayFadeStartLoc,
        &HighwayFadeStart,
        SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.HighwayFade,
        gprAssets.HighwayFadeEndLoc,
        &HighwayEnd,
        SHADER_UNIFORM_FLOAT
    );
    EndShaderMode();
    int DontIn = 0;
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT
    );
    HighwayFadeStart = highwayLength + (smasherPos * 2);
    HighwayEnd = highwayLength + (smasherPos * 4);
    SetShaderValue(
        gprAssets.HighwayFade,
        gprAssets.HighwayFadeStartLoc,
        &HighwayFadeStart,
        SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.HighwayFade,
        gprAssets.HighwayFadeEndLoc,
        &HighwayEnd,
        SHADER_UNIFORM_FLOAT
    );

    DrawRenderTexture();

    StartRenderTexture();
    BeginBlendModeSeparate();

    for (int i = 0; i < 4; i++) {
        Color NoteColor = RED;
        if (i == 1)
            NoteColor = YELLOW;
        else if (i == 2)
            NoteColor = BLUE;
        else if (i == 3)
            NoteColor = GREEN;
        gprAssets.smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        // gprAssets.smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        Model smasherTopModel = InnerTomSmasher;
        Color smasherColor =
            ColorBrightness(ColorContrast(player.AccentColor, -0.25), 0.5);
        InnerTomSmasher.materials[0].maps[MATERIAL_MAP_ALBEDO].color = smasherColor;
        InnerTomSmasher.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
            gprAssets.smasherInnerTex;
        OuterTomSmasher.materials[0].maps[MATERIAL_MAP_ALBEDO].color = smasherColor;
        float RandomRotationToSelect = GetRandomValue(-1, 1) / 10.0f;
        // constexpr float thing[4] {0.15, 0.1, -0.1, -0.15};
        Vector3 RotationAxis { 1, 0, RandomRotationToSelect };
        DrawModelEx(
            InnerTomSmasher,
            Vector3 { (diffDistance(player.Difficulty, true) - (float)(i * 1.25))
                          - 0.125f,
                      player.stats->drumSmasherHeights.at(i),
                      smasherPos },
            RotationAxis,
            player.stats->drumSmasherRotations[i],
            { 1.25, 1.0, 1.0 },
            WHITE
        );
        DrawModelEx(
            OuterTomSmasher,
            Vector3 { (diffDistance(player.Difficulty, true) - (float)(i * 1.25))
                          - 0.125f,
                      player.stats->drumSmasherHeights.at(i),
                      smasherPos },
            RotationAxis,
            player.stats->drumSmasherRotations[i],
            { 1.25, 1.0, 1.0 },
            WHITE
        );
        smasherTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        if (player.stats->HeldFrets[i] || player.stats->HeldFretsAlt[i]) {
            smasherTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
                gprAssets.smasherTopPressedTex;
        } else {
            smasherTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture =
                gprAssets.smasherTopUnpressedTex;
        }

        DrawModelEx(
            smasherTopModel,
            Vector3 { (diffDistance(player.Difficulty, true) - (float)(i * 1.25))
                          - 0.125f,
                      player.stats->drumSmasherHeights.at(i),
                      smasherPos },
            RotationAxis,
            player.stats->drumSmasherRotations[i],
            { 1.25, 1.0, 1.0 },
            WHITE
        );
    }
    DrawModelEx(
        InnerKickSmasher, Vector3 { 0, 0, smasherPos }, { 0 }, 0, { 1.0, 1.0, 1.0 }, ORANGE
    );
    DrawModelEx(
        OuterKickSmasher,
        Vector3 { 0, 0, smasherPos },
        { 0 },
        0,
        { 1.0, 1.0, 1.0 },
        ColorBrightness(player.AccentColor, 0.5)
    );
    DrawRenderTexture();
}

void gameplayRenderer::nDrawDrumsHitEffects(
    Player &player, Note note, double curSongTime, float notePosX
) {
    // oh my good fucking lord
    double HitShakerDuration = 0.3f;
    double HitShakerIntroDuration = 0.05f;
    double HitShakerOutroDuration = 0.25f;

    DrawPerfectText(note.hitTime, curSongTime, player);
    if (note.hit && curSongTime < note.hitTime + (HitShakerDuration)
        && note.lane != KICK) {
        float RotationDirection = note.perfect ? -10 : 10;
        if (curSongTime < note.hitTime + HitShakerIntroDuration) {
            double TimeSinceHit = curSongTime - (note.hitTime);
            player.stats->drumSmasherRotations.at(note.lane - 1) = Remap(
                TimeSinceHit / HitShakerIntroDuration, 0, 1.0, 0, -RotationDirection
            );
            player.stats->drumSmasherHeights.at(note.lane - 1) = Remap(
                getEasingFunction(EaseInOutQuart)(TimeSinceHit / HitShakerIntroDuration),
                0,
                1.0,
                0,
                0.05
            );
        } else {
            double TimeSinceHit = curSongTime - (note.hitTime + HitShakerIntroDuration);
            player.stats->drumSmasherRotations.at(note.lane - 1) = Remap(
                getEasingFunction(EaseInOutBounce)(TimeSinceHit / HitShakerOutroDuration),
                1.0,
                0,
                0,
                RotationDirection
            );
            player.stats->drumSmasherHeights.at(note.lane - 1) = Remap(
                getEasingFunction(EaseOutBounce)(TimeSinceHit / HitShakerOutroDuration),
                1.0,
                0,
                0,
                0.05
            );
        }
    }

    if (note.hit && curSongTime < note.hitTime + HitAnimDuration) {
        double TimeSinceHit = curSongTime - note.hitTime;
        unsigned char HitAlpha = Remap(
            getEasingFunction(EaseInBack)(TimeSinceHit / HitAnimDuration), 0, 1.0, 196, 0
        );

        float Width = note.lane == KICK ? 5.0f : 1.3f;
        float Height = note.lane == KICK ? 0.125f : 0.25f;
        float Length = note.lane == KICK ? 0.5f : 0.75f;
        float yPos = note.lane == KICK ? 0 : 0.125f;
        Color BoxColor = note.perfect ? Color { 255, 215, 0, HitAlpha }
                                      : Color { 255, 255, 255, HitAlpha };
        // DrawCube(Vector3 { notePosX, yPos, smasherPos }, Width, Height, Length,
        // BoxColor);
    }
    EndBlendMode();
    float KickBounceDuration = 0.75f;

    if (note.hit && note.lane == KICK
        && curSongTime < note.hitTime + KickBounceDuration) {
        double TimeSinceHit = curSongTime - note.hitTime;
        float height = 7.25f;
        if (ThePlayerManager.PlayersActive > 3) {
            height = 10;
        }
        float CameraPos = Remap(
            getEasingFunction(EaseOutBounce)(TimeSinceHit / KickBounceDuration),
            0,
            1.0,
            height - 0.35f,
            height
        );
        cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel].position.y =
            CameraPos;
    }
}

void gameplayRenderer::nDrawFiveLaneHitEffects(
    Player &player, Note note, double curSongTime, float notePosX, int lane
) {
    double PerfectHitAnimDuration = 1.0f;
    double HitShakerDuration = 0.4f;

    if (note.hit && curSongTime < note.hitTime + (HitShakerDuration)) {
        float MaxHeight = 0.3f;
        float MinHeight = 0;
        double TimeSinceHit = curSongTime - (note.hitTime);
        double PercentBetweenKey =
            getEasingFunction(EaseOutBounce)(TimeSinceHit / HitShakerDuration);
        player.stats->fiveLaneSmasherHeights.at(lane) = Clamp(
            Remap(PercentBetweenKey, 1.00, 0, MinHeight, MaxHeight), MinHeight, MaxHeight
        );
    }
    if (curSongTime < player.stats->overdriveHitTime + (OverdriveAnimationDuration)
        && curSongTime > player.stats->overdriveHitTime) {
        double TimeSinceHit = curSongTime - player.stats->overdriveHitTime;
        float height = Height;
        float KickHeight = 0.35f;
        if (ThePlayerManager.PlayersActive == 1) {
            height = Height1p;
            KickHeight = 0.25f;
        };
        if (ThePlayerManager.PlayersActive > 3) {
            height = Height4p;
        }
        float CameraPos = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / OverdriveAnimationDuration),
            0,
            1.0,
            height - KickHeight,
            height
        );
        unsigned char SidesR = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / OverdriveAnimationDuration),
            0,
            1.0,
            YELLOW.r,
            player.AccentColor.r

        );
        unsigned char SidesG = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / OverdriveAnimationDuration),
            0,
            1.0,
            YELLOW.g,
            player.AccentColor.g

        );
        unsigned char SidesB = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / OverdriveAnimationDuration),
            0,
            1.0,
            YELLOW.b,
            player.AccentColor.b

        );
        cameraVectors[ThePlayerManager.PlayersActive - 1][cameraSel].position.y =
            CameraPos;
        gprAssets.expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
            Color { SidesR, SidesG, SidesB, 255 };
    } else {
        gprAssets.expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
            player.AccentColor;
    }

    EnableFadeShaderForSmallObjectsThatUseRaylibMeshFuncs();
    if (note.hit && curSongTime < note.hitTime + HitAnimDuration) {
        double TimeSinceHit = curSongTime - note.hitTime;
        unsigned char HitAlpha = Remap(
            getEasingFunction(EaseInBack)(TimeSinceHit / HitAnimDuration), 0, 1.0, 196, 0
        );
        Color PerfectColor = Color { 255, 215, 0, HitAlpha };
        Color GoodColor = Color { 255, 255, 255, HitAlpha };
        Color BoxColor = note.perfect ? PerfectColor : GoodColor;

        DrawCube(Vector3 { notePosX, 0.125, smasherPos }, 1.0f, 0.25f, 0.5f, BoxColor);
    }
    DisableFadeShaderForSmallObjectsThatUseRaylibMeshFuncs();
}

void gameplayRenderer::nDrawPlasticNote(
    Note note, Color accentColor, Color noteColor, float notePosX, float noteTime
) {
    // why the fuck did i separate sustains and non-sustain note drawing?????

    Vector3 NotePos = { notePosX, 0, noteTime };
    Color InnerColor = noteColor;
    Color BaseColor = WHITE;
    Color SideColor = ColorBrightness(ColorContrast(accentColor, -0.25), 0.5);
    ;

    if (note.renderAsOD) {
        InnerColor = WHITE;
        BaseColor = WHITE;
        SideColor = GOLD;
    }
    if (note.miss) {
        InnerColor = RED;
        BaseColor = RED;
        SideColor = RED;
    }
    if (!note.hit) {
        if (note.phopo && !note.pOpen) {
            HopoParts[mBASE].materials[0].shader = gprAssets.HighwayFade;
            HopoParts[mCOLOR].materials[0].shader = gprAssets.HighwayFade;
            HopoParts[mSIDES].materials[0].shader = gprAssets.HighwayFade;

            Vector3 scale = { 1.0f, 1.0f, 0.5f };
            DrawModelEx(HopoParts[mBASE], NotePos, { 0 }, 0, scale, BaseColor);
            DrawModelEx(HopoParts[mCOLOR], NotePos, { 0 }, 0, scale, InnerColor);
            DrawModelEx(HopoParts[mSIDES], NotePos, { 0 }, 0, scale, SideColor);
        } else if (note.pTap) {
            TapParts[mBASE].materials[0].shader = gprAssets.HighwayFade;
            TapParts[mCOLOR].materials[0].shader = gprAssets.HighwayFade;
            TapParts[mSIDES].materials[0].shader = gprAssets.HighwayFade;
            TapParts[mINSIDE].materials[0].shader = gprAssets.HighwayFade;

            Vector3 scale = { 1.0f, 1.0f, 0.5f };
            DrawModelEx(TapParts[mBASE], NotePos, { 0 }, 0, scale, BaseColor);
            DrawModelEx(TapParts[mCOLOR], NotePos, { 0 }, 0, scale, InnerColor);
            DrawModelEx(TapParts[mSIDES], NotePos, { 0 }, 0, scale, SideColor);
            DrawModelEx(TapParts[mINSIDE], NotePos, { 0 }, 0, scale, BLACK);
        } else if (note.pOpen) {
            OpenParts[mBASE].materials[0].shader = gprAssets.HighwayFade;
            OpenParts[mCOLOR].materials[0].shader = gprAssets.HighwayFade;
            OpenParts[mSIDES].materials[0].shader = gprAssets.HighwayFade;
            if (note.phopo) {
                InnerColor = WHITE;
                SideColor = PURPLE;
                BaseColor = PURPLE;
            }
            Vector3 scale = { 1.0f, 1.0f, 0.5f };
            NotePos.x = 0;
            DrawModelEx(OpenParts[mBASE], NotePos, { 0 }, 0, scale, BaseColor);
            DrawModelEx(OpenParts[mCOLOR], NotePos, { 0 }, 0, scale, InnerColor);
            DrawModelEx(OpenParts[mSIDES], NotePos, { 0 }, 0, scale, SideColor);
        } else {
            StrumParts[mBASE].materials[0].shader = gprAssets.HighwayFade;
            StrumParts[mCOLOR].materials[0].shader = gprAssets.HighwayFade;
            StrumParts[mSIDES].materials[0].shader = gprAssets.HighwayFade;

            Vector3 scale = { 1.0f, 1.0f, 0.5f };
            DrawModelEx(StrumParts[mBASE], NotePos, { 0 }, 0, scale, BaseColor);
            DrawModelEx(StrumParts[mCOLOR], NotePos, { 0 }, 0, scale, InnerColor);
            DrawModelEx(StrumParts[mSIDES], NotePos, { 0 }, 0, scale, SideColor);
        }
    }
}

void gameplayRenderer::nDrawPadNote(
    Note note, Color noteColor, float notePosX, float noteScrollPos
) {
    Vector3 NotePos = { notePosX, 0, noteScrollPos };
    if (note.lift && !note.hit) {
        Color BaseColor = noteColor;
        Color SidesColor = WHITE;
        if (note.renderAsOD) {
            BaseColor = WHITE;
            SidesColor = GOLD;
        }
        if (note.miss) {
            BaseColor = RED;
            SidesColor = RED;
        }
        for (auto &part : LiftParts) {
            part.materials[0].shader = gprAssets.HighwayFade;
        }
        DrawModel(LiftParts[0], NotePos, 1.0f, SidesColor);
        DrawModel(LiftParts[1], NotePos, 1.0f, BaseColor);
    } else if (!note.hit) {
        Color InnerColor = noteColor;
        Color SidesColor = WHITE;
        Color BottomColor = WHITE;
        if (note.renderAsOD) {
            InnerColor = WHITE;
            SidesColor = GOLD;
        }
        if (note.miss) {
            InnerColor = RED;
            SidesColor = RED;
            BottomColor = RED;
        }
        for (auto &part : StrumParts) {
            part.materials[0].shader = gprAssets.HighwayFade;
        }
        Vector3 scale = { 1.0f, 1.0f, 0.5f };
        DrawModelEx(StrumParts[mBASE], NotePos, { 0 }, 0, scale, BottomColor);
        DrawModelEx(StrumParts[mCOLOR], NotePos, { 0 }, 0, scale, InnerColor);
        DrawModelEx(StrumParts[mSIDES], NotePos, { 0 }, 0, scale, SidesColor);
    }
}

void gameplayRenderer::nDrawSustain(
    Note note, Color noteColor, float notePosX, float length, float relTime, float relEnd
) {
    float sustainLen = relEnd - relTime;
    Matrix sustainMatrix = MatrixMultiply(
        MatrixScale(1, 1, sustainLen),
        MatrixTranslate(notePosX, 0.01f, relTime + (sustainLen / 2))
    );
    BeginBlendMode(BLEND_ALPHA);
    Material Sustain = gprAssets.sustainMat;

    gprAssets.sustainMat.maps[MATERIAL_MAP_DIFFUSE].color =
        ColorTint(noteColor, { 180, 180, 180, 255 });
    gprAssets.sustainMatHeld.maps[MATERIAL_MAP_DIFFUSE].color =
        ColorBrightness(noteColor, 0.5f);

    // use default... by default
    if (note.held && !note.renderAsOD) // normal held
        Sustain = gprAssets.sustainMatHeld;
    else if (note.held && note.renderAsOD) // OD hold
        Sustain = gprAssets.sustainMatHeldOD;
    else if (!note.held && note.accounted) // released
        Sustain = gprAssets.sustainMatMiss;
    else if (note.renderAsOD) // not hit but OD
        Sustain = gprAssets.sustainMatOD;
    Sustain.shader = gprAssets.HighwayFade;
    Sustain.shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
    DrawMesh(sustainPlane, Sustain, sustainMatrix);
    if (note.held) {
        DrawCube(Vector3 { notePosX, 0.1, smasherPos }, 0.4f, 0.2f, 0.4f, noteColor);
    }

    EndBlendMode();
}

void gameplayRenderer::nDrawCodaLanes(
    float length, double sTime, double cLen, double curTime, float NoteSpeed, int Difficulty
) {
    for (int i = 0; i < 5; i++) {
        double relTime =
            GetNoteOnScreenTime(sTime, curTime, NoteSpeed, Difficulty, length);
        double relEnd = GetNoteOnScreenTime(cLen, curTime, NoteSpeed, Difficulty, length);

        float sustainLen = (length * (float)relEnd) - (length * (float)relTime);
        float notePosX = 2.0f - (1.0f * i);
        Matrix sustainMatrix = MatrixMultiply(
            MatrixScale(1, 1, sustainLen),
            MatrixTranslate(
                notePosX,
                0.015f,
                smasherPos + (length * (float)relTime) + (sustainLen / 2.0f)
            )
        );

        Color noteColor = GRYBO[i];
        // if (TheGameMenu.hehe) {
        //     noteColor = TRANS[i];
        // }
        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
        Material lane = gprAssets.CodaLane;
        lane.shader.locs[SHADER_LOC_MAP_ALBEDO] = gprAssets.HighwayColorLoc;
        lane.maps[MATERIAL_MAP_ALBEDO].color =
            ColorTint(noteColor, { 180, 180, 180, 255 });
        // use default... by default

        DrawMesh(sustainPlane, lane, sustainMatrix);
        EndBlendMode();
    }
}

void gameplayRenderer::nDrawFiveLaneUnderlay(float length, bool pad, Player &player) {
    Color SidesColor;
    unsigned char BaseBrightness = 128;
    bool emh = player.Difficulty < 3 && !player.ClassicMode;
    if (TheSongTime.GetSongTime()
        < player.stats->overdriveHitTime + (OverdriveAnimationDuration)) {
        double TimeSinceHit = TheSongTime.GetSongTime() - player.stats->overdriveHitTime;
        unsigned char SidesR = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / OverdriveAnimationDuration),
            0,
            1.0,
            YELLOW.r,
            WHITE.r

        );
        unsigned char SidesG = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / OverdriveAnimationDuration),
            0,
            1.0,
            YELLOW.g,
            WHITE.g

        );
        unsigned char SidesB = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / OverdriveAnimationDuration),
            0,
            1.0,
            YELLOW.b,
            WHITE.b
        );
        BaseBrightness = Remap(
            getEasingFunction(EaseOutQuart)(TimeSinceHit / OverdriveAnimationDuration),
            0,
            1.0,
            255,
            128
        );
        SidesColor = Color { SidesR, SidesG, SidesB, 255 };
    } else {
        BaseBrightness = 128;
        SidesColor = WHITE;
    }
    for (int i = 0; i < (emh ? 4 : 5); i++) {
        /*
        double relTime =
            GetNoteOnScreenTime(sTime, curTime, NoteSpeed, Difficulty, length);
        double relEnd = GetNoteOnScreenTime(cLen, curTime, NoteSpeed, Difficulty, length);
*/
        float sustainLen = length * 2;
        float notePosX = (emh ? 1.5f : 2.0f) - (1.0f * i);
        Matrix sustainMatrix = MatrixMultiply(
            MatrixScale(1, 1, sustainLen),
            MatrixTranslate(notePosX, 0.005f, length - 0.1f)
        );
        int divLane = 0;
        float Alpha = 1.0f;
        unsigned char A = 180;
        if (pad) {
            if (i == 0 || i == 2)
                divLane = 2;
            if (i == 1 || (emh ? i == 3 : i == 4))
                divLane = 0;
            if (i == 3 && !emh)
                continue;
            // if (i == 1 || i == 2) A = 255;
        } else {
            if (i <= 1)
                divLane = 2;
            if (i == 2)
                divLane = 1;
            if (i >= 3)
                divLane = 0;
            if (i == 1 || i == 3)
                Alpha = 0.25f;
            if (i == 2)
                Alpha = 0.15f;
        }
        if (i == 0 || i == 4) {
            Alpha = 0.5f;
        }

        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
        Material lane = gprAssets.CodaLane;
        lane.shader.locs[SHADER_LOC_MAP_ALBEDO] = gprAssets.HighwayColorLoc;
        lane.maps[MATERIAL_MAP_ALBEDO].texture = dividerTex[divLane];
        // use default... by default
        if (player.stats->Overdrive)
            SidesColor = YELLOW;
        bool GrooveFlash = false;
        if (player.stats->IsBassOrVox()) {
            if (player.stats->noODmultiplier() >= 6) {
                GrooveFlash = true;
            }
        } else {
            if (player.stats->noODmultiplier() >= 4) {
                GrooveFlash = true;
            }
        }
        if (GrooveFlash && (i == 0 || (emh ? i == 3 : i == 4))) {
            lane.maps[MATERIAL_MAP_ALBEDO].color = ColorTint(
                SidesColor,
                { 255, 255, 255, TickToChar(CurrentTick, BaseBrightness, 256, 480) }
            );

        } else {
            lane.maps[MATERIAL_MAP_ALBEDO].color =
                ColorAlpha(ColorTint(SidesColor, { A, A, A, BaseBrightness }), Alpha);
        }
        DrawMesh(dividerPlane, lane, sustainMatrix);
        EndBlendMode();
    }
}

void gameplayRenderer::nDrawSoloSides(
    float length,
    double sTime,
    double cLen,
    double curTime,
    float NoteSpeed,
    int Difficulty,
    bool Classic
) {
    for (int i = 0; i < (Classic || Difficulty == 3 ? 5 : 4); i++) {
        if (Classic || Difficulty == 3) {
            if (i == 1 || i == 2 || i == 3)
                continue;
        } else {
            if (i == 1 || i == 2)
                continue;
        }
        double relTime =
            GetNoteOnScreenTime(sTime, curTime, NoteSpeed, Difficulty, length);
        double relEnd = GetNoteOnScreenTime(cLen, curTime, NoteSpeed, Difficulty, length);

        float sustainLen = (length * (float)relEnd) - (length * (float)relTime);
        float notePosX = diffDistance(Difficulty, Classic) - (1.0f * i);
        Matrix sustainMatrix = MatrixMultiply(
            MatrixScale(1, 1, sustainLen),
            MatrixTranslate(
                notePosX,
                0.005f,
                smasherPos + (length * (float)relTime) + (sustainLen / 2.0f)
            )
        );

        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
        Material lane = gprAssets.SoloSides;
        lane.maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.soloTexture;
        lane.maps[MATERIAL_MAP_ALBEDO].color = SKYBLUE;
        lane.maps[MATERIAL_MAP_DIFFUSE].color = SKYBLUE;
        lane.shader.locs[SHADER_LOC_MAP_ALBEDO] = gprAssets.HighwayColorLoc;
        // use default... by default
        if (i == 0) {
            lane.maps[MATERIAL_MAP_ALBEDO].texture = invSoloTex;
        }
        DrawMesh(soloPlane, lane, sustainMatrix);
        EndBlendMode();
    }
}
