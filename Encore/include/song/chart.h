#pragma once
#include <vector>
#include <string>
#include "midifile/MidiFile.h"
// #include "song.h"
#include "game/timingvalues.h"
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
	void parseNotes(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events, int diff, int instrument) {
		std::vector<bool> notesOn{ false,false,false,false,false};
		bool odOn = false;
        bool soloOn = false;
		std::vector<double> noteOnTime{ 0.0, 0.0, 0.0, 0.0, 0.0};
		std::vector<int> noteOnTick{ 0,0,0,0,0 };
		std::vector<int> notePitches = diffNotes[diff];
		int odNote = 116;

		int curODPhrase = -1;
        int curSolo = -1;
		int curBPM = 0;
        resolution = midiFile.getTicksPerQuarterNote();
		for (int i = 0; i < events.getSize(); i++) {
			if (events[i].isNoteOn()) {
				double time = midiFile.getTimeInSeconds(trkidx, i);
				int tick = midiFile.getAbsoluteTickTime(time);
				if ((int)events[i][1] >= notePitches[0] && (int)events[i][1] <= notePitches[1]) {
					int lane = (int)events[i][1] - notePitches[0];
					if (!notesOn[lane]) {
						noteOnTime[lane] = time;
						noteOnTick[lane] = tick;
						notesOn[lane] = true;
						int noteIdx = findNoteIdx(time, lane);
						if (noteIdx != -1) {
							notes[noteIdx].valid = true;
						}
						else {
							Note newNote;
							newNote.time = time;
							newNote.lane = lane;
							newNote.valid = true;
							notes.push_back(newNote);
						}
					}
				}
				else if ((int)events[i][1] >= notePitches[2] && (int)events[i][1] <= notePitches[3]) {
					int lane = (int)events[i][1] - notePitches[2];
					int noteIdx = findNoteIdx(time, lane);
					if (noteIdx != -1) {
						notes[noteIdx].lift = true;
					}
					else {
						Note newNote;
						newNote.time = time;
						newNote.valid = false;
						newNote.lane = lane;
						newNote.lift = true;
						notes.push_back(newNote);
					}
				}
				else if ((int)events[i][1] == odNote) {
					if (!odOn) {
						odOn = true;
						odPhrase newPhrase;
						newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
						odPhrases.push_back(newPhrase);
						curODPhrase++;
						
					}
					
				} else if ((int)events[i][1] == soloNote) {
                    if (!soloOn) {
                        soloOn = true;
                        solo newSolo;
                        newSolo.start = midiFile.getTimeInSeconds(trkidx, i);
                        Solos.push_back(newSolo);
                        curSolo++;
                    }
                }
			}
			else if (events[i].isNoteOff()) {
				double time = midiFile.getTimeInSeconds(trkidx, i);
				int tick = midiFile.getAbsoluteTickTime(time);
				if ((int)events[i][1] >= notePitches[0] && (int)events[i][1] <= notePitches[1]) {
					int lane = (int)events[i][1] - notePitches[0];
					if (notesOn[lane] == true) {
						int noteIdx = findNoteIdx(noteOnTime[lane], lane);
						if (noteIdx != -1) {
							notes[noteIdx].beatsLen = (tick - noteOnTick[lane]) / (float)midiFile.getTicksPerQuarterNote();
							if (notes[noteIdx].beatsLen > 0.25) {
								notes[noteIdx].len = time - notes[noteIdx].time;
							}
							else {
								notes[noteIdx].beatsLen = 0;
								notes[noteIdx].len = 0;
							}
						}
						noteOnTick[lane] = 0;
						noteOnTime[lane] = 0;
						notesOn[lane] = false;
					}
				}
				else if ((int)events[i][1] == odNote) {
					if (odOn == true) {
						odPhrases[curODPhrase].end = time;
						odOn = false;
					}
				}
                else if ((int)events[i][1] == soloNote) {
                    if (soloOn == true) {
                        Solos[curSolo].end = time;
                        soloOn = false;
                    }
                }
			}
		}
		std::cout << "ENC: Processed base notes for " << instrument << " " << diff << std::endl;

		curODPhrase = 0;
		if (odPhrases.size() > 0) {
			LoadingState = OVERDRIVE;
			for (Note &note : notes) {
				if (note.time > odPhrases[curODPhrase].end && curODPhrase<odPhrases.size()-1)
					curODPhrase++;
				if (note.time >= odPhrases[curODPhrase].start && note.time < odPhrases[curODPhrase].end)
					odPhrases[curODPhrase].noteCount++;
			}
		}
		std::cout << "ENC: Processed overdrive for " << instrument << " " << diff << std::endl;
        curSolo = 0;
        if (Solos.size() > 0) {
        	LoadingState = SOLOS;
            for (Note &note : notes) {
                if (note.time > Solos[curSolo].end && curSolo<Solos.size()-1)
                    curSolo++;
                if (note.time >= Solos[curSolo].start && note.time <= Solos[curSolo].end)
                    Solos[curSolo].noteCount++;
            }
        }
		std::cout << "ENC: Processed solos for " << instrument << " " << diff << std::endl;
		int mult = 1;
		int multCtr = 0;
		int noteIdx = 0;
		bool isBassOrVocal = (instrument == 1 || instrument == 3);
		for (auto it = notes.begin(); it != notes.end();) {
			LoadingState = BASE_SCORE;
			Note& note = *it;
			if (!note.valid) {
				it = notes.erase(it);
			}
			else {
				baseScore += (36 * mult);
				baseScore += (note.beatsLen * 12) * mult;
				if (noteIdx == 9) mult = 2;
				else if (noteIdx == 19) mult = 3;
				else if (noteIdx == 29) mult = 4;
				else if (noteIdx == 39 && isBassOrVocal) mult = 5;
				else if (noteIdx == 49 && isBassOrVocal) mult = 6;
				noteIdx++;
				++it;
			}
		}
		std::cout << "ENC: Processed base score for " << instrument << " " << diff << std::endl;
		LoadingState = NOTE_SORTING;
        std::sort(notes.begin(), notes.end(),
                  compareNotes);
		std::cout << "ENC: Processed notes for " << instrument << " " << diff << std::endl;
	}
    void parsePlasticNotes(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events, int diff, int instrument) {
        bool odOn = false;
        bool soloOn = false;
        bool forceOn = false;
		bool tapOn = false;
        bool forceOff = false;
        std::vector<int> notePitches = pDiffNotes[diff];
        std::vector<double> noteOnTime{ 0.0, 0.0, 0.0, 0.0, 0.0};
        std::vector<int> noteOnTick{ 0,0,0,0,0 };
        std::vector<bool> notesOn{ false,false,false,false,false};

        midiFile.linkNotePairs();
        int odNote = 116;
        int curNote = -1;
        int curFOn = -1;
		int curTap = -1;
        int curFOff = -1;
        int curODPhrase = -1;
        int curSolo = -1;
        int curBPM = 0;
        resolution = midiFile.getTicksPerQuarterNote();
        if (instrument == 5 || instrument == 6) {
            for (int i = 0; i < events.getSize(); i++) {
                if (events[i].isNoteOn()) {
                    if (events[i][1] >= notePitches[0] && events[i][1] <= notePitches[4]) {
                        double time = midiFile.getTimeInSeconds(trkidx, i);
                        int tick = midiFile.getAbsoluteTickTime(time);
                        int pitch = events[i][1];
                        int lane = pitch - notePitches[0];
                        if (!notesOn[lane]) {
                            Note newNote;
                            newNote.lane = lane;
                            newNote.tick = tick;
                            newNote.time = time;
                            notesPre.push_back(newNote);
                            notesOn[lane] = true;
                            noteOnTick[lane] = tick;
                            noteOnTime[lane] = time;
                            curNote++;
                        }
                    }
					else if ((int)events[i][1] == pTapNote) {
						if (!tapOn) {
							tapPhrase newPhrase;
							newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
							tapPhrases.push_back(newPhrase);
							tapOn = true;
							curTap++;
						}
					}
                    else if ((int)events[i][1] == pForceOn) {
                        if (!forceOn) {
                            forceOnPhrase newPhrase;
                            newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
                            forcedOnPhrases.push_back(newPhrase);
                            forceOn = true;
                            curFOn++;
                        }
                    }
                    else if ((int)events[i][1] == pForceOff) {
                        if (!forceOff) {
                            forceOffPhrase newPhrase;
                            newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
                            forcedOffPhrases.push_back(newPhrase);
                            forceOff = true;
                            curFOff++;
                        }
                    }
                    else if ((int)events[i][1] == odNote) {
                        if (!odOn) {
                            odOn = true;
                            odPhrase newPhrase;
                            newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
                            odPhrases.push_back(newPhrase);
                            curODPhrase++;

                        }

                    } else if ((int)events[i][1] == pSoloNote) {
                        if (!soloOn) {
                            soloOn = true;
                            solo newSolo;
                            newSolo.start = midiFile.getTimeInSeconds(trkidx, i);
                            Solos.push_back(newSolo);
                            curSolo++;
                        }
                    }
                    } else if (events[i].isNoteOff()) {
                        double time = midiFile.getTimeInSeconds(trkidx, i);
                        int tick = midiFile.getAbsoluteTickTime(time);
                        if ((int)events[i][1] >= notePitches[0] && (int)events[i][1] <= notePitches[4]) {
                            int lane = (int)events[i][1] - notePitches[0];
                            if (notesOn[lane]) {
                                int noteIdx = findNotePreIdx(noteOnTime[lane], lane);
                                if (noteIdx != -1) {
                                    notesPre[noteIdx].beatsLen = (tick - notesPre[noteIdx].tick) / (float)midiFile.getTicksPerQuarterNote();
                                    if (notesPre[noteIdx].beatsLen > 0.25) {
                                        notesPre[noteIdx].len = time - notesPre[noteIdx].time;
                                    }
                                    else {
                                        notesPre[noteIdx].beatsLen = 0;
                                        notesPre[noteIdx].len = 0;
                                    }
                                }
                                noteOnTick[lane] = 0;
                                noteOnTime[lane] = 0;
                                notesOn[lane] = false;
                            }
                        }
						else if ((int)events[i][1] == pTapNote) {
							if (tapOn) {
								tapPhrases[curTap].end = time;
								tapOn = false;
							}
						}
                        else if ((int)events[i][1] == pForceOn) {
                            if (forceOn) {
                                forcedOnPhrases[curFOn].end = time;
                                forceOn = false;
                            }
                        }
                        else if ((int)events[i][1] == pForceOff) {
                            if (forceOff) {
                                forcedOffPhrases[curFOff].end = time;
                                forceOff = false;
                            }
                        }
                        else if ((int)events[i][1] == odNote) {
                            if (odOn) {
                                odPhrases[curODPhrase].end = time;
                                odOn = false;
                            }
                        }
                        else if ((int)events[i][1] == pSoloNote) {
                            if (soloOn) {
                                Solos[curSolo].end = time;
                                soloOn = false;
                            }
                        }
                    }
                }
        };
		std::cout << "ENC: Loaded base notes for " << instrument << " " << diff << std::endl;

            for (int i = 0; i < notesPre.size(); i++) {
            	LoadingState = PLASTIC_CALC;
                Note note = notesPre[i];
                Note newNote;
                newNote.chordSize = 1;
                newNote.mask = PlasticFrets[note.lane];
                for (Note noteMatching : notesPre) {
                    if (noteMatching.tick == note.tick && noteMatching.lane != note.lane) {
                        newNote.pLanes.push_back(noteMatching.lane);
                        newNote.mask |= PlasticFrets[noteMatching.lane];
                        newNote.chord = true;
                        newNote.chordSize++;
                    	if (noteMatching.beatsLen > note.beatsLen) {
							newNote.beatsLen = noteMatching.beatsLen;
                    	} else {
                    		newNote.beatsLen = note.beatsLen;
                    	}
                    }
                }
                newNote.pLanes.push_back(note.lane);
                newNote.tick = note.tick;
                if (notes.size() > 0) {
                    Note lastNote = notes[notes.size() - 1];
                    if (lastNote.tick >= newNote.tick - (midiFile.getTicksPerQuarterNote()/2.82) && lastNote.pLanes[0] != newNote.pLanes[0] && !newNote.chord) {
                        newNote.phopo = true;
                    }
                }
                newNote.len = note.len;
                newNote.time = note.time;
                newNote.valid = true;
                notes.push_back(newNote);

            }
			LoadingState = NOTE_SORTING;
            auto it = std::unique(notes.begin(), notes.end(), areNotesEqual);
            notes.erase(it, notes.end());
            std::sort(notes.begin(), notes.end(),
                  compareNotes);
            auto again = std::unique(notes.begin(), notes.end(), areNotesEqual);
            notes.erase(again, notes.end());
		std::cout << "ENC: Sorted notes for " << instrument << " " << diff << std::endl;
		curTap = 0;
		LoadingState = NOTE_MODIFIERS;
		if (tapPhrases.size() > 0) {
			for (Note &note : notes) {
				if (note.time > tapPhrases[curTap].end && curTap<tapPhrases.size()-1)
					curTap++;

				// std::cout << "fOn: " << forcedOnPhrases[curFOn].start << std::endl << "fOff: "
				//           << forcedOnPhrases[curFOn].end << std::endl;

				if (note.time >= tapPhrases[curTap].start && note.time < tapPhrases[curTap].end) {
					note.pTap = true;
					note.phopo = false;
				}
			}
		}
		std::cout << "ENC: Processed taps for " << instrument << " " << diff << std::endl;
        curFOff = 0;
        if (forcedOffPhrases.size() > 0) {
            for (Note &note : notes) {
                if (note.time > forcedOffPhrases[curFOff].end && curFOff<forcedOffPhrases.size()-1)
                    curFOff++;

                // std::cout << "fOn: " << forcedOnPhrases[curFOn].start << std::endl << "fOff: "
                //           << forcedOnPhrases[curFOn].end << std::endl;

                if (note.time >= forcedOffPhrases[curFOff].start && note.time < forcedOffPhrases[curFOff].end) {
					if (!note.pTap)
                    	note.phopo = false;
                }
            }
        }
            curFOn = 0;
            if (forcedOnPhrases.size() > 0) {
                for (Note &note : notes) {
                    if (note.time > forcedOnPhrases[curFOn].end && curFOn<forcedOnPhrases.size()-1)
                        curFOn++;

                    // std::cout << "fOn: " << forcedOnPhrases[curFOn].start << std::endl << "fOff: "
                    //           << forcedOnPhrases[curFOn].end << std::endl;

                    if (note.time >= forcedOnPhrases[curFOn].start && note.time < forcedOnPhrases[curFOn].end) {
						if (!note.pTap)
                        	note.phopo = true;
                    }
                }
            }

		std::cout << "ENC: Processed hopos for " << instrument << " " << diff << std::endl;
		LoadingState = OVERDRIVE;
        curODPhrase = 0;
        if (odPhrases.size() > 0) {
            for (Note &note : notes) {
                if (note.time > odPhrases[curODPhrase].end && curODPhrase<odPhrases.size()-1)
                    curODPhrase++;
                if (note.time >= odPhrases[curODPhrase].start && note.time < odPhrases[curODPhrase].end)
                    odPhrases[curODPhrase].noteCount++;
            }
        }
		std::cout << "ENC: Processed overdrive for " << instrument << " " << diff << std::endl;
		LoadingState = SOLOS;
        curSolo = 0;
        if (Solos.size() > 0) {
            for (Note &note : notes) {
                if (note.time > Solos[curSolo].end && curSolo<Solos.size()-1)
                    curSolo++;
                if (note.time >= Solos[curSolo].start && note.time < Solos[curSolo].end)
                    Solos[curSolo].noteCount++;
            }
        }
		std::cout << "ENC: Processed solos for " << instrument << " " << diff << std::endl;
		int esc = 0;
		LoadingState = PLASTIC_CALC;
		if (notes.size() > 0) {
			if (esc < notes.size() - 1) {
				if ((notes[esc].len + notes[esc].time > notes[esc+1].time) && notes[esc].len > 0) {
					notes[esc].extendedSustain = true;
				}
			}
		}
		std::cout << "ENC: Processed extended sustains for " << instrument << " " << diff << std::endl;
        int mult = 1;
        int multCtr = 0;
        int noteIdx = 0;
        bool isBassOrVocal = (instrument == 5);
		LoadingState = BASE_SCORE;
        for (auto it = notes.begin(); it != notes.end();) {
            Note& note = *it;
            if (!note.valid) {
                it = notes.erase(it);
            }
            else {
                baseScore += ((36 * note.chordSize) * mult);
                baseScore += (note.beatsLen * 12) * mult;
                if (noteIdx == 9) mult = 2;
                else if (noteIdx == 19) mult = 3;
                else if (noteIdx == 29) mult = 4;
                else if (noteIdx == 39 && isBassOrVocal) mult = 5;
                else if (noteIdx == 49 && isBassOrVocal) mult = 6;
                noteIdx++;
                ++it;
            }
        }
		std::cout << "ENC: Processed base score for " << instrument << " " << diff << std::endl;
		std::cout << "ENC: Processed plastic chart for " << instrument << " " << diff << std::endl;
    }

void parsePlasticDrums(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events, int diff, int instrument, bool proDrums) {

        /*
        * Note:
        * Tap = Yellow Tom
        * Force On = Blue Tom
        * Force Off = Green Tom
        */
        notes = {};
        bool odOn = false;
        bool soloOn = false;
        bool forceOn = false;
        bool tapOn = false;
        bool forceOff = false;
        bool discoFlip = false;
		bool drumFill = false;
		std::vector<int> fillNotes = {120,121,122,123,124};
        std::vector<int> notePitches = pDiffNotes[diff];

        midiFile.linkNotePairs();
        int odNote = 116;

        int yellowTom = 110;
        int blueTom = 111;
        int greenTom = 112;
        int curNote = -1;
        int curFOn = -1;
        int curTap = -1;
        int curFOff = -1;
        int curODPhrase = -1;
        int curSolo = -1;
        int curBPM = 0;
		int curFill = -1;
        resolution = midiFile.getTicksPerQuarterNote();
        for (int i = 0; i < events.getSize(); i++) {
            if (proDrums) {
                if (events[i].isMeta() && (int)events[i][1] == 1) {
                    std::string evt_string = "";
                    for (int k = 3; k < events[i].getSize(); k++) {
                        evt_string += events[i][k];
                    }
                    int mixDiff = evt_string[5] - '0';
                    if (diff == mixDiff) {
                        int drumMixType = evt_string[12] - '0';
                        if (evt_string[13] == ']')
                            discoFlip = false;
                        else {
                            if (evt_string[14] == ']')
                                discoFlip = true;
                            else
                                discoFlip = false;
                        }
                    }
                }
            }
            if (events[i].isNoteOn()) {
                if (events[i][1] >= notePitches[0] && events[i][1] <= notePitches[4]) {
                    double time = midiFile.getTimeInSeconds(trkidx, i);
                    int tick = midiFile.getAbsoluteTickTime(time);
                    int pitch = events[i][1];
                    int lane = pitch - notePitches[0];
                    Note newNote;
                    if (discoFlip) {
                        if (lane == 1) lane = 2;
                        else if (lane == 2) lane = 1;
                    }
                    newNote.lane = lane;
                    newNote.tick = tick;
                    newNote.time = time;
                    newNote.len = 0;
                    newNote.valid = true;
                    notes.push_back(newNote);
                    curNote++;
                }
                else if ((int)events[i][1] == yellowTom) {
                    if (!tapOn) {
                        tapPhrase newPhrase;
                        newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
                        tapPhrases.push_back(newPhrase);
                        tapOn = true;
                        curTap++;
                    }
                }
                else if ((int)events[i][1] == blueTom) {
                    if (!forceOn) {
                        forceOnPhrase newPhrase;
                        newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
                        forcedOnPhrases.push_back(newPhrase);
                        forceOn = true;
                        curFOn++;
                    }
                }
                else if ((int)events[i][1] == greenTom) {
                    if (!forceOff) {
                        forceOffPhrase newPhrase;
                        newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
                        forcedOffPhrases.push_back(newPhrase);
                        forceOff = true;
                        curFOff++;
                    }
                }
                else if ((int)events[i][1] == odNote) {
                    if (!odOn) {
                        odOn = true;
                        odPhrase newPhrase;
                        newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
                        odPhrases.push_back(newPhrase);
                        curODPhrase++;

                    }

                }
                else if ((int)events[i][1] == pSoloNote) {
                    if (!soloOn) {
                        soloOn = true;
                        solo newSolo;
                        newSolo.start = midiFile.getTimeInSeconds(trkidx, i);
                        Solos.push_back(newSolo);
                        curSolo++;
                    }
                }
                else if ((int)events[i][1] == fillNotes[1]) {
                	if (!drumFill) {
                		drumFill = true;
                		DrumFill newFill;
                		newFill.start = midiFile.getTimeInSeconds(trkidx, i);
                		fills.push_back(newFill);
                		curFill++;
                	}
                }
            }
            else if (events[i].isNoteOff()) {
                double time = midiFile.getTimeInSeconds(trkidx, i);
                int tick = midiFile.getAbsoluteTickTime(time);
                if ((int)events[i][1] == yellowTom) {
                    if (tapOn) {
                        tapPhrases[curTap].end = time;
                        tapOn = false;
                    }
                }
                else if ((int)events[i][1] == blueTom) {
                    if (forceOn) {
                        forcedOnPhrases[curFOn].end = time;
                        forceOn = false;
                    }
                }
                else if ((int)events[i][1] == greenTom) {
                    if (forceOff) {
                        forcedOffPhrases[curFOff].end = time;
                        forceOff = false;
                    }
                }
                else if ((int)events[i][1] == odNote) {
                    if (odOn) {
                        odPhrases[curODPhrase].end = time;
                        odOn = false;
                    }
                }
                else if ((int)events[i][1] == pSoloNote) {
                    if (soloOn) {
                        Solos[curSolo].end = time;
                        soloOn = false;
                    }
                }
                else if ((int)events[i][1] == fillNotes[1]) {
                	if (drumFill) {
                		fills[curFill].end = time;
                		drumFill = false;
                	}
                }
            }
        }
        std::cout << "ENC: Loaded base notes for " << instrument << " " << diff << std::endl;

        // LoadingState = NOTE_SORTING;
        std::sort(notes.begin(), notes.end(),
            compareNotesTL);
        std::cout << "ENC: Sorted notes for " << instrument << " " << diff << std::endl;
        curTap = 0;
        if (proDrums) {
            // LoadingState = NOTE_MODIFIERS;
            if (tapPhrases.size() > 0) {
                for (Note& note : notes) {
                    if (note.time > tapPhrases[curTap].end && curTap < tapPhrases.size() - 1)
                        curTap++;

                    // std::cout << "fOn: " << forcedOnPhrases[curFOn].start << std::endl << "fOff: "
                    //           << forcedOnPhrases[curFOn].end << std::endl;

                    if (note.lane == 2 && note.time >= tapPhrases[curTap].start && note.time < tapPhrases[curTap].end) {
                        note.pDrumTom = true;
                    }
                }
            }
            std::cout << "ENC: Processed yellow toms for " << instrument << " " << diff << std::endl;
            curFOn = 0;
            if (forcedOnPhrases.size() > 0) {
                for (Note& note : notes) {
                    if (note.time > forcedOnPhrases[curFOn].end && curFOn < forcedOnPhrases.size() - 1)
                        curFOn++;

                    // std::cout << "fOn: " << forcedOnPhrases[curFOn].start << std::endl << "fOff: "
                    //           << forcedOnPhrases[curFOn].end << std::endl;

                    if (note.lane == 3 && note.time >= forcedOnPhrases[curFOn].start && note.time < forcedOnPhrases[curFOn].end) {
                        note.pDrumTom = true;
                    }
                }
            }
            std::cout << "ENC: Processed blue toms for " << instrument << " " << diff << std::endl;
            curFOff = 0;
            if (forcedOffPhrases.size() > 0) {
                for (Note& note : notes) {
                    if (note.time > forcedOffPhrases[curFOff].end && curFOff < forcedOffPhrases.size() - 1)
                        curFOff++;

                    // std::cout << "fOn: " << forcedOnPhrases[curFOn].start << std::endl << "fOff: "
                    //           << forcedOnPhrases[curFOn].end << std::endl;

                    if (note.lane == 4 && note.time >= forcedOffPhrases[curFOff].start && note.time < forcedOffPhrases[curFOff].end) {
                        note.pDrumTom = true;
                    }
                }
            }
            std::cout << "ENC: Processed green toms for " << instrument << " " << diff << std::endl;
        }
        // LoadingState = OVERDRIVE;
        curODPhrase = 0;
        if (odPhrases.size() > 0) {
            for (Note& note : notes) {
                if (note.time > odPhrases[curODPhrase].end && curODPhrase < odPhrases.size() - 1)
                    curODPhrase++;
                if (note.time >= odPhrases[curODPhrase].start && note.time < odPhrases[curODPhrase].end)
                    odPhrases[curODPhrase].noteCount++;
            }
        }
		curFill = 0;
		if (fills.size() > 0) {
			for (Note& note : notes) {
				if (note.time == fills[curFill].end)
					note.pDrumAct = true;
				if (note.time > fills[curFill].end && curFill < fills.size() - 1)
					curFill++;
				if (note.time >= fills[curFill].start && note.time <= fills[curFill].end) {
					fills[curFill].noteCount++;

				}

			}
		}

        std::cout << "ENC: Processed overdrive for " << instrument << " " << diff << std::endl;
        // LoadingState = SOLOS;
        curSolo = 0;
        if (Solos.size() > 0) {
            for (Note& note : notes) {
                if (note.time > Solos[curSolo].end && curSolo < Solos.size() - 1)
                    curSolo++;
                if (note.time >= Solos[curSolo].start && note.time < Solos[curSolo].end)
                    Solos[curSolo].noteCount++;
            }
        }
        std::cout << "ENC: Processed solos for " << instrument << " " << diff << std::endl;
        int esc = 0;
        int mult = 1;
        int multCtr = 0;
        int noteIdx = 0;
        std::cout << "NOTECOUNT: " << notes.size() << std::endl;
        // LoadingState = BASE_SCORE;
        for (auto it = notes.begin(); it != notes.end();) {
            Note& note = *it;
            if (!note.valid) {
                it = notes.erase(it);
            }
            else {
                baseScore += ((36 * note.chordSize) * mult);
                baseScore += (note.beatsLen * 12) * mult;
                if (noteIdx == 9) mult = 2;
                else if (noteIdx == 19) mult = 3;
                else if (noteIdx == 29) mult = 4;
                noteIdx++;
                ++it;
            }
        }
        std::cout << "ENC: Processed base score for " << instrument << " " << diff << std::endl;
        std::cout << "ENC: Processed plastic chart for " << instrument << " " << diff << std::endl;
    }

	void resetNotes() {
		for (Note& note : notes) {
			note.accounted = false;
			note.hit = false;
			note.miss = false;
			note.held = false;
			note.heldTime = 0;
			note.hitTime = 0;
			note.perfect = false;
			note.countedForODPhrase = false;
            note.hitWithFAS = false;
            note.strumCount = 0;
		}
		for (odPhrase& phrase : odPhrases) {
			phrase.missed = false;
			phrase.notesHit = 0;
			phrase.added = false;
		}
        for (solo& Solo : Solos) {
            Solo.notesHit = 0;
        }
	}
};