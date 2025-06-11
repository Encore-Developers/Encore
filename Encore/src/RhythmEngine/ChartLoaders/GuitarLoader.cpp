//
// Created by maria on 17/05/2025.
//

#include "GuitarLoader.h"
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
            OpenMarker.emplace(event.tick);
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
    if (!chart.empty()) {
        if (chart[0].back().StartTicks + 170 >= event.tick) {
            return 1;
        }
    }
    return 0;
}
void Encore::RhythmEngine::GuitarLoader::CheckEvents(const smf::MidiEvent &event) {
    if (!chart.solos.empty()) {
        if (CurrentSolo < chart.solos.size() - 1
            && chart.solos[CurrentSolo].EndTick < event.tick)
            CurrentSolo++;
    }

    if (!chart.overdrive.empty()) {
        if (CurrentOverdrive < chart.overdrive.size() - 1
            && chart.overdrive[CurrentOverdrive].EndTick < event.tick)
            CurrentOverdrive++;
    }
}
void Encore::RhythmEngine::GuitarLoader::GetChartEvents(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        if (event[1] == 116 && event.isNoteOn()) {
            chart.overdrive.push_back(
                { event.tick,
                event.seconds,
                event.getLinkedEvent()->tick - event.tick,
                  event.getLinkedEvent()->seconds - event.seconds }
            );
        }
        if (event[1] == 103 && event.isNoteOn()) {
            chart.solos.push_back(
                { event.tick,
                event.seconds,
                event.getLinkedEvent()->tick - event.tick,
                  event.getLinkedEvent()->seconds - event.seconds }
            );
        }
    }
}
void Encore::RhythmEngine::GuitarLoader::CreateNote(const smf::MidiEvent &event) {
    chart[0].push_back(
        { event.tick,
          event.getLinkedEvent()->tick - event.tick,
          event.seconds,
          event.getLinkedEvent()->seconds - event.seconds,
          GetNoteType(event),
          PlasticFrets[GetEventLane(Difficulty, event)] }
    );
    if (!chart.solos.empty()) {
        if (event.tick >= chart.solos[CurrentSolo].StartTick) {
            chart.solos[CurrentSolo].NoteCount++;
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
    if (!ForceHopoOn.empty()) {
        if (ForceHopoOn.front().second <= event.tick)
            ForceHopoOn.pop();
    }
    if (!ForceHopoOff.empty()) {
        if (ForceHopoOff.front().second <= event.tick)
            ForceHopoOff.pop();
    }
    if (!TapMarker.empty()) {
        if (TapMarker.front().second <= event.tick)
            TapMarker.pop();
    }
}

void Encore::RhythmEngine::GuitarLoader::GetNotes(smf::MidiEventList track) {
    track.linkNotePairs();
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        // im tired boss
        CheckEvents(event);
        CheckModifiers(event);
        if (IsInPitchRange(Difficulty, event) && event.isNoteOn()) {
            if (chart[0].empty()) {
                CreateNote(event);
                continue;
            }
            if (chart[0].back().StartTicks == event.tick) {
                chart[0].back().Lane += PlasticFrets[GetEventLane(Difficulty, event)];
            } else {
                CreateNote(event);
            }
        }
    }
}
