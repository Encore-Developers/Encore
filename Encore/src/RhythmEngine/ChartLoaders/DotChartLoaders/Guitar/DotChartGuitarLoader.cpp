#include "DotChartGuitarLoader.h"

namespace Encore {
    namespace RhythmEngine {
        int DotChartGuitarLoader::NoteTypeToLane(int type) {
            assert(type != 5 && type != 6);
            // Open note is type 7 but is 0 in our lanes
            if (type == 7) {
                return 0;
            }
            return 1 << type;
        }
        bool DotChartGuitarLoader::InHopoThresh(const NoteEvent &note) {
            if (!hasPreviousNote) {
                return false;
            }
            if (note.lane == 0) {
                return false;
            }
            int threshold = ((65*parent->tpq)/192);
            if (note.start.tick < previousNote.start.tick+threshold) {
                return (note.lane & previousNote.lane) == 0;
            }
            return false;
        }
        void DotChartGuitarLoader::StartNewNote(const DotChartEvent& event) {
            // *Hopefully* the modifiers always come after the note(s) itself
            int tick = std::stoi(event.key);
            int type = std::stoi(event.values.at(1));
            int length = std::stoi(event.values.at(2));
            TimePoint start = {TickToSeconds(tick), tick};
            TimePoint end = {0, 0};
            if (length > 0) {
                end = {TickToSeconds(tick+length), tick+length};
            }
            accumulatedNote = NoteEvent(start, end, 0, NoteTypeToLane(type));
            if (InHopoThresh(accumulatedNote)) {
                accumulatedNote.type = 1;
            }
            hasAccumulatedNote = true;
        }
        void DotChartGuitarLoader::FinishNote() {
            if (hasAccumulatedNote) {
                chart->Lanes[0].push_back(accumulatedNote);
                hasAccumulatedNote = false;
                previousNote = accumulatedNote;
                hasPreviousNote = true;
            }
        }
        void DotChartGuitarLoader::ReceiveEvent(const DotChartEvent& event) {
            DotChartBaseLoader::ReceiveEvent(event);

            int tick = std::stoi(event.key);

            if (event.values.at(0) == "N") {
                int type = std::stoi(event.values.at(1));
                int length = std::stoi(event.values.at(2));
                if (tick != accumulatedNote.start.tick) {
                    FinishNote();
                    StartNewNote(event);
                } else {
                    if (type <= 4 || type == 7) {
                        accumulatedNote.lane |= NoteTypeToLane(type);
                        accumulatedNote.type = 0;
                        if (length > 0) {
                            if (accumulatedNote.end.tick == 0 || accumulatedNote.end.tick < tick+length) {
                                accumulatedNote.end = {TickToSeconds(tick+length), tick+length};
                            }
                        }
                    }
                    if (type == 5) {
                        accumulatedNote.type = accumulatedNote.type == 0 ? 1 : 0;
                    }
                    if (type == 6) {
                        accumulatedNote.type = 2;
                    }
                }
            }
        }
        void DotChartGuitarLoader::EventsDone() {
            FinishNote();
        }
    } // RhythmEngine
} // Encore