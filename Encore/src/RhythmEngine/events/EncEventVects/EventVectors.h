//
// Created by marie on 23/09/2024.
//

#ifndef ELSE_H
#define ELSE_H

#include "EncEventVect.h"
#include "../EncEvents/EncChartEvents.h"
#include "util/enclog.h"
#include "raylib.h"

namespace Encore::RhythmEngine {
    struct SoloEvents final : EncEventVect<solo> {
        bool TickDuringCurrentEvent(int tick) override;
        void UpdateEventViaNote(bool hit, int tick) override;
    };

    struct FillEvents final : EncEventVect<DrumFill> {
    };

    struct ODEvents final : EncEventVect<odPhrase> {
        void ResetEvents() override;

        void UpdateEventViaNote(bool hit, int tick) override;
        void CheckEvents(int tick) override;
        void MissCurrentEvent(int eventTime);
        bool RenderNotesAsOD(double time) const;
        bool TickDuringCurrentEvent(int tick) override;
        bool Perfect() override;

        // run CheckOverdrive instead of CheckEvents
        float CheckOverdrive(double sec);
    };

    struct SectionEvents final : EncEventVect<section> {
        // EncNote
        void UpdateEventViaNote(bool note, int curEvent) override;
    };

    // these last two arent as important to me.
    // when i actually get to this working ill probably move it to its own source files
    struct TrillEvents final : EncEventVect<trill> {
        // EncNote
        void UpdateEventViaNote(bool note, int curEvent) override {
            if (!this->empty())
                return;

            if (!TickDuringCurrentEvent(curEvent))
                return;

            if (note) {
                ++this->at(CurrentEvent).NotesHit;
            }
            ++this->at(CurrentEvent).NoteCount;
        }
    };

    struct RollEvents final : EncEventVect<roll> {
        // EncNote
        void UpdateEventViaNote(bool note, int curEvent) override {
            if (!this->empty())
                return;

            if (!TickDuringCurrentEvent(curEvent))
                return;

            if (note) {
                ++this->at(CurrentEvent).NotesHit;
            }
            ++this->at(CurrentEvent).NoteCount;
        }
    };
}

#endif // ELSE_H
