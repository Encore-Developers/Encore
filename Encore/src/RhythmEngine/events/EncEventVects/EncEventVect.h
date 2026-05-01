//
// Created by marie on 23/09/2024.
//

#ifndef ENCEVENTVEC_H
#define ENCEVENTVEC_H

#include <vector>

template <typename t>
struct EncEventVect : std::vector<t> {
    virtual ~EncEventVect() = default;
    size_t CurrentEvent = 0;

    t &operator=(int event) { return this->at(event); }

    t &operator[](int event) { return this->at(event); }

    virtual bool TickDuringCurrentEvent(int tick) {
        if (tick >= this->at(CurrentEvent).StartTick
            && tick < this->at(CurrentEvent).EndTick) {
            return true;
        }
        return false;
    }

    virtual void CheckEvents(int tick) {
        if (this->empty())
            return;

        if (CurrentEvent < this->size() - 1 && tick >= this->at(CurrentEvent).EndTick) {
            CurrentEvent++;
        }
    }

    virtual void ResetEvents() {
        for (auto &event : *this) {
            event.NotesHit = 0;
        }
        CurrentEvent = 0;
    }

    virtual bool Perfect() {
        if (!this->empty()) {
            return this->at(CurrentEvent).NotesHit == this->at(CurrentEvent).NoteCount;
        }
        return false;
    }

    virtual void UpdateEventViaNote(bool hit, int tick) {
        if (!this->empty()) {
            if (tick >= this->at(CurrentEvent).StartTick
                && tick < this->at(CurrentEvent).EndTick && hit) {
                ++this->at(CurrentEvent).NotesHit;
            }
        }
    }
};

#endif //ENCEVENTVEC_H
