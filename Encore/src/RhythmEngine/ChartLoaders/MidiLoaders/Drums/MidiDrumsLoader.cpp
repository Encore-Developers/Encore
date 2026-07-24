//
// Created by maria on 17/05/2025.
//

#include "MidiDrumsLoader.h"

#include "RhythmEngine/REenums.h"
#include <regex>

#include "RhythmEngine/scoring.h"
// i hate sysex

void Encore::RhythmEngine::MidiDrumsLoader::CheckToms(const smf::MidiEvent &event) {
    if (event.isNoteOn()) {
        if (event[1] == 110) {
            YellowTom.emplace(event.tick, event.getLinkedEvent()->tick);
        } else if (event[1] == 111) {
            BlueTom.emplace(event.tick, event.getLinkedEvent()->tick);
        } else if (event[1] == 112) {
            GreenTom.emplace(event.tick, event.getLinkedEvent()->tick);
        };
        ;
    }
}

[[nodiscard]] Encore::RhythmEngine::NoteEvent::NoteType
Encore::RhythmEngine::MidiDrumsLoader::GetNoteType(const smf::MidiEvent &event) {
    if (GetEventLane(Difficulty, event) == 4) {
        if (!GreenTom.empty()) {
            if (GreenTom.front().first <= event.tick) {
                return NoteEvent::NORMAL;
            }
        }
    }
    if (GetEventLane(Difficulty, event) == 3) {
        if (!BlueTom.empty()) {
            if (BlueTom.front().first <= event.tick) {
                return NoteEvent::NORMAL;
            }
        }
    }
    if (GetEventLane(Difficulty, event) == 2) {
        if (!YellowTom.empty()) {
            if (YellowTom.front().first <= event.tick) {
                return NoteEvent::NORMAL;
            }
        }
    }
    if (GetEventLane(Difficulty, event) <= 1) {
        return NoteEvent::NORMAL;
    }
    return NoteEvent::CYMBAL;
}
void Encore::RhythmEngine::MidiDrumsLoader::CheckEvents(const smf::MidiEvent &event) {
    ITERATE_EVENT_BY_NOTE(solos, CurrentSolo, event)
    if (!chart.overdrive.empty()) {
        if (CurrentOverdrive < chart.overdrive.size() - 1
            && chart.overdrive[CurrentOverdrive].end.tick <= event.tick)
            CurrentOverdrive++;
    }
    if (!chart.sections.empty()) {
        if (CurrentSection < chart.sections.size() - 1 && event.tick >= chart.sections[CurrentSection+1].tickStart)
            CurrentSection++;
    }
    // ITERATE_EVENT_BY_NOTE(overdrive, CurrentOverdrive, event)
}
void Encore::RhythmEngine::MidiDrumsLoader::GetChartEvents(smf::MidiEventList &track) {
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        ATTEMPT_TO_ADD_CHART_EVENT(116, overdrive, event);
        ATTEMPT_TO_ADD_CHART_EVENT(103, solos, event);
        ATTEMPT_TO_ADD_CHART_EVENT(127, trills, event);
        ATTEMPT_TO_ADD_CHART_EVENT(126, rolls, event);

    }
}
void Encore::RhythmEngine::MidiDrumsLoader::CreateNote(const smf::MidiEvent &event) {
    Event pNote {event.seconds, event.tick, event.getLinkedEvent()->seconds, event.getLinkedEvent()->tick};
    int lane = GetEventLane(Difficulty, event);
    if (pNote.tickLen() < Resolution * 0.3541) {
        pNote.end.tick = 0;
        pNote.end.sec = 0;
    }
    int type = GetNoteType(event);
    if (!DiscoFlip.empty()) {
        if (DiscoFlip.front().first <= event.tick) {
            if (lane == 1) {
                lane = 2;
                type = NoteEvent::CYMBAL;
            } else if (lane == 2) {
                lane = 1;
                type = NoteEvent::NORMAL;
            }
        }
    }
    chart.BaseScore += BASE_SCORE_NOTE_POINT * maxMult;
    chart[lane].emplace_back(
        pNote.start, pNote.end, type, PlasticFrets[lane]
    );

    if (!chart.sections.empty()) {
        int end = midiFile->getFileDurationInTicks();
        if (CurrentSection < chart.sections.size() - 1) {
            end = chart.sections.at(CurrentSection + 1).tickStart;
        }
        if (event.tick >= chart.sections.at(CurrentSection).tickStart && event.tick < end)
            chart.sections.at(CurrentSection).notes++;
    }
    if (!chart.solos.empty())
        chart.solos[CurrentSolo].CountNote(event.tick);
    if (!chart.overdrive.empty())
        chart.overdrive[CurrentOverdrive].CountNote(event.tick);
}

void Encore::RhythmEngine::MidiDrumsLoader::GetNoteModifiers(smf::MidiEventList &track) {
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        CheckToms(event);
        if (event.isMeta() && event[1] == 1) {
            std::string eventName;
            for (int k = 3; k < event.getSize(); k++) {
                eventName += event[k];
            }
            //for (int i = 0; i < event[2]; i++) {
            //    eventName.append(std::to_string(event[i+3]));
            //}

            static const std::regex drumsMixRegex("\\[?mix (\\d) drums(\\d)(d|(dnoflip)|(easy)|(easynokick))?\\]?$");
            std::smatch drumsMix;
            std::regex_match(eventName, drumsMix, drumsMixRegex);
            if (drumsMix[3] == 'd' && drumsMix[1] == '3') {
                DiscoFlip.emplace(event.tick, 0);
                // do a disco flip
            } else if (drumsMix[3].matched == false && drumsMix[1] == '3') {
                if (!DiscoFlip.empty())
                    DiscoFlip.back().second = event.tick;
            }
        }
        // can have events for things like overdrive here
        // ugh
        // can i just have notes work for now
    }
}

void Encore::RhythmEngine::MidiDrumsLoader::CheckModifiers(const smf::MidiEvent &event) {
    ITERATE_MODIFIER_BY_NOTE(BlueTom, event)
    ITERATE_MODIFIER_BY_NOTE(YellowTom, event)
    ITERATE_MODIFIER_BY_NOTE(GreenTom, event)
    ITERATE_MODIFIER_BY_NOTE(DiscoFlip, event)
}

void Encore::RhythmEngine::MidiDrumsLoader::GetNotes(smf::MidiEventList &track) {
    for (int eventInt = 0; eventInt < track.size(); eventInt++) {
        smf::MidiEvent &event = track[eventInt];
        // im tired boss
        if (event[0] == 255)
            continue;
        CheckEvents(event);
        CheckModifiers(event);
        if (IsInPitchRange(Difficulty, event) && event.isNoteOn()) {
            CreateNote(event);
        }
    }
}
