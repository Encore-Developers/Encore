//
// Created by maria on 14/01/2026.
//

#include "Track.h"

#include "GemTrackSlot.h"
#include "assets.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "gameplay/enctime.h"

void Encore::Track::Draw() {
    NoteSpeed = player.NoteSpeed; // TODO: should probably find a better way to do this

    BeginTextureMode(GameplayRenderTexture);
    BeginMode3D(camera);
    ClearBackground({0,0,0,0});

    //SetShaderValue(ASSET(trackCurveShader), ASSET(trackCurveShader).GetUniformLoc("trackLength"), &Length, SHADER_UNIFORM_FLOAT);
    //SetShaderValue(ASSET(trackCurveShader), ASSET(trackCurveShader).GetUniformLoc("fadeSize"), &FadeSize, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(ASSET(trackCurveShader));
    rlDisableDepthTest();

    DrawBeatlines();
    DrawSmashers();
    DrawNotes();

    EndShaderMode();

    EndMode3D();
    EndTextureMode();

    int height = (float)GetRenderHeight();
    int width = (float)GetRenderWidth();
    GameplayRenderTexture.texture.width = width;
    GameplayRenderTexture.texture.height = height;
    Rectangle source = { 0, 0, float(width), float(-height) };
    Rectangle res = { 0, 0, float(GetRenderWidth()), float(GetRenderHeight()) };
    Vector2 shaderResolution = { float(GetRenderWidth()), float(GetRenderHeight()) };

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

void Encore::Track::Load() {
    camera = {
        {0, 6.0f, -12.0f},
        { 0.0f, 0.0f, 10.0f },
        { 0.0f, 1.0f, 0.0f },
        40.0f,
    };
    GameplayRenderTexture = LoadRenderTexture(GetRenderWidth(), GetRenderHeight());
    SetTextureFilter(GameplayRenderTexture.texture, TEXTURE_FILTER_BILINEAR);
}
void Encore::Track::DrawNotes() {

    if (player.engine->chart->at(0).empty()) {
        return;
    }

    for (int lane = 0 ; lane < player.engine->chart->Lanes.size(); lane++) {
        std::pair<int, int> NotePoolSize = player.engine->GetNotePoolSize(lane);
        for (int curNote = NotePoolSize.first; curNote < NotePoolSize.second; curNote++) {
            auto *note = &player.engine->chart->at(lane).at(curNote);
            auto slots = GetSlotsForLane(player.engine->UsesNoteMasks() ? note->Lane : lane);
            for (int i = 0; i < 7; i++) {
                if (slots[i]) {
                    auto slot = slots[i];
                    slot->DrawNote(note);
                }
            }
        }
    }

}
void Encore::Track::DrawSmashers() {
    for (int i = 0; i < slots.size(); i++) {
        auto slot = slots.at(i).get();
        slot->DrawSmasher(false);
    }
}

void Encore::Track::DrawBeatlines()
    {
        if (!TheSongTime.Beatlines.empty()) {
            if (TheSongTime.Beatlines.front().time < TheSongTime.GetElapsedTime() - 1) {
                TheSongTime.Beatlines.erase(TheSongTime.Beatlines.begin());
            }

            int beatlinePoolMaxSize = NOTE_POOL_SIZE / 4;
            size_t BeatlinePoolSize = TheSongTime.Beatlines.size() > beatlinePoolMaxSize
                ? beatlinePoolMaxSize
                : TheSongTime.Beatlines.size();
            // because i have to do bounds checks myself
            BeatlinePool = { TheSongTime.Beatlines.begin(), BeatlinePoolSize };

            for (auto &beatline : BeatlinePool) {
                float ScrollPos = GetNotePos3D(
                    beatline.time
                );
                float Size = 0;
                Color beatlineColor = WHITE;
                switch (beatline.type) {
                case Major: {
                    beatlineColor = GRAY;
                    Size = 0.05;
                    break;
                }
                case Minor: {
                    beatlineColor = DARKGRAY;
                    Size = 0.01;
                    break;
                }
                case Measure: {
                    beatlineColor = WHITE;
                    Size = 0.1;
                    break;
                }
                }
                // this sucks
                static std::vector<Vector3> verts;
                verts.resize(0);
                int wideVerts = 10;
                for (int i = 0; i <= wideVerts; i++) {
                    float xPos = Remap(i, 0, wideVerts, -2.5, 2.5);
                    verts.push_back({xPos, 0, ScrollPos-Size});
                    verts.push_back({xPos, 0, ScrollPos+Size});
                }
                DrawTriangleStrip3D(verts.data(), verts.size(), beatlineColor);
                continue;
                DrawCube(
                    { 0, 0, ScrollPos },
                    5,
                    Size,
                    Size,
                    beatlineColor
                );
            }
        }
    };

Encore::TrackSlot **Encore::Track::GetSlotsForLane(uint8_t lane) const {
    static TrackSlot *slotBuffer[7];
    int curIndex = 0;
    auto append_slot = [&](int index) {
        slotBuffer[curIndex] = slots[index].get();
        curIndex++;
    };
    if (player.engine->UsesNoteMasks()) {

        for (int i = 0; i < 5; i++) {
            if (lane & RhythmEngine::PlasticFrets[i]) {
                append_slot(i);
            }
        }
        if (lane == 0) {
            append_slot(5);
        }
        slotBuffer[curIndex] = nullptr;
    } else {
        slotBuffer[0] = slots[lane].get();
        slotBuffer[1] = nullptr;
    }
    return (TrackSlot **)&slotBuffer;
}

void Encore::Track::AddSlot(TrackSlot *slot) {
    slot->index = slots.size();
    slots.emplace_back(slot);
}
void Encore::Track::Configure5Lane() {
    slots.clear();
    AddSlot(new GemTrackSlot(this, 2, 1, SLOT_GREEN));
    AddSlot(new GemTrackSlot(this, 1, 1, SLOT_RED));
    AddSlot(new GemTrackSlot(this, 0, 1, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -1, 1, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -2, 1, SLOT_ORANGE));
    AddSlot(new GemTrackSlot(this, 0, 5, SLOT_OPEN));
}
void Encore::Track::Configure4Lane() {
    slots.clear();
    AddSlot(new GemTrackSlot(this, 2, 1.25, SLOT_GREEN));
    AddSlot(new GemTrackSlot(this, 0.75, 1.25, SLOT_RED));
    AddSlot(new GemTrackSlot(this, -0.75, 1.25, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -2, 1.25, SLOT_BLUE));
}
void Encore::Track::ConfigureDrums() {
    slots.clear();
    AddSlot(new GemTrackSlot(this, 0, 5, SLOT_KICK)); // TODO: make the kick slot a different type
    AddSlot(new GemTrackSlot(this, 2, 1.25, SLOT_RED));
    AddSlot(new GemTrackSlot(this, 0.625, 1.25, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -0.625, 1.25, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -2, 1.25, SLOT_GREEN));

}

float Encore::Track::GetNotePos3D(double noteTime) {
    return ((noteTime - TheSongTime.GetElapsedTime()) * (NoteSpeed * Length));
}

Encore::Track::~Track() {
    UnloadRenderTexture(GameplayRenderTexture);
}
