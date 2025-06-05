//
// Created by marie on 23/09/2024.
//

#ifndef ENCEVENTVEC_H
#define ENCEVENTVEC_H
#include "../../notes/EncNote.h"
#include <vector>

template <typename t>
struct EncEventVect {
    virtual ~EncEventVect() = default;
    std::vector<t> events {};

    t& operator=(int event) {
        return events[event];
    }

    t& operator[](int event) {
        return events[event];
    }

    virtual void CheckEvents(int &curEvent, double time) {
        if (!events.empty() && curEvent < events.size() - 1 && time > events[curEvent].EndSec) {
            curEvent++;
        }
    }

    virtual void ResetEvents() {
        for (auto &event : events) {
            event.NotesHit = 0;
        }
    }

    virtual bool Perfect(int curEvent) {
        if (!events.empty()) {
            return events[curEvent].NotesHit == events[curEvent].NoteCount;
        }
        return false;
    }

    virtual void UpdateEventViaNote(Note& note, int curEvent) {
        if (!events.empty()) {
            if (note.time >= events[curEvent].StartSec
                && note.time < events[curEvent].EndSec
                && note.hit) {
                ++events[curEvent].NotesHit;
            }
        }
    }
};

#endif //ENCEVENTVEC_H
