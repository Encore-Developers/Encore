
#include "GemTrackSlot.h"

void Encore::GemTrackSlot::DrawNote(RhythmEngine::EncNote *note) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    float finalWidth = width;

    if (note->NoteType == 1) finalWidth *= 0.5f;
    DrawCube({xPos, 0.2, pos}, finalWidth, 0.4, 0.5, track->player.QueryColorProfile(colorSlot));
}
void Encore::GemTrackSlot::DrawSustainTail(double startTime, double endTime) {

}
void Encore::GemTrackSlot::DrawSmasher(bool held) {
    // HACK: don't render open and kick smasher
    // REMOVE THIS LATER!
    if (colorSlot == SLOT_OPEN || colorSlot == SLOT_KICK) {
        return;
    }

    Color color = track->player.QueryColorProfile(colorSlot);

    if (index < track->player.engine->stats->HeldFrets.size() && !track->player.engine->stats->HeldFrets[index]) {
        color.r /= 2;
        color.g /= 2;
        color.b /= 2;
    }

    DrawCube({xPos, 0.025, 0}, width, 0.05, 1, color);
}