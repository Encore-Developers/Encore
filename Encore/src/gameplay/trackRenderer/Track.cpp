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
    NoteSpeed = player.NoteSpeed; // TODO: should probably find a better way to do this
    Length = BaseLength * player.HighwayLength;

    ProcessAnimation();
    BeginMode3D(AnimCamera);

    for (auto shader : {ASSETPTR(trackCurveShader), ASSETPTR(noteShader), ASSETPTR(highwayScrollShader)}) {
        shader->SetUniform("trackLength", Length);
        shader->SetUniform("fadeSize", FadeSize);
        shader->SetUniform("curveFac", CurveFac);
        shader->SetUniform("offset", Offset);
        shader->SetUniform("scale", Scale);
    }

    BeginShaderMode(ASSET(trackCurveShader));
    rlDisableDepthTest();

    DrawSurface();

    DrawBeatlines();
    DrawOverdriveMeter();
    DrawSmashers();

    EndMode3D();
    BeginMode3D(AnimCamera);
    rlDisableDepthTest();
    DrawNotes();

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
}

void Encore::Track::DrawSurface() {
    float time = GetNotePos3D(0) / 22.5;
    SetShaderValue(ASSET(highwayScrollShader),
                   ASSET(highwayScrollShader).GetUniformLoc("time"),
                   &time,
                   SHADER_UNIFORM_FLOAT);

    DrawModelEx(ASSET(trackSurface),
                { 0 },
                { 0 },
                0,
                { 1, 1, 1 },
                ColorBrightness(player.AccentColor, -0.25));
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
    static std::vector<Vector3> points;
    points.clear();

    for (int i = 0; i <= 10; i++) {
        auto x = Remap(i, 0, 10, -2.5, 2.5);
        points.push_back({ x, 0, -1.0 });
        points.push_back({ x, 0, -0.5 });
    }

    DrawTriangleStrip3D(points.data(), points.size(), BLACK);

    points.clear();
    for (int i = 0; i <= 10; i++) {
        auto x = Remap(i, 0, 10, -2.5, 2.5);
        x = Lerp(x, 2.5, 1 - player.engine->stats->overdrive.Fill);
        points.push_back({ x, 0, -1.0 });
        points.push_back({ x, 0, -0.5 });
    }

    int denom = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).denom;
    int numer = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).numer;
    int flashInterval = (numer * 480) / denom;

    unsigned char streakFlash =
        BeatToCharViaTickThing(TheSongTime.GetCurrentTick(), 0, 255, flashInterval);
    float Percentage = float(streakFlash) / 255.0f;
    Color OverdriveBarColor = ColorBrightness(GOLD, Percentage);

    DrawTriangleStrip3D(points.data(), points.size(), OverdriveBarColor);
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
        if (note->Lane == RhythmEngine::PlasticFrets[0] && player.Instrument == PlasticDrums) {
            KickTimer = 1;
        }
        for (int i = 0; i < 7; i++) {
            if (slots[i]) {
                auto slot = slots[i];
                slot->AnimateHit();
            } else
                break;
        }
    }
}


inline double easeInQuadd(double x) {
    return x * x;

}

void Encore::Track::ProcessAnimation() {
    if (KickTimer > 0)
        KickTimer -= GetFrameTime()*5;
    else {
        KickTimer = 0;
    }

    AnimCamera.position.y = BaseCamera.position.y - (easeInQuadd(KickTimer)*0.2);
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
    // TODO: make the kick slot a different type
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

Encore::Track::~Track() {
}
