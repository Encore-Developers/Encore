//
// Created by maria on 23/02/2026.
//

#include "KickTrackSlot.h"

#include "Track.h"
#include "assets.h"
#include "rlgl.h"
#include "easing/easing.h"



void Encore::KickTrackSlot::DrawNote(RhythmEngine::EncNote *note, bool missed) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    Vector3 position = { xPos, 0.0, pos };

    // this is kinda nasty, just wanted a quick Thing
    Color color = track->player.QueryColorProfile(colorSlot);
    if (track->player.engine->chart->overdrive.RenderNotesAsOD(note->StartSeconds)) {
        color = track->player.QueryColorProfile(SLOT_OVERDRIVE);
        ASSET(noteShader).SetUniform("frameColor", GOLD);
    } else {
        ASSET(noteShader).SetUniform("frameColor", WHITE);
    }
    ASSET(noteShader).SetUniform("noteColor", color);
    if (missed) {
        ASSET(noteShader).SetUniform("frameColor", Color{120, 120, 120, 255});
        ASSET(noteShader).SetUniform("noteColor", Color{ 255, 50, 50, 255 });
    }

    rlDrawRenderBatchActive();

    DrawModelEx(ASSET(kickNote), position, {0}, 0, {width/5.0f, track->NoteHeight / 2.0f, 0.75}, WHITE);
}

void Encore::KickTrackSlot::DrawSmasher(bool held) {
    Color color = track->player.QueryColorProfile(colorSlot);

    if (animTimer < 1) {
        animTimer += GetFrameTime() * 2.5f;
    } else {
        animTimer = 1;
    }

    if (overhitTimer > 0) {
        overhitTimer -= GetFrameTime() * 1.8;
    } else {
        overhitTimer = 0;
    }

    float bounce = (1 - getEasingFunction(EaseOutBounce)(animTimer)) * 0.9;



    DrawModelEx(ASSET(kickFrame), {xPos, 0, 0}, {0}, 0, {width/5.0f,1+(bounce),1.3f - (bounce*0.1f)}, ColorLerp(WHITE, {50, 50, 50, 255}, overhitTimer*0.2f));
    DrawModelEx(ASSET(kickPiston), {xPos, 0, 0}, {0}, 0, {width/5.0f,1+(bounce),1.3f - (bounce*0.1f)}, ColorLerp(color, {50, 50, 50, 255}, overhitTimer));
}

void Encore::KickTrackSlot::AnimateHit(bool perfect, Color color) {
    animTimer = 0;
    overhitTimer = 0;
    Particle part;
    part.active = true;
    part.type = KICKFLARE;
    part.position = { xPos, 0.15, 0.39 };
    part.color = color;
    track->particleSystem->SpawnParticle(part);
    part.position = { xPos, 0.15, -0.39 };
    track->particleSystem->SpawnParticle(part);
}
void Encore::KickTrackSlot::AnimateOverhit() {
    animTimer = 0.25;
    overhitTimer = 1;
}