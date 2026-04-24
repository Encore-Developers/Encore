
#ifndef ENCORE_EVENT_H
#define ENCORE_EVENT_H

#include "RhythmEngine/REenums.h"
#include "RhythmEngine/Notes/EncNote.h"

#include <unordered_set>

namespace Encore {
    class Event {
    public:
        bool isEvent = true;

        virtual ~Event() = default;

        template<typename T>
        T* GetTyped() {
            return dynamic_cast<T*>(this);
        }
    };

    class NoteHitEvent : public Event {
    public:
        RhythmEngine::EncNote* note;
        RhythmEngine::Judgement judgement = RhythmEngine::GOOD;
        float offset = 0;
        NoteHitEvent(RhythmEngine::EncNote* note) : note(note) {}
        NoteHitEvent(RhythmEngine::EncNote* note, RhythmEngine::Judgement _judgement, float offset) : note(note), judgement(_judgement), offset(offset) {}
    };

    class OverhitEvent : public Event {
    public:
        int lane;
        OverhitEvent(int lane) : lane(lane) {}
    };

    class TrackNotificationEvent : public Event {
    public:
        enum TrackNotificationType {
            OVERDRIVE_READY,
            COMBO,
            YOU_ROCK
        };
        double time;
        TrackNotificationType type;
        int combo;
        TrackNotificationEvent(double _time, TrackNotificationType _type) : time(_time), type(_type) {}
        TrackNotificationEvent(double _time, TrackNotificationType _type, int _combo) : time(_time), type(_type), combo(_combo) {}
    };

    class HighwayBounceEvent : public Event {
    public:
        float timer = 1.0f;
        float mult = 5.0f;
        HighwayBounceEvent() {}
        HighwayBounceEvent(float _mult) : mult(_mult) {}
        HighwayBounceEvent(float _timer, float _mult) : timer(_timer), mult(_mult) {}
    };

    

    class EventSink;

    /// Can fire events
    class EventSource {
    public:
        std::unordered_set<EventSink *> sinks;

        void AddSink(EventSink *sink);
        void RemoveSink(EventSink *sink);

        void FireEvent(Event *);

        // Shorthand to hold the event temporarily on the stack
        template<typename T>
        void FireEventTemp(T ev) {
            T event = ev;
            FireEvent(&event);
        }


        ~EventSource();
    };

    class EventSink {
    public:
        std::unordered_set<EventSource *> sources;

        virtual void HandleEvent(Event *) {};

        ~EventSink();
    };
}

#endif // ENCORE_EVENT_H
