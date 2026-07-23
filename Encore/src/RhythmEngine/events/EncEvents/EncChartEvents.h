#pragma once
//
// Created by marie on 23/09/2024.
//

#include <string>
#include <cstdint>
#include <vector>

namespace Encore::RhythmEngine {
    struct TimePoint {
        double sec;
        int tick;
        explicit TimePoint(const double _sec, const int _tick) : sec(_sec), tick(_tick) {}
    };

    struct Event {
        TimePoint start;
        TimePoint end;
        explicit Event(const double _startSec, const int _startTick, const double _endSec, const int _endTick)
            : start(_startSec, _startTick), end(_endSec, _endTick) {}

        // doubt this is useful
        explicit Event(const TimePoint _start, const TimePoint _end)
            : start(_start), end(_end) {}

        int tickLen() const { return end.tick - start.tick; }
        double secLen() const { return end.sec - start.sec; }
    };

    struct NoteEvent : Event {
        enum NoteType : uint8_t {
            NORMAL = 0,
            HOPO = 1,
            LIFT = 1,
            CYMBAL = 1,
            TAP = 2
        };
        NoteEvent(const double _startSec, const int _startTick, const double _endSec, const int _endTick, const uint8_t _type, const uint8_t _lane)
            : Event(_startSec, _startTick, _endSec, _endTick), type(_type), lane(_lane) {}
        NoteEvent(const TimePoint _start, const TimePoint _end, const uint8_t _type, const uint8_t _lane)
            : Event(_start, _end), type(_type), lane(_lane)  {}
        uint8_t type = 0;
        uint8_t lane = 0;
        bool passed = false;
        bool LiftConverted = false;
        uint8_t OriginalLane = 0;
    };

    struct Syllable {
        TimePoint time;
        std::string syllable;
        bool talkie = false;
    };

    struct ChartEvent : Event {
        int NotesHit = 0;
        int NoteCount = 0;
        ChartEvent(const double _startSec, const int _startTick, const double _endSec, const int _endTick)
            : Event(_startSec, _startTick, _endSec, _endTick) {}
        ChartEvent(const TimePoint _start, const TimePoint _end)
            : Event(_start, _end) {}
    };

    struct LyricPhrase : Event {
        LyricPhrase(const double _startSec, const int _startTick, const double _endSec, const int _endTick)
            : Event(_startSec, _startTick, _endSec, _endTick) {}
        LyricPhrase(const TimePoint _start, const TimePoint _end)
            : Event(_start, _end) {}
        std::vector<Syllable> syllables;
    };

    struct Coda : ChartEvent {
        bool exists = false;
        /*
        bool IsNoteInCoda(Note &note) {
            if (exists) {
                if (note.time >= StartSec && note.time < EndSec) {
                    return true;
                }
            }
            return false;
        }
    */
        bool IsCodaActive(float time) const {
            if (exists) {
                if (time >= start.sec && time <= end.sec) {
                    return true;
                }
            }
            return false;
        }
    };

    struct solo : ChartEvent {
        solo(const double _startSec, const int _startTick, const double _endSec, const int _endTick)
            : ChartEvent(_startSec, _startTick, _endSec, _endTick) {}
        solo(const TimePoint _start, const TimePoint _end)
            : ChartEvent(_start, _end) {}
        void CountNote(const int tick) {
            if (tick >= start.tick && tick < end.tick) {
                NoteCount++;
            }
        }
    };

    struct trill : ChartEvent {
        trill(const double _startSec, const int _startTick, const double _endSec, const int _endTick)
            : ChartEvent(_startSec, _startTick, _endSec, _endTick) {}
        trill(const TimePoint _start, const TimePoint _end)
            : ChartEvent(_start, _end) {}
        uint8_t lane1 = 255;
        uint8_t lane2 = 255;
    };

    struct roll : ChartEvent {
        roll(const double _startSec, const int _startTick, const double _endSec, const int _endTick)
            : ChartEvent(_startSec, _startTick, _endSec, _endTick) {}
        roll(const TimePoint _start, const TimePoint _end)
            : ChartEvent(_start, _end) {}
        uint8_t lane = 255;
    };

    struct DrumFill : ChartEvent {};

    struct odPhrase : ChartEvent {
        enum PhraseState : uint8_t {
            Default = 0,
            Added = 1,
            Missed = 2,
        };
        odPhrase(const double _startSec, const int _startTick, const double _endSec, const int _endTick)
            : ChartEvent(_startSec, _startTick, _endSec, _endTick) {}
        odPhrase(const TimePoint _start, const TimePoint _end)
            : ChartEvent(_start, _end) {}
        PhraseState state = Default;
        bool added() const { return state == Added; }
        bool missed() const { return state == Missed; }
        void CountNote(const int tick) {
            if (tick >= start.tick && tick < end.tick) {
                NoteCount++;
            }
        }
        // bool added = false;
        // bool missed = false;
    };

    // unused, should fix
    struct section : ChartEvent {
        section(const double _startSec, const int _startTick, const double _endSec, const int _endTick)
            : ChartEvent(_startSec, _startTick, _endSec, _endTick) {}
        section(const TimePoint _start, const TimePoint _end)
            : ChartEvent(_start, _end) {}
        std::string Name;
    };

    struct tapPhrase : NoteEvent {};

    struct forceOnPhrase : NoteEvent {};

    struct openMarker : NoteEvent {};

    struct forceOffPhrase : NoteEvent {};
}
