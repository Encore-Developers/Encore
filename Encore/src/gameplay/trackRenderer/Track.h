//
// Created by maria on 14/01/2026.
//

#ifndef ENCORE_BASETRACK_H
#define ENCORE_BASETRACK_H
#include "TrackSlot.h"
#include "raylib.h"
#include "gameplay/enctime.h"
#include "users/player.h"

namespace Encore {
    class TrackSlot;

    class Track {
    public:
        void Draw();
        void Load();
        void DrawNotes();
        void DrawSmashers();
        void DrawBeatlines();
        void DrawSurface();
        void DrawOverdriveMeter();
        TrackSlot **GetSlotsForLane(uint8_t lane) const;

        void AddSlot(TrackSlot* slot);
        void Configure5Lane();
        void Configure4Lane();
        void ConfigureDrums();
        float GetNotePos3D(double noteTime);
        float GetViewEndTime() const;
        float GetZPerSecond() const;

        unsigned char BeatToCharViaTickThing(
            int tick,
            int MinBrightness,
            int MaxBrightness,
            int QuarterNoteLength
        );

        float NoteSpeed = 1;
        float BaseLength = 22;
        float Length = 22;
        float FadeSize = 3;
        float CurveFac = 50;
        float Offset = 0;
        float Scale = 1;

        Track(Player &player_)
            : player(player_) {
        };
        ~Track();
        Player& player;
    protected:
        Camera3D camera;
        std::vector<std::unique_ptr<TrackSlot>> slots;
        std::span<Beatline> BeatlinePool;
    };
}

#endif //ENCORE_BASETRACK_H