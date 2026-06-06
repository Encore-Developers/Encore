#include "PadConverters.h"
#include "RhythmEngine/REenums.h"
#include <bit>

using namespace Encore::RhythmEngine;
using namespace Encore::RhythmEngine::PadConverters;

void ChordPass(BaseChart &sourceChart) {
    for (auto& note : sourceChart.Lanes[0]) {
        int noteCount = std::popcount(note.Lane);
        int originalMask = note.Lane;
        if (noteCount > 1) {
            int lowest = 4;
            int highest = 0;
            for (int i = 0; i < 5; i++) {
                if (note.Lane & PlasticFrets[i]) {
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
            note.Lane = PlasticFrets[lowest] | PlasticFrets[highest];
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
NoteHand PadConverters::GetNoteHand(EncNote &note) {
    return GetLaneHand(note.Lane);
}

void SetOriginalLanePass(BaseChart &sourceChart) {
    for (int i = 0; i < sourceChart.Lanes[0].size(); i++) {
        sourceChart.Lanes[0][i].OriginalLane = sourceChart.Lanes[0][i].Lane;
    }
}

void HopoPass(BaseChart &sourceChart) {
    int hopoTemperature = 0;
    for (int i = 2; i < sourceChart.Lanes[0].size()-3; i++) {
        EncNote& note = sourceChart.Lanes[0][i];
        EncNote* prevNote = &sourceChart.Lanes[0][i-1];
        int noteCount = std::popcount(note.Lane);
        if (note.NoteType == 1 || note.NoteType == 2) {
            if (noteCount == 1) {
                if (prevNote) {
                    if (GetNoteHand(*prevNote) == GetNoteHand(note) && note.StartSeconds - prevNote->StartSeconds < 0.150 && sourceChart.Lanes[0][i+2].StartSeconds - sourceChart.Lanes[0][i+1].StartSeconds < 0.150 && note.Lane != sourceChart.Lanes[0][i+1].Lane) {
                        bool wasDifferentLane =
                            sourceChart.Lanes[0][i - 2].OriginalLane != note.OriginalLane;
                        // RH flips
                        if (note.Lane == PlasticFrets[2]) {
                            //note.Lane = PlasticFrets[0];
                            if (wasDifferentLane && sourceChart.Lanes[0][i-2].Lane == PlasticFrets[0]) {
                                note.Lane = PlasticFrets[1];
                            } else {
                                note.Lane = PlasticFrets[0];
                            }
                        }
                        else if (note.Lane == PlasticFrets[4]) {
                            note.Lane = PlasticFrets[1];
                        }
                        else if (note.Lane == PlasticFrets[3]) {
                            if (wasDifferentLane && sourceChart.Lanes[0][i-2].Lane == PlasticFrets[0]) {
                                note.Lane = PlasticFrets[0];
                            } else {
                                note.Lane = PlasticFrets[1];
                            }
                        }

                        // LH flips
                        else if (note.Lane == PlasticFrets[0]) {
                            note.Lane = PlasticFrets[2];
                        }
                        else if (note.Lane == PlasticFrets[1]) {
                            if (wasDifferentLane && sourceChart.Lanes[0][i-2].Lane == PlasticFrets[4]) {
                                note.Lane = PlasticFrets[3];
                            } else {
                                note.Lane = PlasticFrets[4];
                            }
                        }
                    }
                }
            }
        }
        if (noteCount == 1 && GetLaneHand(note.OriginalLane) == GetLaneHand(prevNote->OriginalLane)) {
            if (hopoTemperature < 2) {
                note.Lane = note.OriginalLane;
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
    EncNote* prevNote = nullptr;

    ChordPass(sourceChart);
    SetOriginalLanePass(sourceChart);
    HopoPass(sourceChart);

    for (auto& note : sourceChart.Lanes[0]) {
        bool liftConverted = false;
        for (int i = 0; i < 5; i++) {
            if (note.Lane & PlasticFrets[i]) {
                EncNote newNote = note;
                newNote.Lane = PlasticFrets[i];
                newNote.NoteType = 0;
                if (prevNote) {
                    if (!(note.Lane & ~prevNote->Lane) && prevNote->Lane & note.Lane) {
                        if ((note.StartSeconds - prevNote->StartSeconds < ((std::popcount(note.Lane) > 1) ? 0.175 : 0.150) && note.LengthSeconds == 0)
                            && !newChart.Lanes[i].empty()
                            && !prevNote->LiftConverted) {
                            newNote.NoteType = 1;
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