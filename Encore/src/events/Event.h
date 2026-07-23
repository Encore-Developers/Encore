
#ifndef ENCORE_EVENT_H
#define ENCORE_EVENT_H

#include "RhythmEngine/REenums.h"

#include <unordered_set>

#include "RhythmEngine/events/EncEvents/EncChartEvents.h"

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
        RhythmEngine::NoteEvent* note;
        RhythmEngine::Judgement judgement = RhythmEngine::GOOD;
        double offset = 0;
        explicit NoteHitEvent(RhythmEngine::NoteEvent* note) : note(note) {}
        NoteHitEvent(RhythmEngine::NoteEvent* note, const RhythmEngine::Judgement _judgement, const float offset) : note(note), judgement(_judgement), offset(offset) {}
    };

    class OverhitEvent : public Event {
    public:
        size_t lane;
        explicit OverhitEvent(const size_t lane) : lane(lane) {}
    };

    class MultFlashEvent : public Event {
        public:
        bool comboBreak = false;
        explicit MultFlashEvent(const bool comboBreak) : comboBreak(comboBreak) {};
    };

    class TrackNotificationEvent : public Event {
    public:
        enum TrackNotificationType {
            OVERDRIVE_READY,
            COMBO,
            YOU_ROCK,
            BASSGROOVE,
            HOTSTART,
            FULLCOMBO
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


        virtual ~EventSource();
    };

    class EventSink {
    public:
        std::unordered_set<EventSource *> sources;

        virtual void HandleEvent(Event *) {};

        ~EventSink();
    };
}

#endif // ENCORE_EVENT_H
