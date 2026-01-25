
#include "GemTrackSlot.h"

void Encore::GemTrackSlot::DrawNote(RhythmEngine::EncNote *note) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    DrawCube({xPos, 0, pos}, 1, 0.5, 0.5, WHITE);
}
void Encore::GemTrackSlot::DrawSustainTail(double startTime, double endTime) {

}
void Encore::GemTrackSlot::DrawSmasher(bool held) {

}