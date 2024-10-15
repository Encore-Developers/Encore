#pragma once
#include <vector>
#include <string>
#include "midifile/MidiFile.h"
// #include "song.h"
#include "util/enclog.h"
#include "raylib.h"
#include "events/EncEventVects/EventVectors.h"

#include <atomic>
#include <algorithm>
#include <iso646.h>

constexpr uint8_t PlasticFrets[6] {
    // open			0		     0| technically not a "fretted note" so i put it on
    // the empty space
    0b000001,
    // green		1		    0 |
    0b000010,
    // red			2		   0  |
    // gr chord		3		   00 |
    0b000100,
    // yellow		4		  0   |
    // gy chord		5		  0 0 |
    // ry chord		6		  00  |
    // gry chord	7		  000 |
    0b001000,
    // blue			8		 0    |
    // gb chord		9		 0  0 |
    // rb chord		10		 0 0  |
    // grb chord	11		 0 00 |
    // yb chord		12		 00   |
    // gyb chord	13		 00 0 |
    // ryb chord	14		 000  |
    // gryb chord	15		 0000 |
    0b010000,
    // orange		16		0     |
    // go chord		17		0   0 |
    // ro chord		18		0  0  |
    // gro chord	19		0  00 |
    // yo chord		20		0 0   |
    // gyo chord	21		0 0 0 |
    // ryo chord	22		0 00  |
    // gryo chord	23		0 000 |
    // bo chord		24		00    |
    // gbo chord	25		00  0 |
    // rbo chord	26		00 0  |
    // grbo chord	27		00 00 |
    // ybo chord	28		000   |
    // gybo chord	29		000 0 |
    // rybo chord	30		0000  |
    // grybo chord	31		00000 |
    0b000000
};


/*
inline uint8_t LaneToPlasticFret(int lane) {
    switch (lane) {
    case 0: return PlasticFrets::Green;
    case 1: return PlasticFrets::Red;
    case 2: return PlasticFrets::Yellow;
    case 3: return PlasticFrets::Blue;
    case 4: return PlasticFrets::Orange;
    default: return PlasticFrets::Green;
    }
}
*/

enum ChartLoadingState {
    BEATLINES,
    NOTE_PARSING,
    NOTE_SORTING,
    PLASTIC_CALC,
    NOTE_MODIFIERS,
    OVERDRIVE,
    SOLOS,
    BASE_SCORE,
    EXTRA_PROCESSING,
    READY
};

inline std::atomic<int> CurrentChart = -1;
inline std::atomic<int> LoadingState = -1;


class Chart {
private:
    std::vector<std::vector<int> > diffNotes = {
        { 60, 63, 66, 69 }, { 72, 75, 78, 81 }, { 84, 87, 90, 93 }, { 96, 100, 102, 106 }
    };
    int soloNote = 101;

    std::vector<std::vector<int> > pDiffNotes = { { 60, 61, 62, 63, 64 },
                                                  { 72, 73, 74, 75, 76 },
                                                  { 84, 85, 86, 87, 88 },
                                                  { 96, 97, 98, 99, 100 } };
    int pSoloNote = 103;
    int pForceOn = 101;
    int pTapNote = 104;
    int pForceOff = 102;
    static bool compareNotes(const Note &a, const Note &b) { return a.time < b.time; }
    static bool compareNotesTL(const Note &a, const Note &b) {
        if (a.time == b.time) {
            return a.lane < b.lane;
        }
        return a.time < b.time;
    }
    static bool areNotesEqual(const Note &a, const Note &b) { return a.tick == b.tick; }

public:

    std::vector<Note> notes;

    // this is plastic shit that should probably be put deeper as its really only used
    // for chart
    std::vector<Note> notesPre;
    std::vector<forceOnPhrase> forcedOnPhrases;
    std::vector<tapPhrase> tapPhrases;
    std::vector<forceOffPhrase> forcedOffPhrases;
    std::vector<openMarker> openMarkers;
    int track = 0;
    // std::vector<section> Sections {};
    // std::vector<odPhrase> odPhrases;
    // std::vector<solo> Solos;
    // std::vector<DrumFill> fills;
    SoloEvents solos;
    ODEvents overdrive;
    FillEvents fills;
    SectionEvents sections;

    void getSections(smf::MidiFile &midiFile, int trkidx) {
        int Section = 0;
        smf::MidiEventList events = midiFile[trkidx];
        for (int i = 0; i < events.getSize(); i++) {
            if (events[i].isMeta() && (int)events[i][1] == 1) {
                double time = midiFile.getTimeInSeconds(trkidx, i);
                int tick = midiFile.getAbsoluteTickTime(time);
                section newSection;
                std::string Name;
                for (int k = 3; k < events[i].getSize(); k++) {
                    Name += events[i][k];
                }
                if (Name.substr(0, 5) == "[prc_") {
                    newSection.StartTick = tick;
                    newSection.StartSec = time;
                    newSection.Name = Name.substr(5);
                    newSection.Name.pop_back();
                    Encore::EncoreLog(LOG_DEBUG, TextFormat("New section: %s at %5.4f", newSection.Name.c_str(), newSection.StartSec));

                    if (Section > 0) {
                        sections.events[Section - 1].EndSec = time;
                        sections.events[Section - 1].EndTick = tick;
                    }
                    Section++;
                    sections.events.push_back(std::move(newSection));
                } else if (Name.substr(0, 9) == "[section ") {
                    newSection.StartTick = tick;
                    newSection.StartSec = time;
                    newSection.Name = Name.substr(9);
                    newSection.Name.pop_back();
                    Encore::EncoreLog(LOG_DEBUG, TextFormat("New section: %s at %5.4f", newSection.Name.c_str(), newSection.StartSec));

                    if (Section > 0) {
                        sections.events[Section - 1].EndSec = time;
                        sections.events[Section - 1].EndTick = tick;
                    }
                    Section++;
                    sections.events.push_back(std::move(newSection));
                }
            }
        }
    }
    bool valid = false;


    bool plastic = false;
    // plastic stuff :tm:
    // note: clones dont do thresh. they do 12ths
    int hopoThreshold = 170;


    std::vector<std::vector<int> > notes_perlane { {}, {}, {}, {}, {} };
    int baseScore = 0;
    int findNoteIdx(double time, int lane) {
        // if i is smaller than the amount of notes
        for (int i = 0; i < notes.size(); i++) {
            // if a note exists at the time given, and is in the same lane, return that
            // note's value
            if (notes[i].time == time && notes[i].lane == lane)
                return i;
        }
        return -1;
    }

    int diff = -1;


    int findNotePreIdx(double time, int lane) {
        // if i is smaller than the amount of notes
        for (int i = 0; i < notesPre.size(); i++) {
            // if a note exists at the time given, and is in the same lane, return that
            // note's value
            if (notesPre[i].time == time && notesPre[i].lane == lane)
                return i;
        }
        return -1;
    }

    std::vector<int> findPNoteIdx(double time) {
        std::vector<int> sameNotes;
        // if i is smaller than the amount of notes
        for (int i = 0; i < notesPre.size(); i++) {
            // if a note exists at the time given, and is in the same lane, return that
            // note's value
            if (notesPre[i].time == time)
                sameNotes.push_back(i);
        }
        if (sameNotes.empty()) {
            sameNotes.push_back(-1);
        }
        return sameNotes;
    }


    int resolution = 480;
    void parseNotes(
        smf::MidiFile &midiFile,
        int trkidx,
        smf::MidiEventList events,
        int diff,
        int instrument
    );
    void parsePlasticNotes(
        smf::MidiFile &midiFile, int trkidx, int diff, int instrument, int hopoThresh
    );
    void parseSection(
        smf::MidiFile &midiFile,
        int trkidx,
        smf::MidiEventList events,
        int diff,
        int instrument
    );
    void parsePlasticSection(
        smf::MidiFile &midiFile,
        int trkidx,
        int start,
        int end,
        int diff,
        int instrument,
        int hopoThresh
    );
    void parsePlasticDrums(
        smf::MidiFile &midiFile,
        int trkidx,
        smf::MidiEventList events,
        int diff,
        int instrument,
        bool proDrums,
        bool doubleKick
    );

    void resetNotes() {
        notes.clear();
        notesPre.clear();

        sections.ResetEvents();
        overdrive.ResetEvents();
        solos.ResetEvents();
        fills.ResetEvents();

        tapPhrases.clear();
        forcedOnPhrases.clear();
        forcedOffPhrases.clear();
    }

    void restartNotes() {
        for (auto &note : notes) {
            note.accounted = false;
            note.miss = false;
            note.hit = false;
            note.perfect = false;
            note.heldTime = 0.0;
            note.HitOffset = 0.0;
            note.held = false;
            note.hitTime = 0.0;
            note.strumCount = 0;
            note.GhostCount = 0;
            note.hitWithFAS = false;
            note.countedForODPhrase = false;
            note.countedForSection = false;
            note.countedForSolo = false;
            note.Ghosted = false;
        }
        notesPre.clear();
        sections.ResetEvents();
        overdrive.ResetEvents();
        solos.ResetEvents();
        fills.ResetEvents();
    }
};
