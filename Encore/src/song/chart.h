#pragma once
#include <vector>
#include <string>
#include "midifile/MidiFile.h"
// #include "song.h"
#include "timingvalues.h"
#include <atomic>
#include <algorithm>

class Note
{
public:
	double time;
	double len;
	double beatsLen;
	double heldTime=0.0;
	double sustainThreshold = 0.2;
    double HitOffset = 0.0;
	int lane;
	bool lift = false;
	bool hit = false;
	bool held = false;
	bool valid = false;
	bool miss = false;
	bool accounted = false;
    bool countedForSolo = false;
	bool countedForSection = false;
	bool countedForODPhrase = false;
    bool perfect = false;
	bool renderAsOD = false;
    double hitTime = 0;
    int tick;

	// CLASSIC
    // 0-4 for grybo, helps with chords
    int strumCount = 0;
    int chordSize = 0;
    bool hitWithFAS = false;
    int mask;
    bool chord = false;
    std::vector<int> pLanes;
	bool pForceOn = false;
	bool pForceOff = false;
    bool phopo = false;
	bool extendedSustain = false;
	bool pDrumTom = false;
	bool pSnare = false;
	bool pDrumAct = false;
	int GhostCount = 0;
	bool Ghosted = false;

	bool isGood(double eventTime, double inputOffset) const {
		return (time - goodBackend + inputOffset < eventTime &&
			time + goodFrontend + inputOffset > eventTime);
	}
	bool isPerfect(double eventTime, double inputOffset) {
		return (time - perfectBackend + inputOffset < eventTime &&
			time + perfectFrontend + inputOffset > eventTime);
	}

	bool pTap = false;
	bool pOpen = false;
};

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

struct solo {
    double start;
    double end;
    int noteCount = 0;
    int notesHit = 0;
    bool perfect = false;
};

struct tapPhrase {
	double start;
	double end;
};

struct forceOnPhrase {
    double start;
    double end;
};

struct forceOffPhrase {
    double start;
    double end;
};

struct DrumFill
{
	double start;
	double end;
	int noteCount = 0;
	int notesHit = 0;
	bool ready = false;
};

struct odPhrase 
{
	double start;
	double end;
	int noteCount = 0;
	int notesHit = 0;
	bool missed = false;
	bool added = false;
};

struct section {
	int tickStart;
	int tickEnd;
	double Start;
	double End;
	int totalNotes = 0;
	int notesHit = 0;
	std::string Name;
};

class Chart 
{
private:
	std::vector<std::vector<int>> diffNotes = { {60,63,66,69}, {72,75,78,81}, {84,87,90,93}, {96,100,102,106} };
    int soloNote = 101;

    std::vector<std::vector<int>> pDiffNotes = { {60,61,62,63,64}, {72,73,74,75,76}, {84,85,86,87,88}, {96,97,98,99,100} };
    int pSoloNote = 103;
    int pForceOn = 101;
	int pTapNote = 104;
    int pForceOff = 102;
    static bool compareNotes(const Note& a, const Note& b) {
        return a.time < b.time;
    }
	static bool compareNotesTL(const Note& a, const Note& b) {
    	if (a.time == b.time) {
    		return a.lane < b.lane;
    	}
    	return a.time < b.time;
    }
    static bool areNotesEqual(const Note& a, const Note& b) {
        return a.tick == b.tick;
    }
public:
	std::vector<section> Sections{};
	void getSections(smf::MidiFile& midiFile, int trkidx) {
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
				if (Name.substr(0,5) == "[prc_") {
					newSection.tickStart = tick;
					newSection.Start = time;
					newSection.Name = Name.substr(5);
					newSection.Name.pop_back();
					std::cout << "New section: " << newSection.Name << " at " << newSection.Start << std::endl;
					if (Section > 0) {
						Sections[Section-1].End = time;
						Sections[Section-1].tickEnd = tick;
					}
					Section++;
					Sections.push_back(std::move(newSection));
				}
				else if (Name.substr(0,9) == "[section ") {
					newSection.tickStart = tick;
					newSection.Start = time;
					newSection.Name = Name.substr(9);
					newSection.Name.pop_back();
					std::cout << "New section: " << newSection.Name << " at " << newSection.Start << std::endl;
					if (Section > 0) {
						Sections[Section-1].End = time;
						Sections[Section-1].tickEnd = tick;
					}
					Section++;
					Sections.push_back(std::move(newSection));

				}
			}
		}
	}
	bool valid = false;
    std::vector<int> PlasticFrets = {
    					// open			0		     0| technically not a "fretted note" so i put it on the empty space
        0b000001, // green				1		    0 |
        0b000010, // red				2		   0  |
    					// gr chord		3		   00 |
        0b000100, // yellow				4		  0   |
    					// gy chord		5		  0 0 |
    					// ry chord		6		  00  |
    					// gry chord	7		  000 |
        0b001000, // blue				8		 0    |
    					// gb chord		9		 0  0 |
						// rb chord		10		 0 0  |
						// grb chord	11		 0 00 |
    					// yb chord		12		 00   |
						// gyb chord	13		 00 0 |
						// ryb chord	14		 000  |
    					// gryb chord	15		 0000 |
        0b010000  // orange				16		0     |
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
    };


    bool plastic = false;
    // plastic stuff :tm:
    // note: clones dont do thresh. they do 12ths
    int hopoThreshold = 170;

	std::vector<Note> notes;
	std::vector <std::vector<int>> notes_perlane{ {},{},{},{},{} };
	int baseScore = 0;
	int findNoteIdx(double time, int lane) {
        // if i is smaller than the amount of notes
		for (int i = 0; i < notes.size();i++) {
            // if a note exists at the time given, and is in the same lane, return that note's value
			if (notes[i].time == time && notes[i].lane == lane)
				return i;
		}
		return -1;
	}

	int diff = -1;
    std::vector<Note> notesPre;

    int findNotePreIdx(double time, int lane) {
        // if i is smaller than the amount of notes
        for (int i = 0; i < notesPre.size();i++) {
            // if a note exists at the time given, and is in the same lane, return that note's value
            if (notesPre[i].time == time && notesPre[i].lane == lane)
                return i;
        }
        return -1;
    }

    std::vector<int> findPNoteIdx(double time) {
        std::vector<int> sameNotes;
        // if i is smaller than the amount of notes
        for (int i = 0; i < notesPre.size();i++) {
            // if a note exists at the time given, and is in the same lane, return that note's value
            if (notesPre[i].time == time)
                sameNotes.push_back(i);
        } if (sameNotes.empty()) {
            sameNotes.push_back(-1);
        }
        return sameNotes;
    }


    std::vector<forceOnPhrase> forcedOnPhrases;
	std::vector<tapPhrase> tapPhrases;
    std::vector<forceOffPhrase> forcedOffPhrases;
	std::vector<odPhrase> odPhrases;
    std::vector<solo> Solos;
	std::vector<DrumFill> fills;
    int resolution = 480;
	void parseNotes(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events, int diff, int instrument);
    void parsePlasticNotes(smf::MidiFile& midiFile, int trkidx, int diff, int instrument, int hopoThresh);
	void parseSection(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events, int diff, int instrument);
	void parsePlasticSection(smf::MidiFile& midiFile, int trkidx, int start, int end, int diff, int instrument, int hopoThresh);
	void parsePlasticDrums(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events, int diff, int instrument, bool proDrums, bool doubleKick);

	void resetNotes() {
		notes.clear();
		notesPre.clear();
		odPhrases.clear();
		Solos.clear();
		fills.clear();
		tapPhrases.clear();
		forcedOnPhrases.clear();
		forcedOffPhrases.clear();
	}
};


