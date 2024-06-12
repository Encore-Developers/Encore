#pragma once
#include <vector>
#include <string>
#include "midifile/MidiFile.h"
#include "song.h"
#include "game/timingvalues.h"
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
	//For plastic support later
	bool forceStrum = false;
	bool forceHopo = false;
    bool hopo = false;

	bool isGood(double eventTime, double inputOffset) const {
		return (time - goodBackend + inputOffset < eventTime &&
			time + goodFrontend + inputOffset > eventTime);
	}
	bool isPerfect(double eventTime, double inputOffset) {
		return (time - perfectBackend + inputOffset < eventTime &&
			time + perfectFrontend + inputOffset > eventTime);
	}
};

struct solo {
    double start;
    double end;
    int noteCount = 0;
    int notesHit = 0;
    bool perfect = false;
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
public:
    // plastic stuff :tm:
    // note: clones dont do thresh. they do 12ths
    int hopoThreshold = 170;

	std::vector<Note> notes;
	std::vector <std::vector<int>> notes_perlane{ {},{},{},{},{} };
	int baseScore = 0;
	int findNoteIdx(double time, int lane) {
		for (int i = 0; i < notes.size();i++) {
			if (notes[i].time == time && notes[i].lane == lane)
				return i;
		}
		return -1;
	}
	std::vector<odPhrase> odPhrases;
    std::vector<solo> Solos;
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
		curODPhrase = 0;
		if (odPhrases.size() > 0) {
			for (Note &note : notes) {
				if (note.time > odPhrases[curODPhrase].end && curODPhrase<odPhrases.size()-1)
					curODPhrase++;
				if (note.time >= odPhrases[curODPhrase].start && note.time <= odPhrases[curODPhrase].end)
					odPhrases[curODPhrase].noteCount++;
			}
		}
        curSolo = 0;
        if (Solos.size() > 0) {
            for (Note &note : notes) {
                if (note.time > Solos[curSolo].end && curSolo<Solos.size()-1)
                    curSolo++;
                if (note.time >= Solos[curSolo].start && note.time <= Solos[curSolo].end)
                    Solos[curSolo].noteCount++;
            }
        }
		int mult = 1;
		int multCtr = 0;
		int noteIdx = 0;
		bool isBassOrVocal = (instrument == 1 || instrument == 3);
		for (auto it = notes.begin(); it != notes.end();) {
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