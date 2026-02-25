
#include "events/Event.h"
#include <assert.h>

void Encore::EventSource::AddSink(EventSink *sink) {
    sinks.insert(sink);
    sink->sources.insert(this);
}

void Encore::EventSource::RemoveSink(EventSink *sink) {
    sinks.erase(sink);
    sink->sources.erase(this);
}
void Encore::EventSource::FireEvent(Event *event) {
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        auto sink = *it;
        sink->HandleEvent(event);
    }
}
Encore::EventSource::~EventSource() {
    for (auto it = sinks.begin(); it != sinks.end(); ++it) {
        auto sink = *it;
        sink->sources.erase(this);
    }
}

Encore::EventSink::~EventSink() {
    sources.clear();
    //for (auto it = sources.begin(); it != sources.end(); ++it) {
    //    auto source = *it;
    //    source->RemoveSink(this);
    //}
}
