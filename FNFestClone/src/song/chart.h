#pragma once
#include <vector>
#include "midifile/MidiFile.h"
#include "song.h"
struct Note 
{
	double time;
	double len;
	int lane;
	bool lift;
	//For plastic support later
	bool forceStrum;
	bool forceHopo;
};

struct odPhrase 
{
	double start;
	double end;
	bool missed = false;
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
	void parseNotes(smf::MidiFile &midiFile, int trkidx, smf::MidiEventList events, int diff) {
		std::vector<int> notePitches = diffNotes[diff];
		int odNote = 116;
		for (int i = 0; i < events.getSize(); i++) {
			if (events[i].isNoteOn()) {
				if ((int)events[i][1] >= notePitches[0] && (int)events[i][1] <= notePitches[1]) {
					double time = midiFile.getTimeInSeconds(trkidx, i);
					int lane = (int)events[i][1] - notePitches[0];
					int noteIdx = findNoteIdx(time, lane);
					if (noteIdx != -1) {
						notes[noteIdx].len = events[i].getDurationInSeconds();
					}
					else {
						Note newNote;
						newNote.time = time;
						newNote.len = events[i].getDurationInSeconds();
						newNote.lane = lane;
						notes.push_back(newNote);
					}
				}
				else if ((int)events[i][1] >= notePitches[2] && (int)events[i][1] <= notePitches[3]) {
					double time = midiFile.getTimeInSeconds(trkidx, i);
					int lane = (int)events[i][1] - notePitches[2];
					int noteIdx = findNoteIdx(time, lane);
					if (noteIdx != -1) {
						notes[noteIdx].lift = true;
					}
					else {
						Note newNote;
						newNote.time = time;
						newNote.len = -1;
						newNote.lane = lane;
						newNote.lift = true;
						notes.push_back(newNote);
					}
				}
				else if ((int)events[i][1] == odNote) {
					odPhrase newPhrase;
					newPhrase.start = midiFile.getTimeInSeconds(trkidx, i);
					newPhrase.end = newPhrase.start+events[i].getDurationInSeconds();
					odPhrases.push_back(newPhrase);
				}
			}
		}
	}
};