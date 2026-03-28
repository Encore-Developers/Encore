//
// Created by maria on 23/02/2026.
//

#include "OpenTrackSlot.h"

#include "assets.h"

void Encore::OpenTrackSlot::DrawNote(RhythmEngine::EncNote *note) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    Vector3 position = { xPos, 0.0, pos };
    Color color = track->player.QueryColorProfile(colorSlot);
    ASSET(noteShader).SetUniform("frameColor", WHITE);
    ASSET(noteShader).SetUniform("noteColor", color);

    DrawModelEx(ASSET(openNote), position, {0}, 0, {width/5.0f, track->NoteHeight, 1}, WHITE);
}

void Encore::OpenTrackSlot::DrawSustainTail(double startTime, double endTime) {
    if (endTime <= startTime) {
        return;
    }
    float midPos = track->GetNotePos3D((endTime + startTime) / 2);
    float sustainLength = endTime - startTime;

    DrawCube({ xPos, 0.1, midPos },
             0.4,
             0.01,
             sustainLength * track->GetZPerSecond(),
             track->player.QueryColorProfile(colorSlot));
}