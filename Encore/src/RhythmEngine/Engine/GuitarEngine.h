//
// Created by maria on 01/06/2025.
//

#ifndef GUITARENGINE_H
#define GUITARENGINE_H
#include "BaseEngine.h"
#include "GuitarStats.h"

namespace Encore::RhythmEngine {
    class GuitarEngine : public BaseEngine {
        bool ActivateOverdrive(ControllerEvent &event) override;
        void CheckMissedNotes(double CurrentTime);

        /*
         * STRUM PATH: ___________________________________________________________________
         * Check to see if the player overstrummed.
         * Check to set Fret After Strum
         * Check to see if frets are held down and the current note is strummable
         *
         * FRET (hopo/tap) PATH: _________________________________________________________
         * Check to see if Fret After Strum is active, and within limits.
         * If FAS isnt active, check if a combo is needed to hit the note.
         * Also check ghosting. that's a later addition lmao
         *
         * i should probably import the channel and action just to check if the strum path
         * is required to complete
         *
         *  wait. think about it
         *  strumming is just a "confirm"
         *  it shouldnt definitively say "yes" or "no" yet
         *  only when checking the frets doesnt work
         *  you're checking the frets after the strum path
         *
         *
         */
        int RunHitStateCheck(ControllerEvent &event) override;
        void HitNote(bool strumInput);
        bool PlayerIsPaused() override { return stats->Paused; };
        void TogglePause() override { stats->Paused = !stats->Paused; };
    public:
        // std::unordered_map<std::string, RhythmTimer> Timers {
        //     { "FAS", RhythmTimer(0.025) }, { "SAH", RhythmTimer(0.125) }
        // };
        void UpdateOnFrame(double CurrentTime) override;
        void SetStatsInputState(ControllerEvent &event) override;
        // bool CanNoteBeHit() override;
        void Overhit();
        std::shared_ptr<BaseChart> chart;
        std::shared_ptr<GuitarStats> stats;
        Player* player;
        GuitarEngine(auto _chart, auto _stats, Player* _player)
            : BaseEngine(_chart, _stats, _player), chart(_chart), stats(_stats), player(_player) {
            Timers = { { "FAS", RhythmTimer(0.125) }, { "SAH", RhythmTimer(goodBackend + goodFrontend) } };
        };
        ~GuitarEngine() override {};
        virtual bool UsesNoteMasks() override {return true;};
    };
}

#endif // GUITARENGINE_H