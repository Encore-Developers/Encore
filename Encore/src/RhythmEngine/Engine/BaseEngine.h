//
// Created by maria on 01/06/2025.
//

#ifndef BASEENGINE_H
#define BASEENGINE_H
#include "BaseStats.h"
#include "RhythmTimer.h"
#include "timingvalues.h"
#include "RhythmEngine/NoteVector.h"

#include <memory>
#include <span>
#include <unordered_map>

namespace Encore::RhythmEngine {

    enum HitState {
        HitNote = 0,
        OverhitNote = 1,
        CheckNextInput = 2
    };

    class BaseEngine {
    public:
        BaseEngine(auto _chart, auto _stats) : chart(_chart), stats(_stats) {};
        virtual ~BaseEngine() {};
        virtual void SetStatsInputState(InputChannel channel, Action action) {};
        bool EarlyStrike(double noteStartTime, double inputTime, double inputOffset) {
            if (noteStartTime - goodFrontend > inputTime - inputOffset) {
                return true;
            }
            return false;
        }
        bool InHitwindow(double noteStartTime, double inputTime, double inputOffset) {
            if ((noteStartTime - goodFrontend < inputTime - inputOffset)
                && (noteStartTime + goodBackend > inputTime - inputOffset)) {
                return true;
            }
            return false;
        }
        void ProcessInput(InputChannel channel, Action action);
        /*
         * Before hitting the note,
         * check to see if note can be hit with the current information
         * (currently ongoing combo, lift note, etc)
         * If that check went well, hit the note.
         * If not, the player overhit the note and should be penalized.
         */
        std::shared_ptr<BaseChart<EncNote, 5> > chart;
        std::shared_ptr<BaseStats<5> > stats;
        std::unordered_map<std::string, RhythmTimer> Timers;
        virtual void UpdateOnFrame(double CurrentTime) {

        };
        void CheckMissedNotes(int Lane, double SongTime) const {
            auto &chartLane = chart->at(Lane);
            if (chartLane.empty())
                return;
            auto itr = chartLane.begin();

            for (int notePos = 0; notePos < chartLane.size(); notePos++) {
                // no need to check things in the hitwindow
                auto& curNote = chartLane.at(notePos);
                if (curNote.StartSeconds + curNote.LengthSeconds + goodBackend
                    >= SongTime - stats->InputOffset) {
                    return;
                }
                if (curNote.StartSeconds + goodBackend < SongTime - stats->InputOffset
                    && !curNote.NotePassed) {
                    stats->Combo = 0;
                    stats->Misses += 1;
                    stats->AttemptedNotes += 1;
                    curNote.NotePassed = true;
                    chart->overdrive.UpdateEventViaNote(false, curNote.StartTicks);
                    stats->AudioMuted = true;
                    Encore::EncoreLog(
                        LOG_DEBUG, TextFormat("Missed note %01i", stats->AttemptedNotes)
                    );
                }
                if (curNote.StartSeconds + curNote.LengthSeconds + goodBackend + 0.5
                < SongTime - stats->InputOffset) {
                    chartLane.erase(chartLane.begin());
                }
            }

        }
        void RemoveMissedNote(
            const EncNote &note, const double CurrentTime, const int lane
        ) const {
            if (note.StartSeconds + note.LengthSeconds + goodBackend + 0.5
                < CurrentTime - stats->InputOffset) {
                chart->at(lane).erase(chart->at(lane).begin());
            }
        }
    private:
        virtual bool PlayerIsPaused() = 0;
        virtual void TogglePause() = 0;
        virtual void HitNote() {};
        virtual void Overhit() {};
        virtual int RunHitStateCheck(InputChannel channel, Action action) = 0;
        virtual bool ActivateOverdrive(InputChannel channel, Action action) = 0;

        bool PauseGame(InputChannel channel, Action action);
    };
}

#endif // BASEENGINE_H
