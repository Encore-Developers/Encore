//
// Created by maria on 23/02/2026.
//

#include "OpenTrackSlot.h"

#include "assets.h"
#include "Track.h"

void Encore::OpenTrackSlot::DrawNote(RhythmEngine::EncNote *note, bool missed) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    Vector3 position = { xPos, 0.0, pos };
    Color color = track->player.QueryColorProfile(colorSlot);

    if (note->NoteType == 0) {
        ASSET(noteShader).SetUniform("noteColor", color);
        ASSET(noteShader).SetUniform("frameColor", WHITE);
    } else {
        ASSET(noteShader).SetUniform("noteColor", WHITE);
        ASSET(noteShader).SetUniform("frameColor", color);
    }
    if (note->LengthSeconds > 0) {
        DrawSustainTail(note->StartSeconds, note->StartSeconds + note->LengthSeconds);
    }
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