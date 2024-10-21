//
// Created by marie on 09/06/2024.
//

#include "gameplayRenderer.h"

#include "assets.h"
#include "enctime.h"
#include "settings-old.h"
#include "menus/gameMenu.h"
#include "raymath.h"
#include "menus/uiUnits.h"
#include "rlgl.h"
#include "easing/easing.h"
#include "song/audio.h"
#include "users/playerManager.h"
#include "util/enclog.h"

Assets &gprAssets = Assets::getInstance();
SettingsOld &gprSettings = SettingsOld::getInstance();
AudioManager &gprAudioManager = AudioManager::getInstance();
Units &gprU = Units::getInstance();

// Color accentColor = {255,0,255,255};
float defaultHighwayLength = 11.5f;
Color OverdriveColor = { 255, 200, 0, 255 };

#define LETTER_BOUNDRY_SIZE 0.25f
#define TEXT_MAX_LAYERS 32
#define LETTER_BOUNDRY_COLOR VIOLET

std::vector<Color> GRYBO = { GREEN, RED, YELLOW, BLUE, ORANGE };
std::vector<Color> TRANS = { SKYBLUE, PINK, WHITE, PINK, SKYBLUE };

void BeginBlendModeSeparate() {
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);\
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
}

enum DrumNotes {
    KICK,
    RED_DRUM,
    YELLOW_DRUM,
    BLUE_DRUM,
    GREEN_DRUM
};

bool SHOW_LETTER_BOUNDRY = false;
bool SHOW_TEXT_BOUNDRY = false;

// code from examples lol
static void DrawTextCodepoint3D(
    Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint
) {
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to
    // '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize / (float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding)
        / (float)font.baseSize * scale;
    position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding)
        / (float)font.baseSize * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow
    // shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding,
                         font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f * font.glyphPadding,
                         font.recs[index].height + 2.0f * font.glyphPadding };

    float width = (float)(font.recs[index].width + 2.0f * font.glyphPadding)
        / (float)font.baseSize * scale;
    float height = (float)(font.recs[index].height + 2.0f * font.glyphPadding)
        / (float)font.baseSize * scale;

    if (font.texture.id > 0) {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f
        // -> 1.0f)
        const float tx = srcRec.x / font.texture.width;
        const float ty = srcRec.y / font.texture.height;
        const float tw = (srcRec.x + srcRec.width) / font.texture.width;
        const float th = (srcRec.y + srcRec.height) / font.texture.height;

        if (SHOW_LETTER_BOUNDRY) {
            DrawCubeWiresV(
                Vector3 { position.x + width / 2, position.y, position.z + height / 2 },
                Vector3 { width, LETTER_BOUNDRY_SIZE, height },
                LETTER_BOUNDRY_COLOR
            );
        }

        rlCheckRenderBatchLimit(4 + 4 * backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        // Front Face
        rlNormal3f(0.0f, 1.0f, 0.0f); // Normal Pointing Up
        rlTexCoord2f(tx, ty);
        rlVertex3f(x, y, z); // Top Left Of The Texture and Quad
        rlTexCoord2f(tx, th);
        rlVertex3f(x, y, z + height); // Bottom Left Of The Texture and Quad
        rlTexCoord2f(tw, th);
        rlVertex3f(x + width, y, z + height); // Bottom Right Of The Texture and Quad
        rlTexCoord2f(tw, ty);
        rlVertex3f(x + width, y, z); // Top Right Of The Texture and Quad

        if (backface) {
            // Back Face
            rlNormal3f(0.0f, -1.0f, 0.0f); // Normal Pointing Down
            rlTexCoord2f(tx, ty);
            rlVertex3f(x, y, z); // Top Right Of The Texture and Quad
            rlTexCoord2f(tw, ty);
            rlVertex3f(x + width, y, z); // Top Left Of The Texture and Quad
            rlTexCoord2f(tw, th);
            rlVertex3f(x + width, y, z + height); // Bottom Left Of The Texture and Quad
            rlTexCoord2f(tx, th);
            rlVertex3f(x, y, z + height); // Bottom Right Of The Texture and Quad
        }
        rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

static void DrawText3D(
    Font font,
    const char *text,
    Vector3 position,
    float fontSize,
    float fontSpacing,
    float lineSpacing,
    bool backface,
    Color tint
) {
    int length = TextLength(text); // Total length in bytes of the text, scanned by
                                   // codepoints in loop

    float textOffsetY = 0.0f; // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f; // Offset X to next character to draw

    float scale = fontSize / (float)font.baseSize;

    for (int i = 0; i < length;) {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found
        // (and return 0x3f) but we need to draw all of the bad bytes using the '?' symbol
        // moving one byte
        if (codepoint == 0x3f)
            codepointByteCount = 1;

        if (codepoint == '\n') {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += scale + lineSpacing / (float)font.baseSize * scale;
            textOffsetX = 0.0f;
        } else {
            if ((codepoint != ' ') && (codepoint != '\t')) {
                DrawTextCodepoint3D(
                    font,
                    codepoint,
                    Vector3 {
                        position.x + textOffsetX, position.y, position.z + textOffsetY },
                    fontSize,
                    backface,
                    tint
                );
            }

            if (font.glyphs[index].advanceX == 0)
                textOffsetX += (float)(font.recs[index].width + fontSpacing)
                    / (float)font.baseSize * scale;
            else
                textOffsetX += (float)(font.glyphs[index].advanceX + fontSpacing)
                    / (float)font.baseSize * scale;
        }

        i += codepointByteCount; // Move text bytes counter to next codepoint
    }
}

static Vector3 MeasureText3D(
    Font font, const char *text, float fontSize, float fontSpacing, float lineSpacing
) {
    int len = TextLength(text);
    int tempLen = 0; // Used to count longer text line num chars
    int lenCounter = 0;

    float tempTextWidth = 0.0f; // Used to count longer text line width

    float scale = fontSize / (float)font.baseSize;
    float textHeight = scale;
    float textWidth = 0.0f;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    for (int i = 0; i < len; i++) {
        lenCounter++;

        int next = 0;
        letter = GetCodepoint(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad byte is found
        // (and return 0x3f) but we need to draw all of the bad bytes using the '?' symbol
        // so to not skip any we set next = 1
        if (letter == 0x3f)
            next = 1;
        i += next - 1;

        if (letter != '\n') {
            if (font.glyphs[index].advanceX != 0)
                textWidth += (font.glyphs[index].advanceX + fontSpacing)
                    / (float)font.baseSize * scale;
            else
                textWidth += (font.recs[index].width + font.glyphs[index].offsetX)
                    / (float)font.baseSize * scale;
        } else {
            if (tempTextWidth < textWidth)
                tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth = 0.0f;
            textHeight += scale + lineSpacing / (float)font.baseSize * scale;
        }

        if (tempLen < lenCounter)
            tempLen = lenCounter;
    }

    if (tempTextWidth < textWidth)
        tempTextWidth = textWidth;

    Vector3 vec = { 0 };
    vec.x = tempTextWidth
        + (float)((tempLen - 1) * fontSpacing / (float)font.baseSize * scale); // Adds
                                                                               // chars
                                                                               // spacing
                                                                               // to
                                                                               // measure
    vec.y = 0.25f;
    vec.z = textHeight;

    return vec;
}

double startTime = 0.0;
bool highwayRaiseFinish = false;

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

    GameplayRenderTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    Image invSolo = LoadImageFromTexture(gprAssets.soloTexture);
    ImageFlipHorizontal(&invSolo);
    invSoloTex = LoadTextureFromImage(invSolo);
    SetTextureWrap(invSoloTex, TEXTURE_WRAP_CLAMP);
    SetTextureWrap(gprAssets.soloTexture, TEXTURE_WRAP_CLAMP);
    UnloadImage(invSolo);

    Texture2D NoteSide = LoadTexture((noteModelPath / "NoteSide.png").string().c_str());
    Texture2D NoteColor = LoadTexture((noteModelPath / "NoteColor.png").string().c_str());
    Texture2D NoteBottom = LoadTexture((noteModelPath / "NoteBottom.png").string().c_str());
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

        // Y UP!!!! REMEMBER!!!!!!
    //							  x,    y,     z
    //                         0.0f, 5.0f, -3.5f
    //								 6.5f

    // singleplayer
    // 0.0f, 0.0f, 6.5f
    float Height = 7.25f;
    float Back = -10.0f;
    float FOV = 45.0f;
    float TargetDistance = 20.0f;
    TheGameRenderer.camera1p.position = Vector3 { 0.0f, Height, Back };
    TheGameRenderer.camera1p.target = Vector3 { 0.0f, 0.0f, TargetDistance };
    TheGameRenderer.camera1p.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera1p.fovy = FOV;

    TheGameRenderer.camera1pVector.push_back(TheGameRenderer.camera1p);

    // 2 player
    float SideDisplacement2p = 0.75f;

    TheGameRenderer.camera2p1.position = Vector3 { SideDisplacement2p, Height, Back };
    TheGameRenderer.camera2p1.target = Vector3 { SideDisplacement2p, 0.0f, TargetDistance };
    TheGameRenderer.camera2p1.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera2p1.fovy = FOV;

    TheGameRenderer.camera2p2.position = Vector3 { -SideDisplacement2p, Height, Back };
    TheGameRenderer.camera2p2.target = Vector3 { -SideDisplacement2p, 0.0f, TargetDistance };
    TheGameRenderer.camera2p2.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera2p2.fovy = FOV;

    TheGameRenderer.camera2pVector.push_back(TheGameRenderer.camera2p1);
    TheGameRenderer.camera2pVector.push_back(TheGameRenderer.camera2p2);

    // 3 player
    float SideDisplacement3p = 1.25f;
    TheGameRenderer.camera3p1.position = Vector3 { 0.0f, Height, Back };
    TheGameRenderer.camera3p1.target = Vector3 { 0.0f, 0.0f, TargetDistance };
    TheGameRenderer.camera3p1.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera3p1.fovy = FOV;
    TheGameRenderer.camera3pVector.push_back(TheGameRenderer.camera3p1);

    TheGameRenderer.camera3p2.position = Vector3 { SideDisplacement3p, Height, Back };
    TheGameRenderer.camera3p2.target = Vector3 { SideDisplacement3p, 0.0f, TargetDistance };
    TheGameRenderer.camera3p2.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera3p2.fovy = FOV;
    TheGameRenderer.camera3pVector.push_back(TheGameRenderer.camera3p2);

    TheGameRenderer.camera3p3.position = Vector3 { -SideDisplacement3p, Height, Back };
    TheGameRenderer.camera3p3.target = Vector3 { -SideDisplacement3p, 0.0f, TargetDistance };
    TheGameRenderer.camera3p3.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera3p3.fovy = FOV;
    TheGameRenderer.camera3pVector.push_back(TheGameRenderer.camera3p3);

    float SideDisplacement4p = 3.0f;
    float SideDisplacement4p2 = 1.0f;
    float Back4p = -10.0f;
    float Height4p = 10.0f;
    TheGameRenderer.camera4p1.position = Vector3 { SideDisplacement4p2, Height4p, Back4p };
    TheGameRenderer.camera4p1.target = Vector3 { SideDisplacement4p2, 0.0f, TargetDistance };
    TheGameRenderer.camera4p1.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera4p1.fovy = FOV;
    TheGameRenderer.camera4pVector.push_back(TheGameRenderer.camera4p1);

    TheGameRenderer.camera4p2.position = Vector3 { SideDisplacement4p, Height4p, Back4p };
    TheGameRenderer.camera4p2.target = Vector3 { SideDisplacement4p, 0.0f, TargetDistance };
    TheGameRenderer.camera4p2.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera4p2.fovy = FOV;
    TheGameRenderer.camera4pVector.push_back(TheGameRenderer.camera4p2);

    TheGameRenderer.camera4p3.position = Vector3 { -SideDisplacement4p, Height4p, Back4p };
    TheGameRenderer.camera4p3.target = Vector3 { -SideDisplacement4p, 0.0f, TargetDistance };
    TheGameRenderer.camera4p3.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera4p3.fovy = FOV;
    TheGameRenderer.camera4pVector.push_back(TheGameRenderer.camera4p3);

    TheGameRenderer.camera4p4.position = Vector3 { -SideDisplacement4p2, Height4p, Back4p };
    TheGameRenderer.camera4p4.target = Vector3 { -SideDisplacement4p2, 0.0f, TargetDistance };
    TheGameRenderer.camera4p4.up = Vector3 { 0.0f, 1.0f, 0.0f };
    TheGameRenderer.camera4p4.fovy = FOV;
    TheGameRenderer.camera4pVector.push_back(TheGameRenderer.camera4p4);

    TheGameRenderer.cameraVectors.push_back(TheGameRenderer.camera1pVector);
    TheGameRenderer.cameraVectors.push_back(TheGameRenderer.camera2pVector);
    TheGameRenderer.cameraVectors.push_back(TheGameRenderer.camera3pVector);
    TheGameRenderer.cameraVectors.push_back(TheGameRenderer.camera4pVector);

}


void gameplayRenderer::RaiseHighway() {
    if (!highwayInAnimation) {
        startTime = GetTime();
        highwayRaiseFinish = false;
        highwayInAnimation = true;
    }
    if (GetTime() <= startTime + animDuration && highwayInAnimation) {
        double timeSinceStart = GetTime() - startTime;
        highwayLevel = Remap(
            1.0 - getEasingFunction(EaseOutExpo)(timeSinceStart / animDuration),
            0,
            1.0,
            0,
            -GetScreenHeight()
        );
        highwayRaiseFinish = true;
    }
    if (GetTime() >= startTime + animDuration && highwayRaiseFinish) {
        highwayInEndAnim = true;
    }
};

void gameplayRenderer::LowerHighway() {

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

};

void gameplayRenderer::NoteMultiplierEffect(
    double time, double hitTime, bool miss, Player *player
) {
    if (time < hitTime + multiplierEffectTime && time > hitTime) {
        Color missColor = { 200, 0, 0, 255 };
        Color comboColor = { 200, 200, 200, 255 };

        Color RingDefault = ColorBrightness(player->AccentColor, -0.7);
        unsigned char r = RingDefault.r;
        unsigned char g = RingDefault.g;
        unsigned char b = RingDefault.b;
        unsigned char a = 255;
        double TimeSinceHit = time - hitTime;
        if (!miss) {
            if (player->stats->Combo <= player->stats->maxMultForMeter() * 10
                && player->stats->Combo % 10 == 0) {
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
            if (player->stats->Combo != 0) {
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
    Rectangle res = { 0,0,float(GetScreenWidth()), float(GetScreenHeight()) };
    // SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, render_texture.texture);
    // SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res, SHADER_UNIFORM_VEC2);
    // BeginShaderMode(gprAssets.fxaa);
    //DrawTextureRec(render_texture.texture, res, {0}, WHITE);

    DrawTexturePro(
        GameplayRenderTexture.texture,
        source,
        res,
        { renderPos, highwayLevel },
        0,
        WHITE
    );
    // EndShaderMode();
}
void gameplayRenderer::RenderPadNotes(
    Player *player, Chart &curChart, double time, float length
) {
    float diffDistance = player->Difficulty == 3 ? 2.0f : 1.5f;
    float lineDistance = player->Difficulty == 3 ? 1.5f : 1.0f;
    float DiffMultiplier =
        Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    StartRenderTexture();
    // glDisable(GL_CULL_FACE);
    for (int lane = 0; lane < (player->Difficulty == 3 ? 5 : 4); lane++) {
        for (int i = player->stats->curNoteIdx[lane];
             i < curChart.notes_perlane[lane].size();
             i++) {

            Color NoteColor = TheGameMenu.hehe && player->Difficulty == 3 ? TRANS[lane]
                                                                    : player->AccentColor;


            Note &curNote = curChart.notes[curChart.notes_perlane[lane][i]];
            // if (curNote.hit) {
            //	player->stats->totalOffset += curNote.HitOffset;
            // }

            if (!curNote.hit && !curNote.accounted && curNote.time + goodBackend < time
                && !TheSongTime.SongComplete()) {
                curNote.miss = true;
                player->stats->MissNote();
                player->stats->Combo = 0;
                curNote.accounted = true;
            } else if (player->Bot) {
                if (!curNote.hit && !curNote.accounted && curNote.time < time
                    && player->stats->curNoteInt < curChart.notes.size() && !TheSongTime.SongComplete()) {
                    curNote.hit = true;
                    player->stats->HitNote(false);
                    if (ThePlayerManager.BandStats.Multiplayer) {
                        ThePlayerManager.BandStats.AddNotePoint(
                            curNote.perfect, player->stats->noODmultiplier()
                        );
                    }
                    if (curNote.len > 0)
                        curNote.held = true;
                    curNote.accounted = true;
                    // player->stats->Notes += 1;
                    // player->stats->Combo++;
                    curNote.accounted = true;
                    curNote.hitTime = time;
                }
            }

            if (player->stats->Overdrive) {
                player->stats->overdriveActiveFill += curChart.overdrive.AddOverdrive(player->stats->curODPhrase);
                if (player->stats->overdriveActiveFill > 1.0f)
                    player->stats->overdriveActiveFill = 1.0f;
            } else {
                player->stats->overdriveFill += curChart.overdrive.AddOverdrive(player->stats->curODPhrase);
                if (player->stats->overdriveFill > 1.0f)
                    player->stats->overdriveFill = 1.0f;
            }

            curChart.solos.UpdateEventViaNote(curNote, player->stats->curSolo);
            curChart.overdrive.UpdateEventViaNote(curNote, player->stats->curODPhrase);
            curChart.sections.UpdateEventViaNote(curNote, player->stats->curSection);
            curChart.fills.UpdateEventViaNote(curNote, player->stats->curFill);


            double relTime = GetNoteOnScreenTime(
                curNote.time, time, player->NoteSpeed, player->Difficulty, length
            );
            double relEnd = GetNoteOnScreenTime(
                curNote.time + curNote.len,
                time,
                player->NoteSpeed,
                player->Difficulty,
                length
            );
            float notePosX = diffDistance
                - (1.0f
                   * (float)(player->LeftyFlip
                                 ? (player->Difficulty == 3 ? 4 : 3) - curNote.lane
                                 : curNote.lane));
            if (relTime > 1.5) {
                break;
            }
            if (relEnd > 1.5)
                relEnd = 1.5;
            if (relEnd < -1)
                continue;
            // Vector3 NotePos = {notePosX, 0, smasherPos + (length * (float) relTime)};
            float noteScrollPos = smasherPos + (length * (float)relTime);

            nDrawPadNote(curNote, NoteColor, notePosX, noteScrollPos);

            if ((curNote.len) > 0) {
                if (curNote.hit && curNote.held) {
                    if (curNote.heldTime < (curNote.len * (player->NoteSpeed * DiffMultiplier))) {
                        curNote.heldTime = 0.0 - relTime;
                        // note: this was old sustain scoring code
                        // if (!bot) {
                        // player.sustainScoreBuffer[curNote.lane] =
                        //		(float) (curNote.heldTime / curNote.len) * (12 *
                        //curNote.beatsLen) * 		player.multiplier(player->Instrument);
                        //}
                        if (relTime < 0.0)
                            relTime = 0.0;
                    }
                    if (relEnd <= 0.0) {
                        if (relTime < 0.0)
                            relTime = relEnd;
                        // if (!bot) {
                        // player.score += player.sustainScoreBuffer[curNote.lane];
                        // player.sustainScoreBuffer[curNote.lane] = 0;
                        // }
                        curNote.held = false;
                    }
                } else if (curNote.hit && !curNote.held) {
                    relTime = relTime + curNote.heldTime;
                }
                float sustainLen = (length * (float)relEnd) - (length * (float)relTime);
                Matrix sustainMatrix = MatrixMultiply(
                    MatrixScale(1, 1, sustainLen),
                    MatrixTranslate(
                        notePosX,
                        0.01f,
                        smasherPos + (length * (float)relTime) + (sustainLen / 2.0f)
                    )
                );

                nDrawSustain(curNote, NoteColor, notePosX, sustainMatrix);
            }

            nDrawFiveLaneHitEffects(curNote, time, notePosX);
            NoteMultiplierEffect(time, curNote.time, curNote.miss, player);

            if (relEnd < -1
                && player->stats->curNoteIdx[lane]
                    < curChart.notes_perlane[lane].size() - 1)
                player->stats->curNoteIdx[lane] = i + 1;
        }
    }
    EndMode3D();

    DrawRenderTexture();
}

void gameplayRenderer::RenderClassicNotes(
    Player *player, Chart &curChart, double time, float length
) {
    float diffDistance = 2.0f;
    float lineDistance = 1.5f;
    float DiffMultiplier =
        Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    StartRenderTexture();
    // glDisable(GL_CULL_FACE);

    for (auto &curNote : curChart.notes) {

        //if (curNote.time < TheSongTime.GetFakeStartTime()) {
        //    player->stats->curNoteInt++;
        //    continue;
        //}
        if (curNote.time > TheSongTime.GetSongLength())
            continue;

        if (TheSongList.curSong->BRE.IsNoteInCoda(curNote) && TheSongList.curSong->BRE.IsCodaActive(time)) {
            player->stats->curNoteInt++;
            continue;
        }

        if (!curNote.hit && !curNote.accounted
            && curNote.time + goodBackend + player->InputCalibration < time
            && !TheSongTime.SongComplete() && player->stats->curNoteInt < curChart.notes.size()
            && !player->Bot) {
            Encore::EncoreLog(
                LOG_INFO,
                TextFormat("Missed note at %f, note %01i", time, player->stats->curNoteInt)
            );
            player->stats->MissNote();
            curNote.miss = true;
            curNote.accounted = true;
        } else if (player->Bot) {
            if (!curNote.hit && !curNote.accounted
                && curNote.time + player->InputCalibration < time
                && player->stats->curNoteInt < curChart.notes.size()
                && !TheSongTime.SongComplete()) {
                ThePlayerManager.BandStats.AddClassicNotePoint(
                    curNote.perfect, player->stats->noODmultiplier(), curNote.chordSize
                );
                player->stats->HitPlasticNote(curNote);
                curNote.cHitNote(time, 0);
            }
        }


        if (player->stats->Overdrive) {
            player->stats->overdriveActiveFill += curChart.overdrive.AddOverdrive(player->stats->curODPhrase);
            if (player->stats->overdriveActiveFill > 1.0f)
                player->stats->overdriveActiveFill = 1.0f;
        } else {
            player->stats->overdriveFill += curChart.overdrive.AddOverdrive(player->stats->curODPhrase);
            if (player->stats->overdriveFill > 1.0f)
                player->stats->overdriveFill = 1.0f;
        }

        curChart.solos.UpdateEventViaNote(curNote, player->stats->curSolo);
        curChart.overdrive.UpdateEventViaNote(curNote, player->stats->curODPhrase);
        curChart.sections.UpdateEventViaNote(curNote, player->stats->curSection);
        curChart.fills.UpdateEventViaNote(curNote, player->stats->curFill);

        double relTime = GetNoteOnScreenTime(
            curNote.time, time, player->NoteSpeed, player->Difficulty, length
        );
        double relEnd = GetNoteOnScreenTime(
            curNote.time + curNote.len, time, player->NoteSpeed, player->Difficulty, length
        );
        if (relEnd < -1)
            continue;



        for (int lane : curNote.pLanes) {

            int noteLane = gprSettings.mirrorMode ? 4 - lane : lane;

            Color NoteColor = TheGameMenu.hehe ? TRANS[lane] : GRYBO[lane];
            if (curNote.pOpen)
                NoteColor = PURPLE;

            float notePosX = diffDistance - (1.0f * noteLane);
            if (relTime > 1.5) {
                break;
            }
            if (relEnd > 1.5)
                relEnd = 1.5;

            float NoteScroll = smasherPos + (length * (float)relTime);

            nDrawPlasticNote(curNote, NoteColor, notePosX, NoteScroll);

            // todo: REMOVE SUSTAIN CHECK FROM RENDERER
            // todo: PLEASE REMOVE SUSTAIN CHECK FROM RENDERER
            // todo: PLEASE REMOVE LOGIC FROM RENDERER PLEASE :sob:
            if ((curNote.len) > 0) {
                int pressedMask = 0b000000;

                for (int pressedButtons = 0;
                     pressedButtons < player->stats->HeldFrets.size();
                     pressedButtons++) {
                    if (player->stats->HeldFrets[pressedButtons]
                        || player->stats->HeldFretsAlt[pressedButtons])
                        pressedMask += PlasticFrets[pressedButtons];
                }

                // Note &lastNote =
                //    curChart.notes
                //        [player->stats->curNoteInt == 0 ? 0
                //                                        : player->stats->curNoteInt - 1];
                bool chordMatch =
                    (extendedSustainActive ? pressedMask >= curNote.mask
                                           : pressedMask == curNote.mask);
                bool singleMatch =
                    (extendedSustainActive ? pressedMask >= curNote.mask
                                           : pressedMask >= curNote.mask
                             && pressedMask < (curNote.mask * 2));

                bool noteMatch = (curNote.chord ? chordMatch : singleMatch);

                if (curNote.extendedSustain)
                    Encore::EncoreLog(LOG_INFO, "extended sustain lol");
                if (noteMatch && curNote.held ) {
                    curNote.heldTime = curNote.time - time;
                }
                if ((!noteMatch && curNote.held)
                    && curNote.time + curNote.len + 0.1 > time && !player->Bot) {
                    curNote.held = false;
                    player->stats->Score += player->stats->SustainScoreBuffer[lane];
                    player->stats->SustainScoreBuffer[lane] = 0;
                    if (curNote.extendedSustain == true)
                        extendedSustainActive = false;
                }

                if ((curNote.hit && curNote.held) && curNote.time + curNote.len > time) {
                    if (curNote.heldTime < curNote.len) {
                        player->stats->SustainScoreBuffer[lane] =
                                (float) (curNote.heldTime / curNote.len) * (12 *
                                curNote.beatsLen) *
                               player->stats->multiplier();
                        if (relTime < 0.0)
                            relTime = 0.0;
                    }
                    if (relEnd <= 0.0) {
                        if (relTime < 0.0)
                            relTime = relEnd;
                        curNote.held = false;
                        if (curNote.extendedSustain == true)
                            extendedSustainActive = false;
                    }
                } else if (curNote.hit && !curNote.held && !player->Bot) {
                    relTime = relTime + curNote.heldTime;
                }
                float sustainLen = (length * (float)relEnd) - (length * (float)relTime);
                Matrix sustainMatrix = MatrixMultiply(
                    MatrixScale(1, 1, sustainLen),
                    MatrixTranslate(
                        notePosX,
                        0.01f,
                        smasherPos + (length * (float)relTime) + (sustainLen / 2.0f)
                    )
                );

                nDrawSustain(curNote, NoteColor, notePosX, sustainMatrix);
            }
            nDrawFiveLaneHitEffects(curNote, time, notePosX);
            NoteMultiplierEffect(time, curNote.time, curNote.miss, player);
        }
    }
    EndMode3D();

    DrawRenderTexture();
}

void gameplayRenderer::DrawHitwindow(Player *player, float length) {
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);


        float lineDistance = player->Difficulty == 3 ? 1.5f : 1.0f;
        float Diff = HighwaySpeedDifficultyMultiplier(player->Difficulty);
        double hitwindowFront =
            ((goodFrontend-player->InputCalibration)) * (player->NoteSpeed * Diff) * (11.5f / length);
        double hitwindowBack =
            ((-goodBackend-player->InputCalibration)) * (player->NoteSpeed * Diff) * (11.5f / length);

        float Front = (float)(smasherPos + (length * hitwindowFront));
        float Back = (float)(smasherPos + (length * hitwindowBack));
        Color GoodColor = Color { 128, 128, 128, 96 };
        Color PerfectColor = Color { 255, 255, 255, 64 };
    float GoodY = 0.001f;
    float PerfectY = 0.002f;
    if (true) {
        GoodColor = {0};
        PerfectColor = {0};
    }
        DrawTriangle3D(
            { 0 - lineDistance - 1.0f, GoodY, Back },
            { 0 - lineDistance - 1.0f, GoodY, Front },
            { lineDistance + 1.0f, GoodY, Back },
            GoodColor
        );

        DrawTriangle3D(
            { lineDistance + 1.0f, GoodY, Front },
            { lineDistance + 1.0f, GoodY, Back },
            { 0 - lineDistance - 1.0f, GoodY, Front },
            GoodColor
        );

        double perfectFront =
            ((perfectFrontend-player->InputCalibration)) * (player->NoteSpeed * Diff) * (11.5f / length);
        double perfectBack =
            ((-perfectBackend-player->InputCalibration)) * (player->NoteSpeed * Diff) * (11.5f / length);

        float pFront = (float)(smasherPos + (length * perfectFront));
        float pBack = (float)(smasherPos + (length * perfectBack));

        DrawTriangle3D(
            { 0 - lineDistance - 1.0f, PerfectY, pBack },
            { 0 - lineDistance - 1.0f, PerfectY, pFront },
            { lineDistance + 1.0f, PerfectY, pBack },
            PerfectColor
        );

        DrawTriangle3D(
            { lineDistance + 1.0f, PerfectY, pFront },
            { lineDistance + 1.0f, PerfectY, pBack },
            { 0 - lineDistance - 1.0f, PerfectY, pFront },
            PerfectColor
        );

}

void gameplayRenderer::RenderHud(Player *player, float length) {
    StartRenderTexture();
    BeginBlendModeSeparate();
    DrawHitwindow(player, length);
    DrawModelEx(gprAssets.odFrame, Vector3 { 0, 0.0f, 0.8f }, {0}, 0, {1.0,1.0,1.4}, WHITE);
    DrawModelEx(gprAssets.odBar, Vector3 { 0, 0.0f, 0.8f }, {0}, 0, {1.0,1.0,1.4}, WHITE);

    float FillPct = player->stats->comboFillCalc();
    Vector4 MultFillColor { 0.8, 0.8, 0.8, 1 };

    if (player->stats->IsBassOrVox()) {
        if (player->stats->noODmultiplier() >= 6) {
            MultFillColor = { 0.2, 0.6, 1, 1 };
        }
    } else {
        if (player->stats->noODmultiplier() >= 4) {
            MultFillColor = { 0.2, 0.6, 1, 1 };
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
        ColorBrightness(player->AccentColor, -0.5);
    gprAssets.MultInnerFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
        ColorBrightness(player->AccentColor, -0.4);

    Vector4 FColor = { 0.5f, 0.4f, 0.1, 1.0f };
    float ForFCTime = GetTime();
    int FCING = player->stats->FC ? 2 : 0;
    SetTextureWrap(gprAssets.MultFCTex1, TEXTURE_WRAP_REPEAT);
    SetTextureWrap(gprAssets.MultFCTex2, TEXTURE_WRAP_REPEAT);
    SetTextureWrap(gprAssets.MultFCTex3, TEXTURE_WRAP_REPEAT);
    Color basicColor = player->Bot ? ColorBrightness(SKYBLUE, 0.2)
                                   : ColorBrightness(player->AccentColor, -0.4);
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
    DrawModel(gprAssets.MultInnerDot, Vector3 { 0, 0.0f, 1.1f }, 1.1, WHITE);
    DrawModel(gprAssets.MultFill, Vector3 { 0, 0.0f, 1.1f }, 1.1, WHITE);
    DrawModel(gprAssets.MultOuterFrame, Vector3 { 0, 0.0f, 1.1f }, 1.1, WHITE);
    DrawModel(
        gprAssets.MultInnerFrame,
        Vector3 { 0, 0.0f, 1.1f },
        1.1,
        ColorBrightness(player->AccentColor, -0.4)
    );

    DrawRenderTexture();
}

void gameplayRenderer::RenderGameplay(
    Player *player,
    double time,
    Song song
) {


    if (GameplayRenderTexture.texture.width != GetScreenWidth() || GameplayRenderTexture.texture.height != GetScreenHeight() || IsWindowResized()) {
        GameplayRenderTexture = LoadRenderTexture( GetScreenWidth(), GetScreenHeight());
        // hud_tex.texture.width = width;
        // hud_tex.texture.height = height;
        GenTextureMipmaps(&GameplayRenderTexture.texture);
        SetTextureFilter(GameplayRenderTexture.texture, TEXTURE_FILTER_BILINEAR);
    }
    Chart &curChart = song.parts[player->Instrument]->charts[player->Difficulty];

    float highwayLength = (defaultHighwayLength * 1.5f) * player->HighwayLength;
    player->stats->Difficulty = player->Difficulty;
    player->stats->Instrument = player->Instrument;
    // float multFill = (!player->stats->Overdrive ? (float)(player->stats->multiplier() -
    // 1) : (!player->stats->Multiplayer ? ((float)(player->stats->multiplier() / 2) - 1)
    // / (float)player->stats->maxMultForMeter() : (float)(player->stats->multiplier() -
    // 1)));
    float multFill =
        (!player->stats->Overdrive ? (float)(player->stats->multiplier() - 1)
                                   : ((float)(player->stats->multiplier() / 2) - 1))
        / (float)player->stats->maxMultForMeter();

    // float multFill = 0.0;
    float DiffMultiplier =
        Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    // float relTime = (time) / (highwayLength/4); // 1.75;
    // float highwaySpeedTime = (relTime - smasherPos);
    // float highwaySpeedTime = GetNoteOnScreenTime(0, time, player->NoteSpeed, player->Difficulty, -player->HighwayLength);
    float highwaySpeedTime = (time * player->NoteSpeed) / (2.11f * player->HighwayLength);
    int PlayerComboMax =
        (player->Instrument == PAD_BASS || player->Instrument == PAD_VOCALS
         || player->Instrument == PLASTIC_BASS)
        ? 50
        : 30;
    Color highwayColor = ColorContrast(
        player->AccentColor,
        Clamp(Remap(player->stats->Combo, 0, PlayerComboMax, -0.6f, 0.0f), -0.6, 0.0f)
    );

    Vector4 ColorForHighway = Vector4 { highwayColor.r / 255.0f,
                                        highwayColor.g / 255.0f,
                                        highwayColor.b / 255.0f,
                                        highwayColor.a / 255.0f };

    SetShaderValue(
        gprAssets.Highway,
        gprAssets.HighwayTimeShaderLoc,
        &highwaySpeedTime,
        SHADER_UNIFORM_FLOAT
    );

    //SetShaderValue(
    //    gprAssets.Highway,
    //    gprAssets.HighwayColorShaderLoc,
    //    &ColorForHighway,
    //    SHADER_UNIFORM_VEC4
    //);
    SetShaderValue(
        gprAssets.odMultShader, gprAssets.multLoc, &multFill, SHADER_UNIFORM_FLOAT
    );

    SetShaderValue(
        gprAssets.multNumberShader,
        gprAssets.uvOffsetXLoc,
        &player->stats->uvOffsetX,
        SHADER_UNIFORM_FLOAT
    );
    SetShaderValue(
        gprAssets.multNumberShader,
        gprAssets.uvOffsetYLoc,
        &player->stats->uvOffsetY,
        SHADER_UNIFORM_FLOAT
    );
    float comboFill = player->stats->comboFillCalc();
    int isBassOrVocal = 0;
    if (player->Instrument == PAD_BASS || player->Instrument == PAD_VOCALS
        || player->Instrument == PLASTIC_BASS) {
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
        &player->stats->overdriveFill,
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
    gprAssets.smasherPressed.materials->shader = gprAssets.HighwayFade;
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
    gprAssets.smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = ColorBrightness(highwayColor, -0.35);
    gprAssets.smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;

    gprAssets.expertHighway.meshes[0].colors = (unsigned char *)ColorToInt(highwayColor);
    gprAssets.emhHighway.meshes[0].colors = (unsigned char *)ColorToInt(highwayColor);
    gprAssets.smasherBoard.meshes[0].colors = (unsigned char *)ColorToInt(ColorBrightness(highwayColor, -0.5));
    gprAssets.smasherBoardEMH.meshes[0].colors =
        (unsigned char *)ColorToInt(highwayColor);

    gprAssets.expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
        player->AccentColor;
    gprAssets.emhHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
        player->AccentColor;

    if (player->Bot)
        player->stats->FC = false;

    // if (player->Bot) player->Bot = true;
    // else player->Bot = false;

    RaiseHighway();
    if (highwayInEndAnim && !songPlaying) {

        if (Restart) {
            gprAudioManager.restartStreams();
            Restart = false;
        }
        else gprAudioManager.BeginPlayback(gprAudioManager.loadedStreams[0].handle);
        songPlaying = true;
        double songEnd = floor(gprAudioManager.GetMusicTimeLength())
                                >= (song.end <= 0 ? 0 : song.end)
                            ? floor(gprAudioManager.GetMusicTimeLength())
                            : song.end - 0.01;
        highwayInEndAnim = false;
        TheSongTime.Start(songEnd);
    }

    if (player->stats->Overdrive) {
        // THIS IS LOGIC!
        player->stats->overdriveFill = player->stats->overdriveActiveFill
            - (float)((time - player->stats->overdriveActiveTime)
                      / (1920 / song.bpms[player->stats->curBPM].bpm));
        if (player->stats->overdriveFill <= 0) {
            player->stats->overdriveActivateTime = time;
            player->stats->Overdrive = false;
            player->stats->overdriveActiveFill = 0;
            player->stats->overdriveActiveTime = 0.0;
            ThePlayerManager.BandStats.PlayersInOverdrive -= 1;
            ThePlayerManager.BandStats.Overdrive = false;
        }
    }

    for (int i = player->stats->curBPM; i < song.bpms.size(); i++) {
        if (time > song.bpms[i].time && i < song.bpms.size() - 1)
            player->stats->curBPM++;
    }

    curChart.overdrive.CheckEvents(player->stats->curODPhrase, time);
    curChart.solos.CheckEvents(player->stats->curSolo, time);
    curChart.fills.CheckEvents(player->stats->curFill, time);
    curChart.sections.CheckEvents(player->stats->curFill, time);

    // BeginShaderMode(gprAssets.HighwayFade);
    if (player->Instrument == PLASTIC_DRUMS) {
        RenderPDrumsHighway(
            player, song, time
        );
    } else if (player->Difficulty == 3
               || (player->ClassicMode && player->Instrument != 4)) {
        RenderExpertHighway(
            player, song, time
        );
               } else {
                   RenderEmhHighway(player, song, time);
               }
    if (player->ClassicMode) {
        if (player->Instrument == PLASTIC_DRUMS) {
            RenderPDrumsNotes(player, curChart, time, highwayLength);
        } else {
            RenderClassicNotes(player, curChart, time, highwayLength);
        }
    } else {
        RenderPadNotes(player, curChart, time, highwayLength);
    }
    // EndShaderMode();
    // if (!player->Bot)
    if (!song.BRE.IsCodaActive(time)){
        RenderHud(player, highwayLength);
    }
}

void gameplayRenderer::DrawHighwayMesh(
    float LengthMultiplier, bool Overdrive, float ActiveTime, float SongTime
) {
    Vector3 HighwayPos { 0, 0, -0.2f };
    Vector3 HighwayScale { 1, 1, 1.5f * LengthMultiplier };
    gprAssets.expertHighway.materials[0].maps->texture.height /= 2.0f;
    gprAssets.odHighwayX.materials[0].maps->texture.height /= 2.0f;
    DrawModelEx(gprAssets.expertHighwaySides, HighwayPos, { 0 }, 0, HighwayScale, WHITE);
    DrawModelEx(gprAssets.expertHighway, HighwayPos, { 0 }, 0, HighwayScale, WHITE);
    unsigned char OverdriveAlpha = 255;
    double OverdriveAnimDuration = 0.25f;
    DrawModel(gprAssets.smasherBoard, Vector3 { 0, 0.0015f, -0.25f }, 1.0f, WHITE);
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

    if (SongTime <= ActiveTime + OverdriveAnimDuration && SongTime > 0.0) {
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
            gprAssets.odHighwayX,
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
void gameplayRenderer::RenderExpertHighway(
    Player *player,
    Song song,
    double time
) {
    StartRenderTexture();
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
    PlayerGameplayStats *stats = player->stats;

    float diffDistance = 2.0f;
    float lineDistance = 1.5f;
    int UseIn = 1;
    int DontIn = 0;
    float highwayLength = (defaultHighwayLength * 1.5f) * player->HighwayLength; //* player->HighwayLength;
    float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));

    DrawHighwayMesh(player->HighwayLength, player->stats->Overdrive, player->stats->overdriveActiveTime, time);
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &UseIn, SHADER_UNIFORM_INT
    );
    BeginShaderMode(gprAssets.HighwayFade);

    if (!player->ClassicMode)
        DrawCylinderEx(
            Vector3 { lineDistance - 1.0f, 0, smasherPos },
            Vector3 { lineDistance - 1.0f, 0, (highwayLength * 1.5f) + smasherPos },
            0.025f,
            0.025f,
            15,
            Color { 128, 128, 128, 128 }
        );

    EndShaderMode();
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT
    );

    DrawRenderTexture();

    StartRenderTexture();
    BeginBlendModeSeparate();


    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &UseIn, SHADER_UNIFORM_INT
    );
    BeginShaderMode(gprAssets.HighwayFade);

    // DrawHitwindow(player, highwayLength);

    if (!song.beatLines.empty()) {
        DrawBeatlines(player, &song, highwayLength, time);
    }

    if (!song.parts[player->Instrument]->charts[player->Difficulty].overdrive.events.empty()) {
        DrawOverdrive(
            player,
            song.parts[player->Instrument]->charts[player->Difficulty],
            highwayLength,
            time
        );
    }
    float darkYPos = 0.015f;
    // BeginBlendMode(BLEND_ALPHA);
    EndShaderMode();
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT
    );
    if (!song.parts[player->Instrument]->charts[player->Difficulty].solos.events.empty()) {
        DrawSolo(
            player,
            song.parts[player->Instrument]->charts[player->Difficulty],
            highwayLength,
            time
        );
    }
    if (song.BRE.exists) {
        DrawCoda(highwayLength, time, player);
    }

    DrawRenderTexture();

    StartRenderTexture();
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);


    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
    rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);

    for (int i = 0; i < 5; i++) {
        Color NoteColor; // = TheGameMenu.hehe && player->Difficulty == 3 ? i == 0 || i == 4 ?
                         // SKYBLUE : i == 1 || i == 3 ? PINK : WHITE : accentColor;
        int noteColor = gprSettings.mirrorMode ? 4 - i : i;
        if (player->ClassicMode) {
            NoteColor = TheGameMenu.hehe ? TRANS[i] : GRYBO[i];
        } else {
            NoteColor = TheGameMenu.hehe ? TRANS[i] : player->AccentColor;
        }
        Model smasherTopModel = gprAssets.smasherInner;
        Color smasherColor = ColorBrightness(ColorContrast(player->AccentColor, -0.25), 0.5);
        gprAssets.smasherInner.materials[0].maps[MATERIAL_MAP_ALBEDO].color = smasherColor;
        gprAssets.smasherInner.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.smasherInnerTex;
        gprAssets.smasherOuter.materials[0].maps[MATERIAL_MAP_ALBEDO].color = smasherColor;

        //if (player->stats->HeldFrets[noteColor]
        //    || player->stats->HeldFretsAlt[noteColor]) {
            DrawModel(
                gprAssets.smasherInner,
                Vector3 { (float)diffDistance-(i), 0.01f, smasherPos },
                1.0f,
                WHITE
            );
        DrawModel(
            gprAssets.smasherOuter,
            Vector3 { (float)diffDistance-(i), 0.01f, smasherPos },
            1.0f,
            WHITE
        );
        smasherTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        if (player->stats->HeldFrets[noteColor] || player->stats->HeldFretsAlt[noteColor]) {
            smasherTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.smasherTopPressedTex;
        } else {
            smasherTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.smasherTopUnpressedTex;
        }

        DrawModel(
                smasherTopModel,
                Vector3 { (float)diffDistance-(i), 0.015f, smasherPos },
                1.0f,
                WHITE
        );

    }

    DrawRenderTexture();
}

void gameplayRenderer::RenderEmhHighway(
    Player *player, Song song, double time
) {
    StartRenderTexture();

    float diffDistance = player->Difficulty == 3 ? 2.0f : 1.5f;
    float lineDistance = player->Difficulty == 3 ? 1.5f : 1.0f;

    float highwayLength = (defaultHighwayLength * 1.5f) * player->HighwayLength;
    float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));

    DrawModel(
        gprAssets.emhHighwaySides,
        Vector3 { 0,
                  0,
                  gprSettings.highwayLengthMult < 1.0f ? -(highwayPosShit * (0.875f))
                                                       : 0 },
        1.0f,
        WHITE
    );
    DrawModel(
        gprAssets.emhHighway,
        Vector3 { 0,
                  0,
                  gprSettings.highwayLengthMult < 1.0f ? -(highwayPosShit * (0.875f))
                                                       : 0 },
        1.0f,
        WHITE
    );
    if (gprSettings.highwayLengthMult > 1.0f) {
        DrawModel(
            gprAssets.emhHighway,
            Vector3 { 0, 0, ((highwayLength * 1.5f) + smasherPos) - 20 },
            1.0f,
            WHITE
        );
        DrawModel(
            gprAssets.emhHighwaySides,
            Vector3 { 0, 0, ((highwayLength * 1.5f) + smasherPos) - 20 },
            1.0f,
            WHITE
        );
        if (highwayLength > 23.0f) {
            DrawModel(
                gprAssets.emhHighway,
                Vector3 { 0, 0, ((highwayLength * 1.5f) + smasherPos) - 40 },
                1.0f,
                WHITE
            );
            DrawModel(
                gprAssets.emhHighwaySides,
                Vector3 { 0, 0, ((highwayLength * 1.5f) + smasherPos) - 40 },
                1.0f,
                WHITE
            );
        }
    }
    if (player->stats->Overdrive) {
        DrawModel(gprAssets.odHighwayEMH, Vector3 { 0, 0.001f, 0 }, 1, WHITE);
    }

    DrawTriangle3D(
        { -diffDistance - 0.5f, 0.02, smasherPos },
        { -diffDistance - 0.5f, 0.02, (highwayLength * 1.5f) + smasherPos },
        { diffDistance + 0.5f, 0.02, smasherPos },
        Color { 0, 0, 0, 64 }
    );
    DrawTriangle3D(
        { diffDistance + 0.5f, 0.02, (highwayLength * 1.5f) + smasherPos },
        { diffDistance + 0.5f, 0.02, smasherPos },
        { -diffDistance - 0.5f, 0.02, (highwayLength * 1.5f) + smasherPos },
        Color { 0, 0, 0, 64 }
    );

    DrawModel(gprAssets.smasherBoardEMH, Vector3 { 0, 0.001f, 0 }, 1.0f, WHITE);

    for (int i = 0; i < 4; i++) {
        Color NoteColor = TheGameMenu.hehe && player->Difficulty == 3 ? i == 0 || i == 4
                ? SKYBLUE
                : i == 1 || i == 3 ? PINK
                                   : WHITE
                                                                  : player->AccentColor;

        gprAssets.smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        //gprAssets.smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;

        if (player->stats->HeldFrets[i] || player->stats->HeldFretsAlt[i]) {
            DrawModel(
                gprAssets.smasherPressed,
                Vector3 { (float)(i), 0.01f, 0 },
                1.0f,
                WHITE
            );
        } else {
           // DrawModel(
           //     gprAssets.smasherReg,
           //     Vector3 { diffDistance - (float)(i), 0.01f, smasherPos },
           //     1.0f,
           //     WHITE
           // );
        }
    }
    for (int i = 0; i < 3; i++) {
        float radius = (i == 1) ? 0.03 : 0.01;
        DrawCylinderEx(
            Vector3 { lineDistance - (float)i, 0, smasherPos + 0.5f },
            Vector3 { lineDistance - (float)i, 0, (highwayLength * 1.5f) + smasherPos },
            radius,
            radius,
            4.0f,
            Color { 128, 128, 128, 128 }
        );
    }
    if (!song.beatLines.empty()) {
        DrawBeatlines(player, &song, highwayLength, time);
    }
    if (!song.parts[player->Instrument]->charts[player->Difficulty].solos.events.empty()) {
        DrawSolo(
            player,
            song.parts[player->Instrument]->charts[player->Difficulty],
            highwayLength,
            time
        );
    }
    if (!song.parts[player->Instrument]->charts[player->Difficulty].overdrive.events.empty()) {
        DrawOverdrive(
            player,
            song.parts[player->Instrument]->charts[player->Difficulty],
            highwayLength,
            time
        );
    }

    EndBlendMode();
    EndMode3D();

    DrawRenderTexture();
}

void gameplayRenderer::DrawBeatlines(
    Player *player, Song *song, float length, double musicTime
) {
    Model beatline = gprAssets.beatline;
    beatline.materials[0].shader = gprAssets.HighwayFade;
    float yPos = 0.0f;
    if (song->beatLines.size() > 0) {
        for (int i = 0; i < song->beatLines.size(); i++) {
            if (song->beatLines[i].Time <= song->end) {

                Color BeatLineColor = { 255, 255, 255, 255 };
                double relTime = GetNoteOnScreenTime(
                    song->beatLines[i].Time,
                    musicTime,
                    player->NoteSpeed,
                    player->Difficulty,
                    length
                );
                if (relTime < -0.5)
                    continue;
                if (i > 0) {
                    double secondLine = GetNoteOnScreenTime(
                        ((song->beatLines[i - 1].Time + song->beatLines[i].Time) / 2),
                        musicTime,
                        player->NoteSpeed,
                        player->Difficulty,
                        length
                    );
                    if (secondLine > 1.5)
                        break;

                    Color BeatLineColor2 = { 255, 255, 255, 255 };
                    Vector3 SecondaryBeatlinePos =
                        Vector3 { 0, yPos, smasherPos - 0.1f + (length * (float)secondLine) };
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
                    /*DrawCylinderEx(Vector3{ -diffDistance - 0.5f,yPos,smasherPos +
                       (length * (float)secondLine) + 0.02f }, Vector3{ diffDistance +
                       0.5f,yPos,smasherPos + (length * (float)secondLine) + 0.02f },
                                   0.01f,
                                   0.01f,
                                   4,
                                   BeatLineColor);*/
                }
                if (relTime > 1.5)
                    break;

                float radius = song->beatLines[i].Major ? 1.5f : 0.95f;

                Color BeatLineColorA = (song->beatLines[i].Major)
                    ? Color { 255, 255, 255, 255 }
                    : Color { 255, 255, 255, 255 };
                beatline.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BeatLineColorA;
                beatline.meshes[0].colors = (unsigned char *)ColorToInt(BeatLineColorA);
                Vector3 BeatlinePos =
                    Vector3 { 0, yPos, smasherPos - 0.1f + (length * (float)relTime) };
                DrawModelEx(
                    beatline, BeatlinePos, { 0 }, 0, { 1, 1, radius }, BeatLineColorA
                );
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
}

void gameplayRenderer::DrawOverdrive(
    Player *player, Chart &curChart, float length, double musicTime
) {
    float DiffMultiplier =
        Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    float start = curChart.overdrive[player->stats->curODPhrase].StartSec;
    float end = curChart.overdrive[player->stats->curODPhrase].EndSec;
    /* for unisons
        eDrawSides((player->NoteSpeed * DiffMultiplier), musicTime, start, end, length,
       0.1, WHITE);
    */
}

void gameplayRenderer::DrawSolo(
    Player *player, Chart &curChart, float length, double musicTime
) {
    float DiffMultiplier =
        Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    float start = curChart.solos[player->stats->curSolo].StartSec;
    float end = curChart.solos[player->stats->curSolo].EndSec;
    nDrawSoloSides(length, start, end, musicTime, player->NoteSpeed, player->Difficulty);
    eDrawSides(
        (player->NoteSpeed * DiffMultiplier), musicTime, start, end, length, 0.09, SKYBLUE
    );

    if (!curChart.solos.events.empty()
        && musicTime >= curChart.solos[player->stats->curSolo].StartSec - 1
        && musicTime <= curChart.solos[player->stats->curSolo].EndSec + 2.5) {
        int solopctnum = Remap(
            curChart.solos[player->stats->curSolo].NotesHit,
            0,
            curChart.solos[player->stats->curSolo].NoteCount,
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
            curChart.solos[player->stats->curSolo].NotesHit,
            curChart.solos[player->stats->curSolo].NoteCount
        );

        // DrawTextEx(gprAssets.josefinSansItalic, soloHit, SoloHitPos,
        // gprU.hinpct(0.04f), 0, accColor);

        float posY = -20;

        float height = -2;
        float pctDist = 1.2;
        float praiseDist = 0;
        float backgroundHeight = height -1.2;
        float soloScale = 1.15;

        float fontSizee = 160;
        float fontSize = 75;

        gprAssets.SoloBox.materials[0].maps[MATERIAL_MAP_ALBEDO].color =
            player->AccentColor;
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
        if (musicTime <= curChart.solos[player->stats->curSolo].EndSec)
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
        if (musicTime >= curChart.solos[player->stats->curSolo].EndSec
            && musicTime <= curChart.solos[player->stats->curSolo].EndSec + 2.5) {
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
    Player *player, Chart &curChart, float length, double musicTime
) {
    float DiffMultiplier =
        Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);

    float start = curChart.fills[player->stats->curFill].StartSec;
    float end = curChart.fills[player->stats->curFill].EndSec;
    eDrawSides(
        (player->NoteSpeed * DiffMultiplier), musicTime, start, end, length, 0.075, GREEN
    );
}

void gameplayRenderer::DrawCoda(float length, double musicTime, Player *player) {
    float DiffMultiplier =
        Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);

    float start = TheSongList.curSong->BRE.StartSec;
    float end = TheSongList.curSong->BRE.EndSec;
    nDrawCodaLanes(length, start, end, musicTime, player->NoteSpeed, player->Difficulty);
    eDrawSides(
        (player->NoteSpeed * DiffMultiplier), musicTime, start, end, length, 0.075, PURPLE
    );
}

void gameplayRenderer::eDrawSides(
    float scrollPos,
    double time,
    double start,
    double end,
    float length,
    double radius,
    Color color
) {
    float soloStart = (float)((start - time)) * scrollPos * (11.5f / length);
    float soloEnd = (float)((end - time)) * scrollPos * (11.5f / length);

    // horrifying.
    // main calc
    bool Beginning =
        (float)(smasherPos + (length * soloStart)) >= (length * 1.5f) + smasherPos;
    bool Ending =
        (float)(smasherPos + (length * soloEnd)) >= (length * 1.5f) + smasherPos;
    float HighwayEnd = (length * 1.5f) + smasherPos;

    // right calc
    float RightSideX = 2.7f;
    float StartPos = Beginning ? HighwayEnd : (float)(smasherPos + (length * soloStart));
    float EndPos = Ending ? HighwayEnd : (float)(smasherPos + (length * soloEnd));
    Vector3 RightSideStart = { RightSideX, -0.0025, StartPos };
    Vector3 RightSideEnd = { RightSideX, -0.0025, EndPos };

    // left calc
    float LeftSideX = -2.7f;

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
    return ((noteTime - songTime))
        * (noteSpeed * HighwaySpeedDifficultyMultiplier(Difficulty)) * (11.5f / length);
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
    Player *player, Chart &curChart, double time, float length
) {
    float diffDistance = 2.0f;
    float DiffMultiplier =
        Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
    StartRenderTexture();
    // glDisable(GL_CULL_FACE);
    PlayerGameplayStats *stats = player->stats;

    for (auto &curNote : curChart.notes) {

        if (!curNote.hit && !curNote.accounted
            && curNote.time + goodBackend + player->InputCalibration < time && !TheSongTime.SongComplete()
            && stats->curNoteInt < curChart.notes.size() && !TheSongTime.SongComplete() && !player->Bot) {
            Encore::EncoreLog(
                LOG_INFO,
                TextFormat("Missed note at %f, note %01i", time, stats->curNoteInt)
            );
            curNote.miss = true;
            FAS = false;
            stats->MissNote();
            stats->Combo = 0;
            curNote.accounted = true;
            stats->curNoteInt++;
        } else if (player->Bot) {
            if (!curNote.hit && !curNote.accounted && curNote.time < time
                && stats->curNoteInt < curChart.notes.size() && !TheSongTime.SongComplete()) {
                curNote.hit = true;
                player->stats->HitDrumsNote(false, !curNote.pDrumTom);
                ThePlayerManager.BandStats.DrumNotePoint(
                    false, player->stats->noODmultiplier(), !curNote.pDrumTom
                );
                if (curNote.len > 0)
                    curNote.held = true;
                curNote.accounted = true;
                // stats->Combo++;
                curNote.accounted = true;
                curNote.hitTime = time;
                stats->curNoteInt++;
                if (player->stats->overdriveFill >= 0.25 && curNote.pDrumAct
                    && !player->stats->Overdrive) {
                    stats->overdriveActiveTime = time;
                    stats->overdriveActiveFill = stats->overdriveFill;
                    stats->Overdrive = true;
                    stats->overdriveHitAvailable = true;
                    stats->overdriveHitTime = time;
                    ThePlayerManager.BandStats.PlayersInOverdrive += 1;
                    ThePlayerManager.BandStats.Overdrive = true;
                }
            }
        }

        if (stats->StrumNoFretTime > time + fretAfterStrumTime && stats->FAS) {
            stats->FAS = false;
        }

        if (player->stats->Overdrive) {
            player->stats->overdriveActiveFill += curChart.overdrive.AddOverdrive(player->stats->curODPhrase);
            if (player->stats->overdriveActiveFill > 1.0f)
                player->stats->overdriveActiveFill = 1.0f;
        } else {
            player->stats->overdriveFill += curChart.overdrive.AddOverdrive(player->stats->curODPhrase);
            if (player->stats->overdriveFill > 1.0f)
                player->stats->overdriveFill = 1.0f;
        }

        curChart.solos.UpdateEventViaNote(curNote, player->stats->curSolo);
        curChart.overdrive.UpdateEventViaNote(curNote, player->stats->curODPhrase);
        curChart.sections.UpdateEventViaNote(curNote, player->stats->curSection);
        curChart.fills.UpdateEventViaNote(curNote, player->stats->curFill);


        double relTime = GetNoteOnScreenTime(
            curNote.time, time, player->NoteSpeed, player->Difficulty, length
        );
        if (relTime < -1)
            continue;

        std::vector<Color> DRUMS = { ORANGE, RED, YELLOW, BLUE, GREEN };
        Color NoteColor = DRUMS[curNote.lane];

        float notePosX = curNote.lane == KICK
            ? 0
            : (diffDistance - (1.25f * (curNote.lane - 1))) - 0.125f;
        float notePosY = 0;
        if (relTime > 1.5) {
            break;
        }

        Vector3 NoteScale = { 1.0f, 1.0f, 1.0f };
        Vector3 NotePos = { notePosX, notePosY, smasherPos + (length * (float)relTime) };
        if (!curNote.pDrumTom && !curNote.pSnare && !curNote.hit
            && curNote.lane != KICK) { // render cymbals
            Color OuterColor = ColorBrightness(NoteColor, -0.15);
            Color InnerColor = RAYWHITE;
            Color BottomColor = DARKGRAY;
            if (curNote.renderAsOD) {
                InnerColor = WHITE;
                OuterColor = RAYWHITE;
                BottomColor = ColorBrightness(GOLD, -0.5);
            } else if (curNote.miss) {
                InnerColor = RED;
                OuterColor = RED;
                BottomColor = RED;
            }
            if (curNote.pDrumAct && player->stats->overdriveFill >= 0.25
                && !player->stats->Overdrive) {
                NoteScale.y = 2.0f;
                OuterColor = GREEN;
                InnerColor = GREEN;
            }
            gprAssets.CymbalInner.materials[0].maps[MATERIAL_MAP_DIFFUSE].color =
                InnerColor;
            gprAssets.CymbalOuter.materials[0].maps[MATERIAL_MAP_DIFFUSE].color =
                OuterColor;
            gprAssets.CymbalBottom.materials[0].maps[MATERIAL_MAP_DIFFUSE].color =
                BottomColor;
            gprAssets.CymbalInner.materials[0].shader = gprAssets.HighwayFade;
            gprAssets.CymbalOuter.materials[0].shader = gprAssets.HighwayFade;
            gprAssets.CymbalBottom.materials[0].shader = gprAssets.HighwayFade;
            DrawModelEx(gprAssets.CymbalInner, NotePos, { 0 }, 0, NoteScale, InnerColor);
            DrawModelEx(gprAssets.CymbalOuter, NotePos, { 0 }, 0, NoteScale, OuterColor);
            DrawModelEx(gprAssets.CymbalBottom, NotePos, { 0 }, 0, NoteScale, BottomColor);
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
            if (curNote.pDrumAct && player->stats->overdriveFill >= 0.25
                && !player->stats->Overdrive) {
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

        nDrawDrumsHitEffects(curNote, time, notePosX);
        NoteMultiplierEffect(time, curNote.time, curNote.miss, player);
    }
    EndMode3D();

    DrawRenderTexture();
}

void gameplayRenderer::RenderPDrumsHighway(
    Player *player,
    Song song,
    double time
) {
    StartRenderTexture();
    BeginBlendModeSeparate();

    PlayerGameplayStats *stats = player->stats;

    float diffDistance = 2.0f;
    float lineDistance = 1.5f;

    float highwayLength = (defaultHighwayLength * 1.5f) * player->HighwayLength;
    float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));

    DrawHighwayMesh(player->HighwayLength, player->stats->Overdrive, player->stats->overdriveActiveTime, time);

    DrawRenderTexture();

    StartRenderTexture();
    BeginBlendModeSeparate();

    int DoIn = 1;
    SetShaderValue(
        gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DoIn, SHADER_UNIFORM_INT
    );
    BeginShaderMode(gprAssets.HighwayFade);
    if (!song.beatLines.empty()) {
        DrawBeatlines(player, &song, highwayLength, time);
    }

    if (!song.parts[player->Instrument]->charts[player->Difficulty].overdrive.events.empty()) {
        DrawOverdrive(
            player,
            song.parts[player->Instrument]->charts[player->Difficulty],
            highwayLength,
            time
        );
    }
    if (!song.parts[player->Instrument]->charts[player->Difficulty].solos.events.empty()) {
        DrawSolo(
            player,
            song.parts[player->Instrument]->charts[player->Difficulty],
            highwayLength,
            time
        );
    }
    if (!song.parts[player->Instrument]->charts[player->Difficulty].fills.events.empty()
        && player->stats->overdriveFill >= 0.25 && !player->stats->Overdrive) {
        DrawFill(
            player,
            song.parts[player->Instrument]->charts[player->Difficulty],
            highwayLength,
            time
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
        //gprAssets.smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        Model smasherTopModel = gprAssets.smasherInner;
        Color smasherColor = ColorBrightness(ColorContrast(player->AccentColor, -0.25), 0.5);
        gprAssets.smasherInner.materials[0].maps[MATERIAL_MAP_ALBEDO].color = smasherColor;
        gprAssets.smasherInner.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.smasherInnerTex;
        gprAssets.smasherOuter.materials[0].maps[MATERIAL_MAP_ALBEDO].color = smasherColor;

        DrawModelEx(
            gprAssets.smasherInner,
            Vector3 { ((float)diffDistance-(float)(i * 1.25)) - 0.125f, 0.01f, smasherPos },
            {0},
            0,
            {1.25,1.0,1.0},
            WHITE
        );
        DrawModelEx(
            gprAssets.smasherOuter,
            Vector3 { ((float)diffDistance-(float)(i * 1.25)) - 0.125f, 0.01f, smasherPos },
            {0},
            0,
            {1.25,1.0,1.0},
            WHITE
        );
        smasherTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        if (player->stats->HeldFrets[i] || player->stats->HeldFretsAlt[i]) {
            smasherTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.smasherTopPressedTex;
        } else {
            smasherTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.smasherTopUnpressedTex;
        }

        DrawModelEx(
                smasherTopModel,
                Vector3 { ((float)diffDistance-(float)(i * 1.25)) - 0.125f, 0.015f, smasherPos },
                {0},
                0,
                {1.25,1.0,1.0},
                WHITE
        );
    }

    DrawRenderTexture();
}

void gameplayRenderer::nDrawDrumsHitEffects(Note note, double time, float notePosX) {
    // oh my good fucking lord
    double PerfectHitAnimDuration = 1.0f;
    if (note.hit && time < note.hitTime + HitAnimDuration) {
        double TimeSinceHit = time - note.hitTime;
        unsigned char HitAlpha = Remap(
            getEasingFunction(EaseInBack)(TimeSinceHit / HitAnimDuration), 0, 1.0, 196, 0
        );

        float Width = note.lane == KICK ? 5.0f : 1.3f;
        float Height = note.lane == KICK ? 0.125f : 0.25f;
        float Length = note.lane == KICK ? 0.5f : 0.75f;
        float yPos = note.lane == KICK ? 0 : 0.125f;
        Color BoxColor = note.perfect ? Color { 255, 215, 0, HitAlpha }
                                      : Color { 255, 255, 255, HitAlpha };
        DrawCube(Vector3 { notePosX, yPos, smasherPos }, Width, Height, Length, BoxColor);
    }
    EndBlendMode();
    float KickBounceDuration = 0.75f;
    if (note.hit && time < note.hitTime + PerfectHitAnimDuration && note.perfect) {
        double TimeSinceHit = time - note.hitTime;
        unsigned char HitAlpha = Remap(
            getEasingFunction(EaseOutQuad)(TimeSinceHit / PerfectHitAnimDuration),
            0,
            1.0,
            255,
            0
        );
        float HitPosLeft = Remap(
            getEasingFunction(EaseInOutBack)(TimeSinceHit / PerfectHitAnimDuration),
            0,
            1.0,
            3.4,
            3.0
        );

        Color InnerBoxColor = { 255, 161, 0, HitAlpha };
        Color OuterBoxColor = { 255, 161, 0, (unsigned char)(HitAlpha / 2) };
        float Width = 1.0f;
        float Height = 0.01f;
        DrawCube(
            Vector3 { HitPosLeft, -0.1f, smasherPos }, Width, Height, 0.5f, InnerBoxColor
        );
        DrawCube(
            Vector3 { HitPosLeft, -0.11f, smasherPos }, Width, Height, 1.0f, OuterBoxColor
        );
    }
    if (note.hit && note.lane == KICK && time < note.hitTime + KickBounceDuration) {
        double TimeSinceHit = time - note.hitTime;
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

void gameplayRenderer::nDrawFiveLaneHitEffects(Note note, double time, float notePosX) {
    double PerfectHitAnimDuration = 1.0f;
    if (note.hit && time < note.hitTime + HitAnimDuration) {
        double TimeSinceHit = time - note.hitTime;
        unsigned char HitAlpha = Remap(
            getEasingFunction(EaseInBack)(TimeSinceHit / HitAnimDuration), 0, 1.0, 196, 0
        );
        Color PerfectColor = Color { 255, 215, 0, HitAlpha };
        Color GoodColor = Color { 255, 255, 255, HitAlpha };
        Color BoxColor = note.perfect ? PerfectColor : GoodColor;

        DrawCube(Vector3 { notePosX, 0.125, smasherPos }, 1.0f, 0.25f, 0.5f, BoxColor);
    }

    if (note.hit && time < note.hitTime + PerfectHitAnimDuration && note.perfect) {
        double TimeSinceHit = time - note.hitTime;
        unsigned char HitAlpha = Remap(
            getEasingFunction(EaseOutQuad)(TimeSinceHit / PerfectHitAnimDuration),
            0,
            1.0,
            255,
            0
        );
        float HitPosLeft = Remap(
            getEasingFunction(EaseInOutBack)(TimeSinceHit / PerfectHitAnimDuration),
            0,
            1.0,
            3.4,
            3.0
        );

        DrawCube(
            Vector3 { HitPosLeft, -0.1f, smasherPos },
            1.0f,
            0.01f,
            0.5f,
            Color { 255, 161, 0, HitAlpha }
        );
        DrawCube(
            Vector3 { HitPosLeft, -0.11f, smasherPos },
            1.0f,
            0.01f,
            1.0f,
            Color { 255, 161, 0, (unsigned char)(HitAlpha / 2) }
        );
    }
}

void gameplayRenderer::nDrawPlasticNote(
    Note note, Color noteColor, float notePosX, float noteTime
) {
    // why the fuck did i separate sustains and non-sustain note drawing?????

    Vector3 NotePos = { notePosX, 0, noteTime };
    Color InnerColor = noteColor;
    Color BaseColor = WHITE;
    Color SideColor = WHITE;

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

            Vector3 scale = {1.0f,1.0f,0.5f};
            DrawModelEx(HopoParts[mBASE], NotePos, {0}, 0, scale, BaseColor);
            DrawModelEx(HopoParts[mCOLOR], NotePos, {0}, 0, scale, InnerColor);
            DrawModelEx(HopoParts[mSIDES], NotePos, {0}, 0, scale, SideColor);
        }
        else if (note.pTap) {
            TapParts[mBASE].materials[0].shader = gprAssets.HighwayFade;
            TapParts[mCOLOR].materials[0].shader = gprAssets.HighwayFade;
            TapParts[mSIDES].materials[0].shader = gprAssets.HighwayFade;
            TapParts[mINSIDE].materials[0].shader = gprAssets.HighwayFade;

            Vector3 scale = {1.0f,1.0f,0.5f};
            DrawModelEx(TapParts[mBASE], NotePos, {0}, 0, scale, BaseColor);
            DrawModelEx(TapParts[mCOLOR], NotePos, {0}, 0, scale, InnerColor);
            DrawModelEx(TapParts[mSIDES], NotePos, {0}, 0, scale, SideColor);
            DrawModelEx(TapParts[mINSIDE], NotePos, {0}, 0, scale, BLACK);
        }
        else if (note.pOpen) {
            OpenParts[mBASE].materials[0].shader = gprAssets.HighwayFade;
            OpenParts[mCOLOR].materials[0].shader = gprAssets.HighwayFade;
            OpenParts[mSIDES].materials[0].shader = gprAssets.HighwayFade;
            if (note.phopo) {
                InnerColor = WHITE;
                SideColor = PURPLE;
                BaseColor = PURPLE;
            }
            Vector3 scale = {1.0f,1.0f,0.5f};
            NotePos.x = 0;
            DrawModelEx(OpenParts[mBASE], NotePos, {0}, 0, scale, BaseColor);
            DrawModelEx(OpenParts[mCOLOR], NotePos, {0}, 0, scale, InnerColor);
            DrawModelEx(OpenParts[mSIDES], NotePos, {0}, 0, scale, SideColor);
        }
        else {

            StrumParts[mBASE].materials[0].shader = gprAssets.HighwayFade;
            StrumParts[mCOLOR].materials[0].shader = gprAssets.HighwayFade;
            StrumParts[mSIDES].materials[0].shader = gprAssets.HighwayFade;

            Vector3 scale = {1.0f,1.0f,0.5f};
            DrawModelEx(StrumParts[mBASE], NotePos, {0}, 0, scale, BaseColor);
            DrawModelEx(StrumParts[mCOLOR], NotePos, {0}, 0, scale, InnerColor);
            DrawModelEx(StrumParts[mSIDES], NotePos, {0}, 0, scale, SideColor);
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
        Vector3 scale = {1.0f,1.0f,0.5f};
        DrawModelEx(StrumParts[mBASE], NotePos, {0}, 0, scale, BottomColor);
        DrawModelEx(StrumParts[mCOLOR], NotePos, {0}, 0, scale, InnerColor);
        DrawModelEx(StrumParts[mSIDES], NotePos, {0}, 0, scale, SidesColor);
    }
}

void gameplayRenderer::nDrawSustain(
    Note note, Color noteColor, float notePosX, Matrix sustainMatrix
) {
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

    if (note.held)
        DrawCube(Vector3 { notePosX, 0.1, smasherPos }, 0.4f, 0.2f, 0.4f, noteColor);

    EndBlendMode();
}

void gameplayRenderer::nDrawCodaLanes(
    float length, double sTime, double cLen, double curTime, float NoteSpeed, int Difficulty
) {
    for (int i = 0; i < 5; i++) {
        double relTime = GetNoteOnScreenTime(
                sTime, curTime, NoteSpeed, Difficulty, length
            );
        double relEnd = GetNoteOnScreenTime(
            cLen,
            curTime,
            NoteSpeed,
            Difficulty,
            length
        );

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
        if (TheGameMenu.hehe) {
            noteColor = TRANS[i];
        }
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

void gameplayRenderer::nDrawSoloSides(
    float length, double sTime, double cLen, double curTime, float NoteSpeed, int Difficulty
) {
    for (int i = 0; i < 5; i++) {
        if (i == 1 || i == 2 || i == 3 )
            continue;

        double relTime = GetNoteOnScreenTime(
                sTime, curTime, NoteSpeed, Difficulty, length
            );
        double relEnd = GetNoteOnScreenTime(
            cLen,
            curTime,
            NoteSpeed,
            Difficulty,
            length
        );

        float sustainLen = (length * (float)relEnd) - (length * (float)relTime);
        float notePosX = 2.0f - (1.0f * i);
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
