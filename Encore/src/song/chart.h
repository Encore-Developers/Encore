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
	bool valid = false;
    std::vector<int> PlasticFrets = {
        0b000001, // green
        0b000010, // red
        0b000100, // yellow
        0b001000, // blue
        0b010000  // orange
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
    void parsePlasticNotes(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events, int diff, int instrument, int hopoThresh);
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


