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
#include "menus/gameMenu.h"
#include "song/song.h"
#include "tracy/Tracy.hpp"
#include "users/playerManager.h"

void Encore::Track::Draw() {
    ZoneScopedN("Track Draw")
    // make a copy of BaseCamera to set AnimCamera to for calculations
    // please use this
    // if (ThePlayerManager.PlayersActive > 2) {
    //    BaseCamera.target.x = Offset * 3;
    //    BaseCamera.position.x = Offset * 2;
    // }

    if (ColumnFitting) {
        FitToColumn(ColumnLeft, ColumnRight, BaseCamera);
    }

    NoteSpeed = player.NoteSpeed; // TODO: should probably find a better way to do this
    Length = BaseLength * player.HighwayLength;
    player.engine->UpdateCalibration(player.InputCalibration);

    ProcessAnimation();
    if (ThePlayerManager.PlayersActive > 2 && ColumnFitting) {
        AnimCamera.target.x = Offset * 3;
        AnimCamera.position.x = Offset * 2;
        AnimCamera.position.y += (1 - Scale) * 6;
        AnimCamera.target.y -= (1 - Scale) * 6;
    }
    if (ColumnFitting) {
        FitToColumn(ColumnLeft, ColumnRight, AnimCamera);
    }

    BeginMode3D(AnimCamera);

    for (auto shader : { ASSETPTR(trackCurveShader), ASSETPTR(noteShader),
                         ASSETPTR(highwayScrollShader), ASSETPTR(overdriveShader),
                         ASSETPTR(multiplierFillShader), ASSETPTR(indicatorRingShader),
                         ASSETPTR(multNumShader), ASSETPTR(multiplierFrameShader) }) {
        shader->SetUniform("trackLength", Length);
        shader->SetUniform("fadeSize", FadeSize);
        shader->SetUniform("curveFac", CurveFac);
        shader->SetUniform("offset", Offset);
        shader->SetUniform("scale", Scale);
    }
    ASSET(multiplierFillShader).SetUniform("curveFac", 10000000000.0f);
    ASSET(indicatorRingShader).SetUniform("curveFac", 10000000000.0f);
    ASSET(multNumShader).SetUniform("curveFac", 10000000000.0f);
    ASSET(multiplierFrameShader).SetUniform("curveFac", 10000000000.0f);

    ASSET(indicatorRingShader).SetUniform("tex1", ASSET(fcindtex1));
    ASSET(indicatorRingShader).SetUniform("tex2", ASSET(fcindtex2));
    ASSET(indicatorRingShader).SetUniform("baseTex", ASSET(fcindtex3));

    BeginShaderMode(ASSET(trackCurveShader));
    rlDisableDepthTest();

    DrawSurface();
    BeginBlendMode(BLEND_ADDITIVE);
    DrawSolo();
    EndBlendMode();
    DrawBeatlines();
    DrawOverdriveMeter();
    EndShaderMode();
    EndMode3D();

    // this is really fucking stupid. bad fix.
    DrawJudgement();
    DrawCombo();

    BeginMode3D(AnimCamera);
    BeginShaderMode(ASSET(trackCurveShader));
    DrawMultiplier();
    DrawSmashers();

    EndShaderMode();
    EndMode3D();
    BeginMode3D(AnimCamera);
    BeginShaderMode(ASSET(trackCurveShader));
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
        { 0, 6.8f, -13.0f },
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
    ZoneScoped;
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

    OverdriveTimer = Lerp(OverdriveTimer,
                          (int)player.engine->stats->overdrive.Active,
                          GetFrameTime() * 8);
    if (OverdriveTimer > 0.01) {
        ASSET(trackSurface).Fetch().materials[0].maps[0].texture = ASSET(overdriveTex);
        DrawModelEx(ASSET(trackSurface),
                    { 0 },
                    { 0 },
                    0,
                    { 1, 1, 1 },
                    ColorAlpha(GOLD, easeInOutQuad(OverdriveTimer)));
    }

    SpotlightTimer = Lerp(SpotlightTimer,
                          (int)(player.engine->stats->multNoOD() >= 4),
                          GetFrameTime() * 3);
    if (SpotlightTimer > 0.01) {
        ASSET(trackSurface).Fetch().materials[0].maps[0].texture = ASSET(spotlightTex);
        DrawModelEx(ASSET(trackSurface),
                    { 0 },
                    { 0 },
                    0,
                    { 1, 1, 1 },
                    ColorAlpha(ColorBrightness(player.AccentColor, -0.25),
                               easeInOutQuad(SpotlightTimer)));
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
    ZoneScoped;

    int denom = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).denom;
    int numer = TheSongTime.TimeSigChanges.at(TheSongTime.CurrentTimeSig).numer;
    int flashInterval = (numer * 480) / denom;

    unsigned char streakFlash =
        BeatToCharViaTickThing(TheSongTime.GetCurrentTick(), 0, 255, flashInterval);
    float Percentage = float(streakFlash) / 255.0f;
    Color OverdriveBarColor = ColorBrightness(GOLD, Percentage);

    // DrawTriangleStrip3D(points.data(), points.size(), OverdriveBarColor);
    ASSET(overdriveShader).SetUniform("FillColor", OverdriveBarColor);
    ASSET(overdriveShader).SetUniform("FillPct",
                                      1.0f - player.engine->stats->overdrive.Fill);
    ASSET(overdriveShader).SetUniform("BaseColor",
                                      ColorBrightness(player.AccentColor, 0.75));
    DrawModelEx(ASSET(overdriveMeter),
                { 0, -0.1, -0.85 },
                { 1, 0, 0 },
                60,
                { 0.95, 0.8, 0.95 },
                player.AccentColor);

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

void Encore::Track::DrawSolo() {
    for (auto solo : player.engine->chart->solos) {
        float midPos = GetNotePos3D((solo.StartSec + (solo.StartSec + solo.EndSec)) / 2);
        float soloLength = (solo.StartSec + solo.EndSec) - (solo.StartSec);
        DrawCube({ 2.55, 0, midPos },
                 0.1,
                 0.1,
                 soloLength * GetZPerSecond(),
                 { BLUE.r, BLUE.g, BLUE.b, 128 });
        DrawCube({ -2.55, 0, midPos },
                 0.1,
                 0.1,
                 soloLength * GetZPerSecond(),
                 { BLUE.r, BLUE.g, BLUE.b, 128 });
    }
}

Vector2 MultiplierUVCalculation(bool sixmult, int combo, bool overdrive) {
    Vector2 curPos = { 0, 0 };

    int mult = combo / 10;
    if (sixmult) {
        mult = Clamp(mult, 0, 5);
    } else {
        mult = Clamp(mult, 0, 3);
    }
    curPos.x = mult * 0.25f;
    if (curPos.x >= 1) {
        curPos.x -= 1;
        curPos.y += 0.25;
    }
    if (overdrive) {
        curPos.y += 0.5;
    }
    return curPos;
}


void Encore::Track::DrawMultiplier() {
    ZoneScoped;
    Vector3 position = { 0, -0.1, -1.25 };
    Vector3 scale = { 1.1, 1.1, 1.1 };
    ASSET(indicatorRingShader).SetUniform("BaseColor",
                                          ColorBrightness(player.AccentColor, -0.3));
    ASSET(indicatorRingShader).SetUniform("FCColor", GOLD);
    ASSET(indicatorRingShader).SetUniform("time", GetTime());
    ASSET(multiplierFillShader).SetUniform("BaseColor",
                                           ColorBrightness(player.AccentColor, -0.85));
    int MaxMult = player.engine->stats->SixMultiplier ? 6 : 4;
    Color fillColor;
    if (player.engine->stats->multNoOD() == MaxMult) {
        fillColor = SKYBLUE;
    } else {
        fillColor = RAYWHITE;
    }
    ASSET(multiplierFillShader).SetUniform("MultiplierColor", fillColor);
    float fill = player.engine->stats->ComboFillCalc();
    if (fill <= 0.0f) {
        fill = -0.2;
    }
    if (fill >= 1.0f) {
        fill = 1.2f;
    }
    ASSET(multiplierFillShader).SetUniform("FillPercentage", fill);

    if (player.engine->stats->Misses == 0 && player.engine->stats->Overhits == 0) {
        ASSET(indicatorRingShader).SetUniform("isFC", 1.0f);
    } else {
        ASSET(indicatorRingShader).SetUniform("isFC", 0.0f);
    }
    Vector2 numberUV = MultiplierUVCalculation(player.engine->stats->SixMultiplier,
                                               player.engine->stats->Combo,
                                               player.engine->stats->overdrive.Active);

    ASSET(multNumShader).SetUniform("uvOffset", numberUV);
    DrawModelEx(ASSET(multiplierFill), position, { 0 }, 0, scale, WHITE);
    DrawModelEx(ASSET(indicatorRing), position, { 0 }, 0, scale, WHITE);
    DrawModelEx(ASSET(multiplierFrame),
                position,
                { 0 },
                0,
                scale,
                ColorBrightness(player.AccentColor, -0.5));
    DrawModelEx(ASSET(multNumPlane), position, { 0 }, 0, scale, WHITE);
}

float easeOutBack(float x) {
    float c1 = 1.70158;
    float c3 = c1 + 1;

    return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
}

float easeOutQuart(float x) {
    return 1 - pow(1 - x, 4);
}

float easeInQuart(float x) {
    return x * x * x * x;
}

void Encore::Track::DrawCombo() {
    if (player.engine->stats->Combo == 0) return;
    Units &u = Units::getInstance();
    Vector2 pos = {};
    Vector3 WorldMultiplierPosition = { 0, -0.1, -1.3 };
    unsigned char alpha = 255;
    float FontSize = u.hinpct(0.025f);
    // float TextWidth = MeasureTextEx(ASSET(rubikBold), JudgementStr.c_str(), FontSize, 0).
    float TextHeight = MeasureTextEx(ASSET(rubikBold), std::to_string(player.engine->stats->Combo).c_str(), FontSize, 0).
        y;
    float POffset = u.hinpct(0.05f);
    // perfect in

    Vector2 ScreenMultiplierPosition = GetWorldToScreen(
        WorldMultiplierPosition,
        AnimCamera);
    // float subtractStuff = (TextWidth * 0.25);
    // float xPos = ScreenMultiplierPosition.x - subtractStuff - POffset - (TextWidth *
    pos = { ScreenMultiplierPosition.x + POffset, ScreenMultiplierPosition.y - (TextHeight / 2) };
    pos.x += Offset * GetRenderWidth() * 0.5;

    GameMenu::mhDrawText(
        ASSET(rubikBold),
        std::to_string(player.engine->stats->Combo),
        pos,
        FontSize,
        ColorAlpha(WHITE, 0.75),
        ASSET(sdfShader),
        LEFT
    );
}

void Encore::Track::DrawJudgement() {
    Units &u = Units::getInstance();

    if (JudgementTimer > 0)
        JudgementTimer -= GetFrameTime() * 5;
    else {
        JudgementTimer = 0;
    }
    //
    Color JudgementColor = WHITE;
    std::string JudgementStr = "GOOD";
    switch (JudgementType) {
    case -1: {
        JudgementColor = RED;
        JudgementStr = "BAD";
        break;
    }
    case 1: {
        JudgementColor = GOLD;
        JudgementStr = "PERFECT";
        break;
    }
    default: {
        JudgementStr = "GOOD";
        break;
    }
    }
    // Not an if-else because *technically* 0 is possible
    if (LastHitOffset > 0) {
        JudgementStr = "+" + JudgementStr;
    }
    if (LastHitOffset < 0) {
        JudgementStr = "-" + JudgementStr;
    }
    Vector2 pos = {};
    Vector3 WorldMultiplierPosition = { 0, -0.1, -1.3 };
    unsigned char alpha = 255;
    float FontSize = u.hinpct(0.025f);
    float TextWidth = MeasureTextEx(ASSET(rubikBold), JudgementStr.c_str(), FontSize, 0).
        x;
    float TextHeight = MeasureTextEx(ASSET(rubikBold), JudgementStr.c_str(), FontSize, 0).
        y;
    float POffset = u.hinpct(0.05f);
    // perfect in
    float move = 0;
    double MaxAlpha = 255;
    if (JudgementTimer > 1) {
        if (JudgementType == 1) {
            move = 1 - easeInQuart(JudgementTimer - 1);
            FontSize = (FontSize * 0.75f) + ((FontSize * 0.25) * move);
            TextWidth = MeasureTextEx(ASSET(rubikBold), JudgementStr.c_str(), FontSize, 0)
                .x;
            TextHeight = MeasureTextEx(
                ASSET(rubikBold),
                JudgementStr.c_str(),
                FontSize,
                0).y;
        } else {
            move = 1;
            MaxAlpha = 128.0;
        }
        alpha = (unsigned char)(MaxAlpha * (1 - easeInQuart(JudgementTimer - 1)));
    } else {
        if (JudgementType == 1) {
            move = easeOutQuart(JudgementTimer);
        } else {
            move = 1;
            MaxAlpha = 128.0;
        }
        float alphaF = MaxAlpha * (easeInQuart(JudgementTimer) * 1.5);
        if (alphaF > MaxAlpha) {
            alpha = MaxAlpha;
        } else {
            alpha = (unsigned char)(alphaF);
        }
    }

    Vector2 ScreenMultiplierPosition = GetWorldToScreen(
        WorldMultiplierPosition,
        AnimCamera);
    float subtractStuff = (TextWidth * 0.25) * move;
    float xPos = ScreenMultiplierPosition.x - subtractStuff - POffset - (TextWidth *
        0.75);
    pos = { xPos, ScreenMultiplierPosition.y - (TextHeight / 2) };
    pos.x += Offset * GetRenderWidth() * 0.5;

    GameMenu::mhDrawText(
        ASSET(rubikBold),
        JudgementStr,
        pos,
        FontSize,
        { JudgementColor.r, JudgementColor.g, JudgementColor.b, alpha },
        ASSET(sdfShader),
        LEFT
    );
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
    ZoneScoped;
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
            auto slots = GetSlotsForNote(*note);
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
    ZoneScoped;
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
    ZoneScoped;
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
                Size = 0.5;
                break;
            }
            case Minor: {
                beatlineColor = { 255, 255, 255, 190 };
                Size = 0.2;
                break;
            }
            case Measure: {
                beatlineColor = { 255, 255, 255, 240 };
                Size = 0.75;
                break;
            }
            }
            DrawModelEx(ASSET(beatline),
                        { 0, 0, ScrollPos },
                        { 0 },
                        0,
                        { 1, 1, Size },
                        beatlineColor);
        }
    }
    rlDrawRenderBatchActive();
};

Encore::TrackSlot **Encore::Track::GetSlotsForNote(RhythmEngine::EncNote &note) const {
    static TrackSlot *slotBuffer[7];
    int curIndex = 0;
    auto append_slot = [&](int index) {
        if (index >= slots.size()) {
            return;
        }
        slotBuffer[curIndex] = slots[index].get();
        curIndex++;
    };
    auto lane = note.Lane;
    for (int i = 0; i < 5; i++) {
        if (lane & RhythmEngine::PlasticFrets[i]) {
            if (player.Instrument == PlasticDrums && note.NoteType == 1) {
                append_slot(i + 3);
            } else {
                append_slot(i);
            }
        }
    }
    if (lane == 0) {
        append_slot(5);
    }
    slotBuffer[curIndex] = nullptr;
    return (TrackSlot **)&slotBuffer;
}

Encore::TrackSlot **Encore::Track::GetSlotsForLane(uint8_t lane, bool forceMask) const {
    static TrackSlot *slotBuffer[7];
    int curIndex = 0;
    auto append_slot = [&](int index) {
        if (index >= slots.size()) {
            return;
        }
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
    if (auto bounceEvent = event->GetTyped<HighwayBounceEvent>()) {
        KickTimer = bounceEvent->timer;
        KickSpeedMult = bounceEvent->mult;
    }
    if (auto hitEvent = event->GetTyped<NoteHitEvent>()) {
        auto *note = hitEvent->note;
        auto slots = GetSlotsForNote(*note);
        JudgementTimer = 2;
        JudgementType = hitEvent->judgement;
        LastHitOffset = hitEvent->offset;
        for (int i = 0; i < 7; i++) {
            if (slots[i]) {
                auto slot = slots[i];
                if (slot->openHitAnim) {
                    auto allSlots = GetSlotsForLane(31, true);
                    for (int i = 0; i < 7; i++) {
                        if (allSlots[i]) {
                            auto slot = allSlots[i];
                            if (hitEvent->judgement == -1) {
                                slot->AnimateOverhit();
                            } else {
                                slot->AnimateHit(hitEvent->judgement, PURPLE);
                            }
                        } else
                            break;
                    }
                    break;
                }
                if (hitEvent->judgement == -1) {
                    slot->AnimateOverhit();
                } else {
                    slot->AnimateHit(hitEvent->judgement,
                                 player.QueryColorProfile(slot->colorSlot));
                }
            } else
                break;
        }
    }
    if (auto overhitEvent = event->GetTyped<OverhitEvent>()) {
        if (player.engine->UsesNoteMasks()) {
            for (auto &slot : slots) {
                if (slot->index < player.engine->stats->HeldFrets.size() && player.engine
                    ->stats->
                    HeldFrets[slot->index]) {
                    slot->AnimateOverhit();
                }
            }
        } else {
            auto slots = GetSlotsForLane(overhitEvent->lane);
            for (int i = 0; i < 7; i++) {
                if (slots[i]) {
                    auto slot = slots[i];
                    slot->AnimateOverhit();
                } else
                    break;
            }
        }
    }
}


inline double easeInQuadd(double x) {
    return x * x;
}

void Encore::Track::ProcessAnimation() {
    AnimCamera = BaseCamera;
    if (KickTimer > 0)
        KickTimer -= GetFrameTime() * KickSpeedMult;
    else {
        KickTimer = 0;
    }

    AnimCamera.position.y = BaseCamera.position.y - (easeInQuadd(KickTimer) * 0.2);
    AnimCamera.target.y = BaseCamera.target.y - (easeInQuadd(KickTimer) * 0.1);
}

void Encore::Track::AddSlot(TrackSlot *slot) {
    slot->index = slots.size();
    slots.emplace_back(slot);
}

void Encore::Track::Configure5Lane() {
    slots.clear();
    float xMult = player.LeftyFlip ? -1 : 1;
    AddSlot(new GemTrackSlot(this, 2 * xMult, 1, SLOT_GREEN));
    AddSlot(new GemTrackSlot(this, 1 * xMult, 1, SLOT_RED));
    AddSlot(new GemTrackSlot(this, 0 * xMult, 1, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -1 * xMult, 1, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -2 * xMult, 1, SLOT_ORANGE));
    AddSlot(new OpenTrackSlot(this, 0 * xMult, 5, SLOT_OPEN));
}

void Encore::Track::Configure5LaneKickOpen() {
    slots.clear();
    float xMult = player.LeftyFlip ? -1 : 1;
    AddSlot(new GemTrackSlot(this, 2 * xMult, 1, 0.75, SLOT_GREEN));
    AddSlot(new GemTrackSlot(this, 1 * xMult, 1, 0.75, SLOT_RED));
    AddSlot(new GemTrackSlot(this, 0 * xMult, 1, 0.75, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -1 * xMult, 1, 0.75, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -2 * xMult, 1, 0.75, SLOT_ORANGE));
    AddSlot(new KickTrackSlot(this, 0 * xMult, 5, SLOT_OPEN));
}

void Encore::Track::Configure5LaneGemOpen() {
    slots.clear();
    float xMult = player.LeftyFlip ? -1 : 1;
    AddSlot(new GemTrackSlot(this, 1.25 * xMult, 5.0 / 6.0, SLOT_GREEN));
    AddSlot(new GemTrackSlot(this, 0.41666 * xMult, 5.0 / 6.0, SLOT_RED));
    AddSlot(new GemTrackSlot(this, -0.41666 * xMult, 5.0 / 6.0, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -1.25 * xMult, 5.0 / 6.0, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -2.08333 * xMult, 5.0 / 6.0, SLOT_ORANGE));
    AddSlot(new GemTrackSlot(this, 2.08333 * xMult, 5.0 / 6.0, SLOT_OPEN));
}

void Encore::Track::Configure4Lane() {
    slots.clear();
    AddSlot(new GemTrackSlot(this, 1.875, 1.25, SLOT_GREEN));
    AddSlot(new GemTrackSlot(this, 0.625, 1.25, SLOT_RED));
    AddSlot(new GemTrackSlot(this, -0.625, 1.25, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -1.875, 1.25, SLOT_BLUE));
}

void Encore::Track::ConfigureDrums() {
    slots.clear();
    AddSlot(new KickTrackSlot(this, 0, 5, SLOT_KICK));
    AddSlot(new GemTrackSlot(this, 1.875, 1.25, 0.75, SLOT_RED));
    AddSlot(new GemTrackSlot(this, 0.625, 1.25, 0.75, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -0.625, 1.25, 0.75, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -1.875, 1.25, 0.75, SLOT_GREEN));

    AddSlot(new GemTrackSlot(this, 0.625, 1.25, 0.75, SLOT_HIHAT, slots[2].get()));
    AddSlot(new GemTrackSlot(this, -0.625, 1.25, 0.75, SLOT_RIDE, slots[3].get()));
    AddSlot(new GemTrackSlot(this, -1.875, 1.25, 0.75, SLOT_CRASH, slots[4].get()));
}

void Encore::Track::ConfigurePSDrums() {
    slots.clear();
    AddSlot(new KickTrackSlot(this, 0, 5, SLOT_KICK));
    AddSlot(new GemTrackSlot(this, 2.25, 0.5, 0.75, SLOT_RED));
    AddSlot(new GemTrackSlot(this, 0.75, 0.5, 0.75, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -0.75, 0.5, 0.75, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -2.25, 0.5, 0.75, SLOT_GREEN));
    AddSlot(new GemTrackSlot(this, 1.5, 1.0, 0.75, SLOT_HIHAT));
    AddSlot(new GemTrackSlot(this, 0.0, 1.0, 0.75, SLOT_RIDE));
    AddSlot(new GemTrackSlot(this, -1.5, 1.0, 0.75, SLOT_CRASH));
}

void Encore::Track::ConfigureFuckYoyDrums() {
    slots.clear();
    AddSlot(new GemTrackSlot(this, 0.3125, 0.625, 0.75, SLOT_KICK));
    AddSlot(new GemTrackSlot(this, 2.18, 0.625, 0.75, SLOT_RED));
    AddSlot(new GemTrackSlot(this, 0.9375, 0.625, 0.75, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -0.9375, 0.625, 0.75, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -2.18, 0.625, 0.75, SLOT_GREEN));
    AddSlot(new GemTrackSlot(this, 1.5625, 0.625, 0.75, SLOT_HIHAT));
    AddSlot(new GemTrackSlot(this, -0.3125, 0.625, 0.75, SLOT_RIDE));
    AddSlot(new GemTrackSlot(this, -1.5625, 0.625, 0.75, SLOT_CRASH));
}

void Encore::Track::ConfigureDrumsGemKick() {
    slots.clear();
    AddSlot(new GemTrackSlot(this, 2, 1, SLOT_KICK));
    AddSlot(new GemTrackSlot(this, 1, 1, SLOT_RED));
    AddSlot(new GemTrackSlot(this, 0, 1, SLOT_YELLOW));
    AddSlot(new GemTrackSlot(this, -1, 1, SLOT_BLUE));
    AddSlot(new GemTrackSlot(this, -2, 1, SLOT_GREEN));

    AddSlot(new GemTrackSlot(this, 0, 1, 1, SLOT_YELLOW, slots[2].get()));
    AddSlot(new GemTrackSlot(this, -1, 1, 1, SLOT_BLUE, slots[3].get()));
    AddSlot(new GemTrackSlot(this, -2, 1, 1, SLOT_GREEN, slots[4].get()));
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

void Encore::Track::FitToColumn(float left, float right, Camera &camera) {
    float currentLeft = GetWorldToScreenEx({ 3, 0, 0 },
                                           camera,
                                           GetRenderWidth(),
                                           GetRenderHeight()).x;
    float currentRight = GetWorldToScreenEx({ -3, 0, 0 },
                                            camera,
                                            GetRenderWidth(),
                                            GetRenderHeight()).x;
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
