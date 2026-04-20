//
// Created by maria on 14/01/2026.
//

#ifndef ENCORE_BASETRACK_H
#define ENCORE_BASETRACK_H
#include "TrackSlot.h"
#include "raylib.h"
#include "events/Event.h"
#include "gameplay/enctime.h"
#include "particles/ParticleSystem.h"
#include "users/player.h"

namespace Encore {
    class TrackSlot;
    class ParticleSystem;

    class Track : public EventSink {
    public:
        void DrawTrackDebugWindow();
        void Draw();
        void Load();
        void DrawNotes();
        void DrawSmashers();
        void DrawBeatlines();
        void DrawSurface();
        void DrawOverdriveMeter();
        void DrawSolo();
        void DrawMultiplier();

        void DrawJudgement();
        TrackSlot **GetSlotsForNote(RhythmEngine::EncNote& note) const;
        TrackSlot **GetSlotsForLane(uint8_t lane, bool forceMask = false) const;

        virtual void HandleEvent(Event *);

        void ProcessAnimation();

        void AddSlot(TrackSlot* slot);
        void Configure5Lane();
        void Configure5LaneKickOpen();
        void Configure5LaneGemOpen();
        void Configure4Lane();
        void ConfigureDrums();
        void ConfigurePSDrums();
        void ConfigureFuckYoyDrums();
        void ConfigureDrumsGemKick();
        float GetNotePos3D(double noteTime);
        float GetViewEndTime() const;
        float GetZPerSecond() const;

        void FitToColumn(float left, float right, Camera& camera);

        unsigned char BeatToCharViaTickThing(
            int tick,
            int MinBrightness,
            int MaxBrightness,
            int QuarterNoteLength
        );

        float NoteSpeed = 1;
        float BaseLength = 20;
        float Length = 20;
        float FadeSize = 3;
        float CurveFac = 50;
        float Offset = 0;
        float Scale = 1;
        float NoteHeight = 1;
        float KickTimer = 0;
        float KickSpeedMult = 5;
        float SpotlightTimer = 0;
        float OverdriveTimer = 0;
        float JudgementTimer = 0;
        int JudgementType = 0;
        // The column of the screen that this track can occupy. Used for multiplayer positioning.
        float ColumnLeft = -1;
        float ColumnRight = 1;
        bool ColumnFitting = true;

        Camera3D AnimCamera;
        Camera3D BaseCamera;

        std::unique_ptr<ParticleSystem> particleSystem;

        Track(Player &player_)
            : player(player_) {
        };
        ~Track();
        Player& player;
    protected:

        std::vector<std::unique_ptr<TrackSlot>> slots;
        std::span<Beatline> BeatlinePool;
    };
}

#endif //ENCORE_BASETRACK_H