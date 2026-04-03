
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
    for (auto sink : sinks) {
        sink->HandleEvent(event);
    }
}
Encore::EventSource::~EventSource() {
    for (auto sink : sinks) {
        sink->sources.erase(this);
    }
}
Encore::EventSink::~EventSink() {
    for (auto source : sources) {
        source->sinks.erase(this);
    }
}