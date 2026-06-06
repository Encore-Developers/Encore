//
// Created by maria on 23/02/2026.
//

#include "OpenTrackSlot.h"

#include "assets.h"
#include "Track.h"
#include "rlgl.h"

void Encore::OpenTrackSlot::DrawNote(RhythmEngine::EncNote *note, bool missed) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    Vector3 position = { xPos, 0.0, pos };
    Color color = track->player.QueryColorProfile(colorSlot, track->ColorProfileType);


    if (track->player.engine->chart->overdrive.RenderNotesAsOD(note->StartSeconds)) {
        color = track->player.QueryColorProfile(SLOT_OVERDRIVE, track->ColorProfileType);
        ASSET(noteShader).SetUniform("frameColor", track->player.QueryColorProfile(SLOT_FRAME_OVERDRIVE, track->ColorProfileType));
    } else {
        ASSET(noteShader).SetUniform("frameColor", track->player.QueryColorProfile(SLOT_FRAME, track->ColorProfileType));
    }
    if (missed) {
        ASSET(noteShader).SetUniform("frameColor", Color{120, 120, 120, 255});
        color = { 140, 140, 140, 255 };
    }
    ASSET(noteShader).SetUniform("noteColor", color);

    if (note->LengthSeconds > 0) {
        Color sustainColor = missed ? GRAY : color;
        DrawSustainTail(note->StartSeconds, note->StartSeconds + note->LengthSeconds, sustainColor, 0);
    }
    rlDrawRenderBatchActive();

    if (note->NoteType == 1 || note->NoteType == 2) {
        DrawModelEx(ASSET(openHopoNote),
                    position,
                    { 0 },
                    0,
                    {width/5.0f, track->NoteHeight, length},
                    WHITE);

    } else {
        DrawModelEx(ASSET(openNote),
                    position,
                    { 0 },
                    0,
                    {width/5.0f, track->NoteHeight, length},
                    WHITE);
    }
}

void Encore::OpenTrackSlot::DrawSustainTail(double startTime, double endTime, Color color, float whammy) {
    if (endTime <= startTime) {
        return;
    }
    float midPos = track->GetNotePos3D((endTime + startTime) / 2);
    float sustainLength = endTime - startTime;

    DrawCube({ xPos, 0.1, midPos },
             0.4 * width + whammy,
             0.01,
             sustainLength * track->GetZPerSecond(),
             track->player.QueryColorProfile(colorSlot, track->ColorProfileType));
}

void Encore::OpenTrackSlot::DrawSmasher(bool held) {
    for (auto note : track->player.engine->chart->HeldNotePointers) {
        if (!note)
            continue;
        if (note->StartSeconds + note->LengthSeconds < TheSongTime.GetElapsedTime()) {
            continue;
        }
        bool matches = false;
        if (note->Lane == 0) {
            matches = true;
        }
        Color color = track->player.QueryColorProfile(colorSlot, track->ColorProfileType);
        if (track->player.engine->chart->overdrive.RenderNotesAsOD(note->StartSeconds)) {
            color = track->player.QueryColorProfile(SLOT_OVERDRIVE, track->ColorProfileType);
        }
        if (matches) {
            DrawSustainTail(TheSongTime.GetElapsedTime(),
                            note->StartSeconds + note->LengthSeconds, color, track->player.engine->whammy);
            break;
        }
    }
}