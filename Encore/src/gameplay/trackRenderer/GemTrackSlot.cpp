
#include "GemTrackSlot.h"

void Encore::GemTrackSlot::DrawNote(RhythmEngine::EncNote *note) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    float finalWidth = width;

    if (note->NoteType == 1) finalWidth *= 0.5f;
    DrawCube({xPos, 0.2, pos}, finalWidth, 0.4, 0.5, track->player.QueryColorProfile(colorSlot));
    if (note->LengthSeconds > 0) {
        DrawSustainTail(note->StartSeconds, note->StartSeconds+note->LengthSeconds);
    }
}
void Encore::GemTrackSlot::DrawSustainTail(double startTime, double endTime) {
    if (endTime <= startTime) {
        return;
    }
    float midPos = track->GetNotePos3D((endTime + startTime) / 2);
    float sustainLength = endTime - startTime;

    DrawCube({xPos, 0.1, midPos}, 0.2, 0.2, sustainLength*track->GetZPerSecond(), track->player.QueryColorProfile(colorSlot));

}
void Encore::GemTrackSlot::DrawSmasher(bool held) {
    // HACK: don't render open and kick smasher
    // REMOVE THIS LATER!
    if (colorSlot == SLOT_OPEN || colorSlot == SLOT_KICK) {
        return;
    }

    Color color = track->player.QueryColorProfile(colorSlot);

    if (held) {
        color.r /= 2;
        color.g /= 2;
        color.b /= 2;
    }

    DrawCube({xPos, 0.025, 0}, width, 0.05, 1, color);
    for (auto note : track->player.engine->chart->HeldNotePointers) {
        if (!note) continue;
        bool matches = false;
        if (track->player.engine->UsesNoteMasks()) {
            if (note->Lane & RhythmEngine::PlasticFrets[index]) {
                matches = true;
            }
        } else {
            if (note->Lane == index) {
                matches = true;
            }
        }
        if (matches) {
            DrawSustainTail(TheSongTime.GetElapsedTime(), note->StartSeconds+note->LengthSeconds);
            break;
        }
    }
}