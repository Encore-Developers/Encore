
#ifndef ENCORE_EVENT_H
#define ENCORE_EVENT_H

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
        bool perfect = false;
        NoteHitEvent(RhythmEngine::EncNote* note) : note(note) {}
        NoteHitEvent(RhythmEngine::EncNote* note, bool _perfect) : note(note), perfect(_perfect) {}
    };

    

    class EventSink;

    /// Can fire events
    class EventSource {
    public:
        std::unordered_set<EventSink *> sinks;

        void AddSink(EventSink *sink);
        void RemoveSink(EventSink *sink);

        void FireEvent(Event *);


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
