#pragma once
#include <vector>
#include "midifile/MidiFile.h"
#include "song.h"
struct Note 
{
	double time;
	double len;
	double heldTime=0.0;
	double sustainThreshold = 0.2;
	int lane;
	bool lift = false;
	bool hit = false;
	bool held = false;
	bool valid = false;
	bool miss = false;
	bool accounted = false;
	bool countedForODPhrase = false;
    bool perfect = false;
	//For plastic support later
	bool forceStrum;
	bool forceHopo;
};

struct odPhrase 
{
	double start;
	double end;
	int noteCount=0;
	int notesHit = 0;
	bool missed = false;
	bool added = false;
};

class Chart 
{
private:
	std::vector<std::vector<int>> diffNotes = { {60,63,66,69}, {72,75,78,81}, {84,87,90,93}, {96,100,102,106} };
public:
	std::vector<Note> notes;
	int findNoteIdx(double time, int lane) {
		for (int i = 0; i < notes.size();i++) {
			if (notes[i].time == time && notes[i].lane == lane)
				return i;
		}
		return -1;
	}
	std::vector<odPhrase> odPhrases;
	void parseNotes(smf::MidiFile& midiFile, int trkidx, smf::MidiEventList events, int diff) {
		std::vector<bool> notesOn{ false,false,false,false,false};
		bool odOn = false;
		std::vector<double> noteOnTime{ 0.0, 0.0, 0.0, 0.0, 0.0};
		std::vector<int> notePitches = diffNotes[diff];
		int odNote = 116;
		int curODPhrase = -1;
		int curBPM = 0;
		
		for (int i = 0; i < events.getSize(); i++) {
			if (events[i].isNoteOn()) {
				double time = midiFile.getTimeInSeconds(trkidx, i);
				if ((int)events[i][1] >= notePitches[0] && (int)events[i][1] <= notePitches[1]) {
					int lane = (int)events[i][1] - notePitches[0];
					if (!notesOn[lane]) {
						noteOnTime[lane] = time;
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
					
				}
			}
			else if (events[i].isNoteOff()) {
				double time = midiFile.getTimeInSeconds(trkidx, i);
				if ((int)events[i][1] >= notePitches[0] && (int)events[i][1] <= notePitches[1]) {
					int lane = (int)events[i][1] - notePitches[0];
					if (notesOn[lane] == true) {
						int noteIdx = findNoteIdx(noteOnTime[lane], lane);
						if (noteIdx != -1) {
							notes[noteIdx].len = time - notes[noteIdx].time;
						}
						notesOn[lane] = false;
					}
				}
				else if ((int)events[i][1] == odNote) {
					if (odOn == true) {
						odPhrases[curODPhrase].end = time;
						odOn = false;
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
	}
};