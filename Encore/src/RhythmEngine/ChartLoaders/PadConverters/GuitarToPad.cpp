#include "PadConverters.h"
#include "RhythmEngine/REenums.h"
#include <bit>

using namespace Encore::RhythmEngine;
using namespace Encore::RhythmEngine::PadConverters;

void ChordPass(BaseChart &sourceChart) {
    for (auto& note : sourceChart.Lanes[0]) {
        int noteCount = std::popcount(note.lane);
        int originalMask = note.lane;
        if (noteCount > 1) {
            int lowest = 4;
            int highest = 0;
            for (int i = 0; i < 5; i++) {
                if (note.lane & PlasticFrets[i]) {
                    if (i < lowest) {
                        lowest = i;
                    }
                    if (i > highest) {
                        highest = i;
                    }
                }
            }
            if (lowest == 0 && highest == 1) {
                highest = 2;
            } else if (lowest > 1 && highest > 1) {
                if (lowest == 2) {
                    lowest--;
                    highest--;
                } else if (lowest == 3) {
                    lowest = 1;
                }
            }
            note.lane = PlasticFrets[lowest] | PlasticFrets[highest];
        }
    }
}

uint8_t HandFlipLane(uint8_t lane) {
    switch (GetLaneHand(lane)) {
    case CHORD:
        return lane;
    case LEFT:
        return lane << 2;
    case RIGHT:
        return lane >> 2;
    }
}

NoteHand PadConverters::GetLaneHand(uint8_t note) {
    if (std::popcount(note) > 1) {
        return CHORD;
    }
    return note <= PlasticFrets[1] ? LEFT : RIGHT;
}
NoteHand PadConverters::GetNoteHand(NoteEvent &note) {
    return GetLaneHand(note.lane);
}

void SetOriginalLanePass(BaseChart &sourceChart) {
    for (int i = 0; i < sourceChart.Lanes[0].size(); i++) {
        sourceChart.Lanes[0][i].OriginalLane = sourceChart.Lanes[0][i].lane;
    }
}

void HopoPass(BaseChart &sourceChart) {
    int hopoTemperature = 0;
    for (int i = 2; i < sourceChart.Lanes[0].size()-3; i++) {
        NoteEvent& note = sourceChart.Lanes[0][i];
        NoteEvent* prevNote = &sourceChart.Lanes[0][i-1];
        int noteCount = std::popcount(note.lane);
        if (note.type == 1 || note.type == 2) {
            if (noteCount == 1) {
                if (prevNote) {
                    if (GetNoteHand(*prevNote) == GetNoteHand(note) && note.start.sec - prevNote->start.sec < 0.150 && sourceChart.Lanes[0][i+2].start.sec - sourceChart.Lanes[0][i+1].start.sec < 0.150 && note.lane != sourceChart.Lanes[0][i+1].lane) {
                        bool wasDifferentLane =
                            sourceChart.Lanes[0][i - 2].OriginalLane != note.OriginalLane;
                        // RH flips
                        if (note.lane == PlasticFrets[2]) {
                            //note.lane = PlasticFrets[0];
                            if (wasDifferentLane && sourceChart.Lanes[0][i-2].lane == PlasticFrets[0]) {
                                note.lane = PlasticFrets[1];
                            } else {
                                note.lane = PlasticFrets[0];
                            }
                        }
                        else if (note.lane == PlasticFrets[4]) {
                            note.lane = PlasticFrets[1];
                        }
                        else if (note.lane == PlasticFrets[3]) {
                            if (wasDifferentLane && sourceChart.Lanes[0][i-2].lane == PlasticFrets[0]) {
                                note.lane = PlasticFrets[0];
                            } else {
                                note.lane = PlasticFrets[1];
                            }
                        }

                        // LH flips
                        else if (note.lane == PlasticFrets[0]) {
                            note.lane = PlasticFrets[2];
                        }
                        else if (note.lane == PlasticFrets[1]) {
                            if (wasDifferentLane && sourceChart.Lanes[0][i-2].lane == PlasticFrets[4]) {
                                note.lane = PlasticFrets[3];
                            } else {
                                note.lane = PlasticFrets[4];
                            }
                        }
                    }
                }
            }
        }
        if (noteCount == 1 && GetLaneHand(note.OriginalLane) == GetLaneHand(prevNote->OriginalLane)) {
            if (hopoTemperature < 2) {
                note.lane = note.OriginalLane;
            }
            hopoTemperature += 1;
            if (hopoTemperature > 5) {
                hopoTemperature = 5;
            }
        } else {
            hopoTemperature -= 1;
            if (hopoTemperature < 0) {
                hopoTemperature = 0;
            }
        }
    }
}

BaseChart
PadConverters::ConvertGuitarToPad(BaseChart &sourceChart) {
    BaseChart newChart;
    NoteEvent* prevNote = nullptr;

    ChordPass(sourceChart);
    SetOriginalLanePass(sourceChart);
    HopoPass(sourceChart);

    for (auto& note : sourceChart.Lanes[0]) {
        bool liftConverted = false;
        for (int i = 0; i < 5; i++) {
            if (note.lane & PlasticFrets[i]) {
                NoteEvent newNote = note;
                newNote.lane = PlasticFrets[i];
                newNote.type = 0;
                if (prevNote) {
                    if (!(note.lane & ~prevNote->lane) && prevNote->lane & note.lane) {
                        if ((note.start.sec - prevNote->start.sec < ((std::popcount(note.lane) > 1) ? 0.175 : 0.150) && note.secLen() == 0)
                            && !newChart.Lanes[i].empty()
                            && !prevNote->LiftConverted) {
                            newNote.type = 1;
                            liftConverted = true;
                        }
                    }
                }
                newChart.Lanes[i].push_back(newNote);
            }
        }
        if (liftConverted) {
            note.LiftConverted = true;
        }
        prevNote = &note;
    }
    newChart.sections = sourceChart.sections;
    newChart.overdrive = sourceChart.overdrive;
    newChart.solos = sourceChart.solos;
    newChart.BaseScore = sourceChart.BaseScore;
    return newChart;
}