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
#include "imgui.h"
#include "KickTrackSlot.h"
#include "OpenTrackSlot.h"
#include "debug/EncoreDebug.h"
#include "events/Event.h"
#include "song/song.h"

void Encore::Track::Draw() {

    if (ColumnFitting) {
        FitToColumn(ColumnLeft, ColumnRight);
    }

    NoteSpeed = player.NoteSpeed; // TODO: should probably find a better way to do this
    Length = BaseLength * player.HighwayLength;
    player.engine->UpdateCalibration(player.InputCalibration);

    ProcessAnimation();
    BeginMode3D(AnimCamera);

    for (auto shader : { ASSETPTR(trackCurveShader), ASSETPTR(noteShader),
                         ASSETPTR(highwayScrollShader), ASSETPTR(overdriveShader), ASSETPTR(multiplierFillShader), ASSETPTR(indicatorRingShader) }) {
        shader->SetUniform("trackLength", Length);
        shader->SetUniform("fadeSize", FadeSize);
        shader->SetUniform("curveFac", CurveFac);
        shader->SetUniform("offset", Offset);
        shader->SetUniform("scale", Scale);
    }
    ASSET(multiplierFillShader).SetUniform("curveFac", 10000000000.0f);
    ASSET(indicatorRingShader).SetUniform("curveFac", 10000000000.0f);

    ASSET(indicatorRingShader).SetUniform("tex1", ASSET(fcindtex1));
    ASSET(indicatorRingShader).SetUniform("tex2", ASSET(fcindtex2));
    ASSET(indicatorRingShader).SetUniform("baseTex", ASSET(fcindtex3));

    BeginShaderMode(ASSET(trackCurveShader));
    rlDisableDepthTest();

    DrawSurface();

    DrawBeatlines();
    DrawOverdriveMeter();
    DrawMultiplier();
    DrawSmashers();

    EndMode3D();
    BeginMode3D(AnimCamera);
    rlDisableDepthTest();
    DrawNotes();

    particleSystem->Render();
    EndShaderMode();

    EndMode3D();

    if (EncoreDebug::showDebug) {
        DrawTrackDebugWindow();
    }
}


void Encore::Track::Load() {
    BaseCamera = {
        { 0, 8.0f, -14.0f },
        { 0.0f, 0.0f, 15.0f },
        { 0.0f, 1.0f, 0.0f },
        40.0f,
    };
    AnimCamera = BaseCamera;
    player.engine->AddSink(this);
    particleSystem = std::unique_ptr<ParticleSystem>(new ParticleSystem);
    particleSystem->track = this;
}

float easeInOutQuad(float x) {
    return x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
}



void Encore::Track::DrawSurface() {
    float time = GetNotePos3D(0) / 22.5;
    SetShaderValue(ASSET(highwayScrollShader),
                   ASSET(highwayScrollShader).GetUniformLoc("time"),
                   &time,
                   SHADER_UNIFORM_FLOAT);

    ASSET(trackSurface).Fetch().materials[0].maps[0].texture = ASSET(highwayTexture);
    DrawModelEx(ASSET(trackSurface),
                { 0 },
                { 0 },
                0,
                { 1, 1, 1 },
                ColorBrightness(player.AccentColor, -0.25));

    OverdriveTimer = Lerp(OverdriveTimer, (int)player.engine->stats->overdrive.Active, GetFrameTime() * 8);
    if (OverdriveTimer > 0.01) {
        ASSET(trackSurface).Fetch().materials[0].maps[0].texture = ASSET(overdriveTex);
        DrawModelEx(ASSET(trackSurface),
                    { 0 },
                    { 0 },
                    0,
                    { 1, 1, 1 },
                    ColorAlpha(GOLD, easeInOutQuad(OverdriveTimer)));
    }

    SpotlightTimer = Lerp(SpotlightTimer, (int)(player.engine->stats->multNoOD() >= 4) , GetFrameTime() * 3);
    if (SpotlightTimer > 0.01){
        ASSET(trackSurface).Fetch().materials[0].maps[0].texture = ASSET(spotlightTex);
        DrawModelEx(ASSET(trackSurface),
                    { 0 },
                    { 0 },
                    0,
                    { 1, 1, 1 },
                    ColorAlpha(ColorBrightness(player.AccentColor, -0.25), easeInOutQuad(SpotlightTimer)));
    }

    DrawModelEx(ASSET(rails), { 0 }, { 0 }, 0, { 1, 1, 1 }, WHITE);
    // static std::vector<Vector3> points;
    // points.clear();
    // for (int i = 0; i <= 10; i++) {
    //    auto x = Remap(i, 0, 10, -2.5, 2.5);
    //    points.push_back({x, 0, -15});
    //    points.push_back({x, 0, Length});
    // }
    // DrawTriangleStrip3D(points.data(), points.size(), {0, 0, 0, 160});
}

void Encore::Track::DrawOverdriveMeter() {


    int denom = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).denom;
    int numer = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).numer;
    int flashInterval = (numer * 480) / denom;

    unsigned char streakFlash =
        BeatToCharViaTickThing(TheSongTime.GetCurrentTick(), 0, 255, flashInterval);
    float Percentage = float(streakFlash) / 255.0f;
    Color OverdriveBarColor = ColorBrightness(GOLD, Percentage);

    // DrawTriangleStrip3D(points.data(), points.size(), OverdriveBarColor);
    ASSET(overdriveShader).SetUniform("FillColor", OverdriveBarColor);
    ASSET(overdriveShader).SetUniform("FillPct", 1.0f-player.engine->stats->overdrive.Fill);
    ASSET(overdriveShader).SetUniform("BaseColor", ColorBrightness(player.AccentColor, 0.75));
    DrawModelEx(ASSET(overdriveMeter), { 0, -0.1, -0.85 }, { 1, 0, 0 }, 60, { 0.95, 0.8, 0.95 }, player.AccentColor);

    return;

    BeginShaderMode(ASSET(sdfShader));
    rlPushMatrix();
    rlTranslatef(0, 0.5, -0);
    rlRotatef(-90, -1, 0, 0);
    rlRotatef(180, 0, 0, 1);

    std::string multString = TextFormat("%ix", player.engine->stats->multiplier());
    Vector2 size = MeasureTextEx(ASSET(JetBrainsMono), multString.c_str(), 0.8, 0);
    DrawCircleSector({ 0, 0 }, 0.5, 0, 360, 32, BLACK);
    DrawTextEx(ASSET(JetBrainsMono),
               multString.c_str(),
               { -size.x / 2, -size.y / 2 },
               0.8,
               0,
               WHITE);
    rlPopMatrix();
    BeginShaderMode(ASSET(trackCurveShader));
}

void Encore::Track::DrawMultiplier() {
    Vector3 position = {0,-0.1, -1.25};
    Vector3 scale = {1.1, 1.1, 1.1};
    ASSET(indicatorRingShader).SetUniform("BaseColor", ColorBrightness(player.AccentColor, -0.3));
    ASSET(indicatorRingShader).SetUniform("FCColor", GOLD);
    ASSET(indicatorRingShader).SetUniform("time", GetTime());
    ASSET(multiplierFillShader).SetUniform("BaseColor", ColorBrightness(player.AccentColor, -0.85));
    int MaxMult = player.engine->stats->SixMultiplier ? 6 : 4;
    if (player.engine->stats->multNoOD() == MaxMult) {
        ASSET(multiplierFillShader).SetUniform("MultiplierColor", SKYBLUE);
    } else {
        ASSET(multiplierFillShader).SetUniform("MultiplierColor", RAYWHITE);
    }
    ASSET(multiplierFillShader).SetUniform("FillPercentage", player.engine->stats->ComboFillCalc());

    if (player.engine->stats->Misses == 0 && player.engine->stats->Overhits == 0) {
        ASSET(indicatorRingShader).SetUniform("isFC", 1.0f);
    } else {
        ASSET(indicatorRingShader).SetUniform("isFC", 0.0f);
    }


    DrawModelEx(ASSET(multiplierFill), position, { 0 }, 0, scale, WHITE);
    DrawModelEx(ASSET(indicatorRing), position, { 0 }, 0, scale, WHITE);
    DrawModelEx(ASSET(multiplierFrame), position, { 0 }, 0, scale, ColorBrightness(player.AccentColor, -0.7));

}

unsigned char Encore::Track::BeatToCharViaTickThing(
    int tick,
    int MinBrightness,
    int MaxBrightness,
    int QuarterNoteLength
) {
    float TickModulo = tick % QuarterNoteLength;
    return Remap(
        TickModulo / float(QuarterNoteLength),
        0,
        1.0f,
        MaxBrightness,
        MinBrightness
    );
}

void Encore::Track::DrawNotes() {
    if (player.engine->chart->at(0).empty()) {
        return;
    }

    for (int lane = 0; lane < player.engine->chart->Lanes.size(); lane++) {
        std::pair<int, int> NotePoolSize = player.engine->GetNotePoolSize(lane);
        for (int curNote = NotePoolSize.second - 1; curNote >= NotePoolSize.first; curNote
             --) {
            auto *note = &player.engine->chart->at(lane).at(curNote);
            if (note->StartSeconds > GetViewEndTime()) {
                continue;
            }
            if (player.engine->practice) {
                if (note->StartSeconds >= player.engine->pStopTime) {
                    continue;
                }
                if (note->StartSeconds < player.engine->pStartTime) {
                    continue;
                }
            }
            auto slots = GetSlotsForLane(player.engine->UsesNoteMasks()
                ? note->Lane
                : lane);
            for (int i = 0; i < 7; i++) {
                if (slots[i]) {
                    auto slot = slots[i];
                    slot->DrawNote(note);
                } else
                    break;
            }
        }
    }
}

void Encore::Track::DrawSmashers() {
    rlEnableDepthTest();
    glClear(GL_DEPTH_BUFFER_BIT);
    for (int i = 0; i < slots.size(); i++) {
        auto slot = slots.at(i).get();
        slot->DrawSmasher(
            slot->index < player.engine->stats->HeldFrets.size() && player.engine->stats->
            HeldFrets[slot->index]);
    }
    rlDisableDepthTest();
}

void Encore::Track::DrawBeatlines() {
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
                beatlineColor = { 255, 255, 255, 220 };
                Size = 0.05;
                break;
            }
            case Minor: {
                beatlineColor = { 255, 255, 255, 190 };
                Size = 0.01;
                break;
            }
            case Measure: {
                beatlineColor = { 255, 255, 255, 240 };
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
                verts.push_back({ xPos, 0, ScrollPos - Size });
                verts.push_back({ xPos, 0, ScrollPos + Size });
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
    rlDrawRenderBatchActive();
};

Encore::TrackSlot **Encore::Track::GetSlotsForLane(uint8_t lane, bool forceMask) const {
    static TrackSlot *slotBuffer[7];
    int curIndex = 0;
    auto append_slot = [&](int index) {
        slotBuffer[curIndex] = slots[index].get();
        curIndex++;
    };
    if (player.engine->UsesNoteMasks() || forceMask) {
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


void Encore::Track::HandleEvent(Event *event) {
    if (auto hitEvent = event->GetTyped<NoteHitEvent>()) {
        auto *note = hitEvent->note;
        auto slots = GetSlotsForLane(note->Lane, true);
        if (note->Lane == RhythmEngine::PlasticFrets[0] && player.Instrument ==
            PlasticDrums) {
            KickTimer = 1;
            }
        for (int i = 0; i < 7; i++) {
            if (slots[i]) {
                auto slot = slots[i];
                slot->AnimateHit(hitEvent->perfect);
            } else
                break;
        }
    }
}


inline double easeInQuadd(double x) {
    return x * x;
}

void Encore::Track::ProcessAnimation() {
    AnimCamera = BaseCamera;
    if (KickTimer > 0)
        KickTimer -= GetFrameTime() * 5;
    else {
        KickTimer = 0;
    }

    AnimCamera.position.y = BaseCamera.position.y - (easeInQuadd(KickTimer) * 0.2);
}

void Encore::Track::AddSlot(TrackSlot *slot) {
    slot->index = slots.size();
    slots.emplace_back(slot);
}

void Encore::Track::Configure5Lane() {
    slots.clear();
    if (player.LeftyFlip) {
        AddSlot(new GemTrackSlot(this, -2, 1, SLOT_GREEN));
        AddSlot(new GemTrackSlot(this, -1, 1, SLOT_RED));
        AddSlot(new GemTrackSlot(this, 0, 1, SLOT_YELLOW));
        AddSlot(new GemTrackSlot(this, 1, 1, SLOT_BLUE));
        AddSlot(new GemTrackSlot(this, 2, 1, SLOT_ORANGE));
    } else {
        AddSlot(new GemTrackSlot(this, 2, 1, SLOT_GREEN));
        AddSlot(new GemTrackSlot(this, 1, 1, SLOT_RED));
        AddSlot(new GemTrackSlot(this, 0, 1, SLOT_YELLOW));
        AddSlot(new GemTrackSlot(this, -1, 1, SLOT_BLUE));
        AddSlot(new GemTrackSlot(this, -2, 1, SLOT_ORANGE));
    }
    AddSlot(new OpenTrackSlot(this, 0, 1, SLOT_OPEN));
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
    AddSlot(new KickTrackSlot(this, 0, 5, SLOT_KICK));
    AddSlot(new GemTrackSlot(this, 1.875, 1.25, 0.75, SLOT_RED));
    AddSlot(new GemTrackSlot(this, 0.625, 1.25, 0.75, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -0.625, 1.25, 0.75, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -1.875, 1.25, 0.75, SLOT_GREEN));
}

float Encore::Track::GetNotePos3D(double noteTime) {
    return (noteTime - TheSongTime.GetElapsedTime()) * GetZPerSecond();
}

float Encore::Track::GetViewEndTime() const {
    return TheSongTime.GetElapsedTime() + (Length / GetZPerSecond());
}

float Encore::Track::GetZPerSecond() const {
    return NoteSpeed * BaseLength;
}
void Encore::Track::FitToColumn(float left, float right) {
    float currentLeft = GetWorldToScreenEx({2.5, 0, 0}, BaseCamera, GetRenderWidth(), GetRenderHeight()).x;
    float currentRight = GetWorldToScreenEx({-2.5, 0, 0}, BaseCamera, GetRenderWidth(), GetRenderHeight()).x;
    currentLeft = Remap(currentLeft, 0, GetRenderWidth(), -1.0f, 1.0f);
    currentRight = Remap(currentRight, 0, GetRenderWidth(), -1.0f, 1.0f);
    float currentMidPos = (currentLeft + currentRight) / 2;
    float midPos = (left + right) / 2;
    float currentWidth = currentRight - currentLeft;
    float targetWidth = right - left;
    float scale = Clamp(targetWidth / currentWidth, 0.3, 1.0);
    Offset = midPos - (currentMidPos);
    Scale = scale;
}

Encore::Track::~Track() {
}
