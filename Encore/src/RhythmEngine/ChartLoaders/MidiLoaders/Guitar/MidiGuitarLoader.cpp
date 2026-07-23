//
// Created by maria on 17/05/2025.
//

#include "MidiGuitarLoader.h"

#include "RhythmEngine/REenums.h"
#include "RhythmEngine/scoring.h"
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
void Encore::RhythmEngine::MidiGuitarLoader::SysExTap(const smf::MidiEvent &event) {
    if (IsSysExTap(event)) {
        if (IsSEXOn(event)) {
            TapMarker.emplace(event.tick, 0);
        } else if (IsSEXOff(event)) {
            TapMarker.back().second = event.tick;
        }
    }
}
void Encore::RhythmEngine::MidiGuitarLoader::SysExOpen(const smf::MidiEvent &event) {
    if (IsSysExOpen(event)) {
        if (IsSEXOn(event)) {
            OpenMarker.emplace(event.tick, 0);
        } else if (IsSEXOff(event)) {
            OpenMarker.back().second = event.tick;
        }
    }
}
void Encore::RhythmEngine::MidiGuitarLoader::CheckSysEx(const smf::MidiEvent &event) {
    if (IsSysEx(Difficulty, event)) {
        SysExTap(event);
        SysExOpen(event);
    }
}

void Encore::RhythmEngine::MidiGuitarLoader::CheckHopos(const smf::MidiEvent &event) {
    if (!event.isNoteOn()) return;
    assert("Difficulty is too high for hopo check" && Difficulty < 4 && Difficulty > -1);
    if (event[1] == HopoFlags[Difficulty].second) {
        int endTick = event.tick;
        if (event.getLinkedEvent()) endTick = event.getLinkedEvent()->tick;
        ForceHopoOn.emplace(event.tick, endTick);
        return;
    }
    if (event[1] == HopoFlags[Difficulty].first) {
        int endTick = event.tick;
        if (event.getLinkedEvent()) endTick = event.getLinkedEvent()->tick;
        ForceHopoOff.emplace(event.tick, endTick);
    };
}
void Encore::RhythmEngine::MidiGuitarLoader::CheckTaps(const smf::MidiEvent &event) {
    if (event.isNoteOn()) {
        if (event[1] == 104) {
            TapMarker.emplace(event.tick, event.getLinkedEvent()->tick);
        };
    }
}
[[nodiscard]] Encore::RhythmEngine::NoteEvent::NoteType
Encore::RhythmEngine::MidiGuitarLoader::GetNoteType(const smf::MidiEvent &event) {
    if (!TapMarker.empty()) {
        if (TapMarker.front().first <= event.tick) {
            return NoteEvent::TAP;
        }
    }
    if (!ForceHopoOn.empty()) {
        if (ForceHopoOn.front().first <= event.tick) {
            return NoteEvent::HOPO;
        }
    }
    if (!ForceHopoOff.empty()) {
        if (ForceHopoOff.front().first <= event.tick) {
            return NoteEvent::NORMAL;
        }
    }
    if (!chart[0].empty()) {
        auto& note = chart[0].back();
        // DUDE ARE YOU A FUCKING MORON
        // HOPOS HAVE SPECIFIC REQURIEMENTS TO BE HOPOS, NOT JUST "its close enough"
        // GOD FUCKING DAMN
        // ALSO THAT IF STATEMENT IS FUCKING UTTERLY USELESS
        // oh my god i might DIE what the FUCK
        if (note.lane != PlasticFrets[GetEventLane(Difficulty, event)]) {
            if (note.start.tick + Threshold >= event.tick && !(PlasticFrets[GetEventLane(Difficulty, event)] & note.lane)) {
                return NoteEvent::HOPO;
            }
        }
    }
    return NoteEvent::NORMAL;
}
void Encore::RhythmEngine::MidiGuitarLoader::CheckEvents(const smf::MidiEvent &event) {
    ITERATE_EVENT_BY_NOTE(solos, CurrentSolo, event)
    if (!chart.overdrive.empty()) {
        if (CurrentOverdrive < chart.overdrive.size() - 1
            && chart.overdrive[CurrentOverdrive].end.tick <= event.tick)
            CurrentOverdrive++;
    }
    // ITERATE_EVENT_BY_NOTE(overdrive, CurrentOverdrive, event)
    // oh god.
    if (!chart.sections.empty()) {
        if (CurrentSection < chart.sections.size() - 1 && event.tick >= chart.sections[CurrentSection+1].tickStart)
            CurrentSection++;
    }
    ITERATE_EVENT_BY_NOTE(trills, CurrentTrill, event)
    ITERATE_EVENT_BY_NOTE(rolls, CurrentRoll, event)
}
void Encore::RhythmEngine::MidiGuitarLoader::GetChartEvents(smf::MidiEventList &track) {
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        ATTEMPT_TO_ADD_CHART_EVENT(116, overdrive, event);
        ATTEMPT_TO_ADD_CHART_EVENT(103, solos, event);
        ATTEMPT_TO_ADD_CHART_EVENT(127, trills, event);
        ATTEMPT_TO_ADD_CHART_EVENT(126, rolls, event);
    }
}

double SustainBaseScore(double tickLen, double Resolution, double maxMult) {
    return (tickLen / Resolution) * BASE_SCORE_SUSTAIN_POINTS * maxMult;
};

void Encore::RhythmEngine::MidiGuitarLoader::CreateNote(const smf::MidiEvent &event) {
    Event pNote {event.seconds, event.tick, event.getLinkedEvent()->seconds, event.getLinkedEvent()->tick};
    uint8_t lane = PlasticFrets[GetEventLane(Difficulty, event)];
    if (pNote.tickLen() < Resolution * 0.3541) {
        pNote.end.tick = 0;
        pNote.end.sec = 0;
    } else {
        chart.BaseScore += SustainBaseScore(pNote.tickLen(), Resolution, maxMult);
        pNote.end.tick -= 1;
    }

    chart.BaseScore += BASE_SCORE_NOTE_POINT * maxMult;
    if (!OpenMarker.empty()) {
        if (OpenMarker.front().first <= event.tick) {
            lane = 0;
        }
    }

    if (!chart[0].empty()) {
        int sustainChopThreshold = (midiFile->getTPQ() / 16);
        for (auto note = chart[0].end()--;
                note->end.tick >= pNote.start.tick - sustainChopThreshold;
                --note) {
            int noteEnd = note->end.tick;
            if (note->tickLen() > 0 && noteEnd >= event.tick - sustainChopThreshold && noteEnd <= event.tick + 1) {
                note->end.tick = event.tick - (sustainChopThreshold * 4);
                note->end.sec = midiFile->getTimeInSeconds(note->end.tick);
            }
        }
    }

    chart[0].emplace_back(
        pNote.start,
        pNote.end,
        GetNoteType(event),
        lane
    );
    if (!chart.sections.empty()) {
        int end = midiFile->getFileDurationInTicks();
        if (CurrentSection < chart.sections.size() - 1) {
            end = chart.sections.at(CurrentSection + 1).tickStart;
        }
        if (event.tick >= chart.sections.at(CurrentSection).tickStart && event.tick < end)
            chart.sections.at(CurrentSection).notes++;
    }
    if (!chart.trills.empty()) {
        if (event.tick >= chart.trills[CurrentTrill].start.tick) {
            if (chart.trills[CurrentTrill].lane1 == 255) {
                chart.trills[CurrentTrill].lane1 =
                    PlasticFrets[GetEventLane(Difficulty, event)];
            } else if (chart.trills[CurrentTrill].lane2 == 255) {
                chart.trills[CurrentTrill].lane2 =
                    PlasticFrets[GetEventLane(Difficulty, event)];
            }
        }
    }

    if (!chart.solos.empty())
        chart.solos[CurrentSolo].CountNote(event.tick);
    if (!chart.overdrive.empty())
        chart.overdrive[CurrentOverdrive].CountNote(event.tick);

}

void Encore::RhythmEngine::MidiGuitarLoader::GetNoteModifiers(smf::MidiEventList &track) {
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

void Encore::RhythmEngine::MidiGuitarLoader::CheckModifiers(const smf::MidiEvent &event) {
    ITERATE_MODIFIER_BY_NOTE(ForceHopoOn, event)
    ITERATE_MODIFIER_BY_NOTE(ForceHopoOff, event)
    ITERATE_MODIFIER_BY_NOTE(TapMarker, event)
    ITERATE_MODIFIER_BY_NOTE(OpenMarker, event)
}

void Encore::RhythmEngine::MidiGuitarLoader::GetNotes(smf::MidiEventList &track) {
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        // im tired boss
        CheckEvents(event);
        CheckModifiers(event);
        if (IsInPitchRangeGB(Difficulty, event) && event.isNoteOn()) {
            if (chart[0].empty()) {
                CreateNote(event);
                continue;
            }
            auto& note = chart[0].back();
            if (note.start.tick == event.tick) {
                chart.BaseScore += BASE_SCORE_NOTE_POINT * maxMult;
                note.lane += PlasticFrets[GetEventLane(Difficulty, event)];
                if (note.type == 1) {
                    note.type = 0;
                }
                if (!chart.trills.empty()) {
                    // if (event.tick >= chart.trills[CurrentTrill].StartTick) {
                        //chart.rolls[CurrentRoll].lane +=
                         //   PlasticFrets[GetEventLane(Difficulty, event)];
                        //FIXME
                    // }
                }
                if (note.tickLen() > 0) {
                    chart.BaseScore += SustainBaseScore(note.tickLen(), Resolution, maxMult);
                }
                if (!ForceHopoOn.empty()) {
                    if (ForceHopoOn.front().first <= event.tick) {
                        note.type = 1;
                    }
                }
                continue;
            }
            chart.BaseScore += BASE_SCORE_NOTE_POINT * maxMult;
            CreateNote(event);
        }
    }
}
