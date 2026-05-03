//
// Created by maria on 17/05/2025.
//

#include "GuitarLoader.h"

#include "RhythmEngine/REenums.h"
#include "song/scoring.h"
std::vector<smf::uchar> psDiff { 0x00, 0x01, 0x02, 0x03 };

static bool HasPSSig(const smf::MidiEvent &event) {
    return event[1] == 'P' && event[2] == 'S' && event[3] == '\0';
}
static bool HasPSMeta(const smf::MidiEvent &event) {
    return (event[0] == 0xF0 && HasPSSig(event));
}

// i hate sysex
[[nodiscard]] bool WithinSEXDiff(int diff, const smf::MidiEvent &event) {
    return (event[5] == psDiff[diff] || event[5] == 0xFF);
}
[[nodiscard]] bool IsSysEx(int difficulty, const smf::MidiEvent &event) {
    return HasPSMeta(event) && WithinSEXDiff(difficulty, event);
}
static bool IsSysExTap(const smf::MidiEvent &event) {
    return (event[6] == 0x04);
}
static bool IsSysExOpen(const smf::MidiEvent &event) {
    return (event[6] == 0x01);
}
static bool IsSEXOff(const smf::MidiEvent &event) {
    return (event[7] == 0x00);
}
static bool IsSEXOn(const smf::MidiEvent &event) {
    return (event[7] == 0x01);
}
void Encore::RhythmEngine::GuitarLoader::SysExTap(const smf::MidiEvent &event) {
    if (IsSysExTap(event)) {
        if (IsSEXOn(event)) {
            TapMarker.emplace(event.tick, 0);
        } else if (IsSEXOff(event)) {
            TapMarker.back().second = event.tick;
        }
    }
}
void Encore::RhythmEngine::GuitarLoader::SysExOpen(const smf::MidiEvent &event) {
    if (IsSysExOpen(event)) {
        if (IsSEXOn(event)) {
            OpenMarker.emplace(event.tick, 0);
        } else if (IsSEXOff(event)) {
            OpenMarker.back().second = event.tick;
        }
    }
}
void Encore::RhythmEngine::GuitarLoader::CheckSysEx(const smf::MidiEvent &event) {
    if (IsSysEx(Difficulty, event)) {
        SysExTap(event);
        SysExOpen(event);
    }
}

void Encore::RhythmEngine::GuitarLoader::CheckHopos(const smf::MidiEvent &event) {
    if (event.isNoteOn()) {
        if (event[1] == 101) {
            ForceHopoOn.emplace(event.tick, event.getLinkedEvent()->tick);
        } else if (event[1] == 102) {
            ForceHopoOff.emplace(event.tick, event.getLinkedEvent()->tick);
        };
    }
}
void Encore::RhythmEngine::GuitarLoader::CheckTaps(const smf::MidiEvent &event) {
    if (event.isNoteOn()) {
        if (event[1] == 104) {
            TapMarker.emplace(event.tick, event.getLinkedEvent()->tick);
        };
    }
}
[[nodiscard]] int
Encore::RhythmEngine::GuitarLoader::GetNoteType(const smf::MidiEvent &event) {
    if (!TapMarker.empty()) {
        if (TapMarker.front().first <= event.tick) {
            return 2;
        }
    }
    if (!ForceHopoOn.empty()) {
        if (ForceHopoOn.front().first <= event.tick) {
            return 1;
        }
    }
    if (!ForceHopoOff.empty()) {
        if (ForceHopoOff.front().first <= event.tick) {
            return 0;
        }
    }
    if (!chart[0].empty()) {
        // DUDE ARE YOU A FUCKING MORON
        // HOPOS HAVE SPECIFIC REQURIEMENTS TO BE HOPOS, NOT JUST "its close enough"
        // GOD FUCKING DAMN
        // ALSO THAT IF STATEMENT IS FUCKING UTTERLY USELESS
        // oh my god i might DIE what the FUCK
        if (chart[0].back().Lane != PlasticFrets[GetEventLane(Difficulty, event)]) {
            if (chart[0].back().StartTicks + Threshold >= event.tick && !(PlasticFrets[GetEventLane(Difficulty, event)] & chart[0].back().Lane)) {
                return 1;
            }
        }
    }
    return 0;
}
void Encore::RhythmEngine::GuitarLoader::CheckEvents(const smf::MidiEvent &event) {
    ITERATE_EVENT_BY_NOTE(solos, CurrentSolo, event)
    if (!chart.overdrive.empty()) {
        if (CurrentOverdrive < chart.overdrive.size() - 1
            && chart.overdrive[CurrentOverdrive].StartTick + chart.overdrive[CurrentOverdrive].TickLength <= event.tick)
            CurrentOverdrive++;
    }
    // ITERATE_EVENT_BY_NOTE(overdrive, CurrentOverdrive, event)
    ITERATE_EVENT_BY_NOTE(trills, CurrentTrill, event)
    ITERATE_EVENT_BY_NOTE(rolls, CurrentRoll, event)
}
void Encore::RhythmEngine::GuitarLoader::GetChartEvents(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        ATTEMPT_TO_ADD_CHART_EVENT(116, overdrive, event);
        ATTEMPT_TO_ADD_CHART_EVENT(103, solos, event);
        ATTEMPT_TO_ADD_CHART_EVENT(127, trills, event);
        ATTEMPT_TO_ADD_CHART_EVENT(126, rolls, event);
    }
}
void Encore::RhythmEngine::GuitarLoader::CreateNote(const smf::MidiEvent &event) {
    int lengthTicks = event.getLinkedEvent()->tick - event.tick;
    double lengthSec = event.getLinkedEvent()->seconds - event.seconds;
    int lane = PlasticFrets[GetEventLane(Difficulty, event)];
    if (event.getLinkedEvent()->tick - event.tick < 170) {
        lengthTicks = 0;
        lengthSec = 0;
    }
    if (lengthTicks > 0) {
        chart.BaseScore += static_cast<double>(int(lengthTicks / 480)) * BASE_SCORE_SUSTAIN_POINTS;
        lengthTicks -= 1;
    }
    if (!OpenMarker.empty()) {
        if (OpenMarker.front().first <= event.tick) {
            lane = 0;
        }
    }

    if (!chart[0].empty()) {
        int noteEnd = chart[0].back().StartTicks + chart[0].back().LengthTicks;
        if (chart[0].back().LengthTicks > 0 && noteEnd >= event.tick - 80 && noteEnd <= event.tick) {
            chart[0].back().LengthTicks -= 80;
            chart[0].back().LengthSeconds = midiFile->getTimeInSeconds(noteEnd - 80) - chart[0].back().StartSeconds;
        }
    }
    chart[0].emplace_back(
        event.tick,
        lengthTicks,
        event.seconds,
        lengthSec,
        GetNoteType(event),
        lane
    );
    if (!chart.solos.empty()) {
        if (event.tick >= chart.solos[CurrentSolo].StartTick && event.tick < chart.solos[CurrentSolo].StartTick + chart.solos[CurrentSolo].TickLength) {
            chart.solos[CurrentSolo].NoteCount++;
        }
    }
    if (!chart.trills.empty()) {
        if (event.tick >= chart.trills[CurrentTrill].StartTick) {
            if (chart.trills[CurrentTrill].lane1 == 255) {
                chart.trills[CurrentTrill].lane1 =
                    PlasticFrets[GetEventLane(Difficulty, event)];
            } else if (chart.trills[CurrentTrill].lane2 == 255) {
                chart.trills[CurrentTrill].lane2 =
                    PlasticFrets[GetEventLane(Difficulty, event)];
            }
        }
    }
    if (!chart.overdrive.empty()) {
        if (event.tick >= chart.overdrive[CurrentOverdrive].StartTick && event.tick < chart.overdrive[CurrentOverdrive].StartTick + chart.overdrive[CurrentOverdrive].TickLength) {
            chart.overdrive[CurrentOverdrive].NoteCount++;
        }
    }
}

void Encore::RhythmEngine::GuitarLoader::GetNoteModifiers(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        CheckSysEx(event);
        CheckHopos(event);
        CheckTaps(event);
        // can have events for things like overdrive here
        // ugh
        // can i just have notes work for now
    }
}

void Encore::RhythmEngine::GuitarLoader::CheckModifiers(const smf::MidiEvent &event) {
    ITERATE_MODIFIER_BY_NOTE(ForceHopoOn, event)
    ITERATE_MODIFIER_BY_NOTE(ForceHopoOff, event)
    ITERATE_MODIFIER_BY_NOTE(TapMarker, event)
    ITERATE_MODIFIER_BY_NOTE(OpenMarker, event)
}

void Encore::RhythmEngine::GuitarLoader::GetNotes(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        // im tired boss
        CheckEvents(event);
        CheckModifiers(event);
        if (IsInPitchRangeGB(Difficulty, event) && event.isNoteOn()) {
            if (chart[0].empty()) {
                chart.BaseScore += BASE_SCORE_NOTE_POINT;
                CreateNote(event);
            } else if (chart[0].back().StartTicks == event.tick) {
                chart.BaseScore += BASE_SCORE_NOTE_POINT;
                chart[0].back().Lane += PlasticFrets[GetEventLane(Difficulty, event)];
                if (chart[0].back().NoteType == 1) {
                    chart[0].back().NoteType = 0;
                }
                if (!chart.trills.empty()) {
                    if (event.tick >= chart.trills[CurrentTrill].StartTick) {
                        //chart.rolls[CurrentRoll].lane +=
                         //   PlasticFrets[GetEventLane(Difficulty, event)];
                        //FIXME
                    }
                }
                if (chart[0].back().LengthTicks > 0) {
                    chart.BaseScore += (chart[0].back().LengthTicks / 480.0f) * (SUSTAIN_POINTS_PER_BEAT * BASE_SCORE_NOTE_MULT);
                }
                if (!ForceHopoOn.empty()) {
                    if (ForceHopoOn.front().first <= event.tick) {
                        chart[0].back().NoteType = 1;
                    }
                }

            } else {
                chart.BaseScore += BASE_SCORE_NOTE_POINT;
                CreateNote(event);
            }
        }
    }
}
