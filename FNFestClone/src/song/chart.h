#include <vector>
struct Note {
	float time;
	float len;
	bool lift;
	//For plastic support later
	bool forceStrum;
	bool forceHopo;
};

struct odPhrase {
	float start;
	float end;
	bool missed = false;
};
struct Chart {
	std::vector<Note> notes;
	std::vector<odPhrase> odPhrases;
};