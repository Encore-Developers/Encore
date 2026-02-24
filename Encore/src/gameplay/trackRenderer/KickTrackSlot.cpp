//
// Created by maria on 23/02/2026.
//

#include "KickTrackSlot.h"

#include "assets.h"
#include "rlgl.h"

void Encore::KickTrackSlot::DrawNote(RhythmEngine::EncNote *note) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    Vector3 position = { xPos, 0.0, pos };

    // this is kinda nasty, just wanted a quick Thing
    Color color = track->player.QueryColorProfile(colorSlot);

    ASSET(noteShader).SetUniform("frameColor", WHITE);
    ASSET(noteShader).SetUniform("noteColor", color);


    rlDrawRenderBatchActive();

    DrawModelEx(ASSET(kickNote), position, {0}, 0, {1, track->NoteHeight / 2.0f, 1}, WHITE);
}

void Encore::KickTrackSlot::DrawSmasher(bool held) {
    Color color = track->player.QueryColorProfile(colorSlot);

    DrawModelEx(ASSET(kickFrame), {xPos, 0, 0}, {0}, 0, {1,1,1.3f}, WHITE);
    DrawModelEx(ASSET(kickPiston), {xPos, 0, 0}, {0}, 0, {1,1,1.3f}, color);
}