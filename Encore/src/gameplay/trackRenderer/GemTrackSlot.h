#ifndef ENCORE_GEMTRACKSLOT_H
#define ENCORE_GEMTRACKSLOT_H
#include "TrackSlot.h"

namespace Encore {

    class GemTrackSlot : public TrackSlot {
    public:
        GemTrackSlot(Track *track, float xPos, float width, float length, ColorSlot colorSlot) : TrackSlot(track, xPos, width, length, colorSlot) {};
        GemTrackSlot(Track *track, float xPos, float width, ColorSlot colorSlot) : TrackSlot(track, xPos, width, 1, colorSlot) {};

        float animTimer = 1;
        Particle* hitFlare;
        unsigned int hitFlareId;

        virtual void DrawNote(RhythmEngine::EncNote *note);
        virtual void DrawSustainTail(double startTime, double endTime);
        virtual void DrawSmasher(bool held);
        virtual void AnimateHit();
    };

} // Encore

#endif // ENCORE_GEMTRACKSLOT_H
