#pragma once
#include "events/Event.h"
#include "song/song.h"

namespace Encore {
    class ReadyUpEvent : public Event {
    public:
        Song* song;

        GotoReadyUpEvent(Song* song) : song(song) {};
    };

    class LoadSongEvent : public Event {
    public:
        Song* song;

        LoadSongEvent(Song* song) : song(song) {};
    };

    class PlaySongEvent : public Event {
    public:
        Song* song;

        PlaySongEvent(Song* song) : song(song) {};
    };

    class ShowResultsEvent : public Event {};
}