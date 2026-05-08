#include "PadConverters.h"
#include "RhythmEngine/REenums.h"

using namespace Encore::RhythmEngine;

BaseChart
PadConverters::ConvertGuitarToPad(BaseChart &sourceChart) {
    BaseChart newChart;
    EncNote* prevNote = nullptr;
    for (auto& note : sourceChart.Lanes[0]) {
        for (int i = 0; i < 5; i++) {
            if (note.Lane & PlasticFrets[i]) {
                EncNote newNote = note;
                newNote.Lane = PlasticFrets[i];
                newNote.NoteType = 0;
                if (prevNote) {
                    if (!(prevNote->Lane & ~note.Lane) && prevNote->Lane & note.Lane) {
                        if (note.StartSeconds - prevNote->StartSeconds < 0.150
                            && !newChart.Lanes[i].empty()
                            && newChart.Lanes[i].back().NoteType == 0
                            && newChart.Lanes[i].back().StartTicks
                                == prevNote->StartTicks) {
                            newNote.NoteType = 1;
                        }
                    }
                }
                newChart.Lanes[i].push_back(newNote);
            }
        }
        prevNote = &note;
    }
    return newChart;
}