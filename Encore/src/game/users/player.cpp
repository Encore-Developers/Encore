//
// Created by marie on 04/05/2024.
//
#include <game/users/player.h>


Player::Player(std::string name) {
	Name = name;
	Difficulty = 0;
	Instrument = 0;
	ReadiedUpBefore = false;
	Bot = false;
	stud::uuid u = stud::uuid::generate();
	PlayerID = u.string();
	SongsPlayed = 0;
	LeftyFlip = false;
	Online = false;
	ClassicMode = false;
	stats = PlayerGameplayStats::PlayerGameplayStats();
};

PlayerGameplayStats::PlayerGameplayStats(int inst, int diff) {
	Quit = false;
	FC = true;
	Paused = false;
	GoldStars = false;
	Overdrive = false;

	Score = 0;
	Combo = 0;
	MaxCombo = 0;
	Overhits = 0;
	Notes = 0;
	MaxNotes = 0;
	Health = 100.0f;

	overdriveFill = 0.0f;
	overdriveActiveFill = 0.0f;
	overdriveActiveTime = 0.0;
	overdriveActivateTime = 0.0;

	Instrument = inst;
	Difficulty = diff;
	BaseScore = 0;
}