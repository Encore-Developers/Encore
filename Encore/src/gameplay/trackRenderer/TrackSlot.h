
#ifndef ENCORE_TRACKSLOT_H
#define ENCORE_TRACKSLOT_H
#include "Track.h"
#include "RhythmEngine/Notes/EncNote.h"
#include "raylib.h"
#include "users/ColorProfile.h"

namespace Encore {
    class Track;

    class TrackSlot {
    public:
        float xPos;
        float width;
        float length;
        ColorSlot colorSlot;
        Track *track;
        int index = 0;

        virtual void DrawNote(RhythmEngine::EncNote *note) = 0;
        virtual void DrawSustainTail(double startTime, double endTime) = 0;
        virtual void DrawSmasher(bool held) = 0;
        virtual void AnimateHit(bool perfect) = 0;

        TrackSlot(Track *track, float xPos, float width, ColorSlot colorSlot) : xPos(xPos), width(width), length(1), colorSlot(colorSlot), track(track) {};
        TrackSlot(Track *track, float xPos, float width, float length, ColorSlot colorSlot) : xPos(xPos), width(width), length(length), colorSlot(colorSlot), track(track) {};
    };

} // Encore

#endif // ENCORE_TRACKSLOT_H
