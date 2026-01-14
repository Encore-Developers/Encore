//
// Created by maria on 14/01/2026.
//

#include "BaseTrack.h"
#include "raylib.h"
void Encore::BaseTrack::Draw() {
    BeginTextureMode(GameplayRenderTexture);
    BeginMode3D(camera);
    ClearBackground({0,0,0,0});

    DrawBeatlines();
    DrawNotes();
    DrawStrikeline();

    EndMode3D();
    EndTextureMode();

    int height = (float)GetScreenHeight();
    int width = (float)GetScreenWidth();
    GameplayRenderTexture.texture.width = width;
    GameplayRenderTexture.texture.height = height;
    Rectangle source = { 0, 0, float(width), float(-height) };
    Rectangle res = { 0, 0, float(GetScreenWidth()), float(GetScreenHeight()) };
    Vector2 shaderResolution = { float(GetScreenWidth()), float(GetScreenHeight()) };

    // BeginShaderMode(gprAssets.fxaa);
    // DrawTextureRec(GameplayRenderTexture.texture, res, {0}, WHITE);

    // SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, GameplayRenderTexture.texture);
    //SetShaderValue(
    //    gprAssets.fxaa, gprAssets.resLoc, &shaderResolution, SHADER_UNIFORM_VEC2
    //);
    DrawTexturePro(
        GameplayRenderTexture.texture, source, res, { 0, 0 }, 0, WHITE
    );
}

void Encore::BaseTrack::Load() {
    camera = {
        {0, 7.0f, -12.0f},
        { 0.0f, 0.0f, 10.0f },
        { 0.0f, 1.0f, 0.0f },
        40.0f,
    };
    GameplayRenderTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
}
