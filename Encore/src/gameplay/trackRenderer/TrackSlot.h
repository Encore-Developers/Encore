
#ifndef ENCORE_TRACKSLOT_H
#define ENCORE_TRACKSLOT_H
#include "Track.h"
#include "RhythmEngine/Notes/EncNote.h"
#include "raylib.h"


namespace Encore {
    class Track;

    class TrackSlot {
    public:
        float xPos;
        float width;
        int colorSlot; // TODO: figure out a good system for these
        Track *track;

        virtual void DrawNote(RhythmEngine::EncNote *note) = 0;
        virtual void DrawSustainTail(double startTime, double endTime) = 0;
        virtual void DrawSmasher(bool held) = 0;

        TrackSlot(Track *track, float xPos, float width, int colorSlot) : xPos(xPos), width(width), colorSlot(colorSlot), track(track) {};
    };

} // Encore

#endif // ENCORE_TRACKSLOT_H
