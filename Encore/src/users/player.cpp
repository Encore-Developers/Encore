//
// Created by marie on 04/05/2024.
//
#include <fstream>
#include <iostream>
#include "player.h"
#include <nlohmann/json.hpp>

Player::Player() {
	Name = "New Player";
	Difficulty = 0;
	Instrument = 0;
	ReadiedUpBefore = false;
	Bot = false;

	// gen stuff, move to own function (thanks https://github.com/mariusbancila/stduuid)
	std::random_device rd;
	auto seed_data = std::array<int, std::mt19937::state_size> {};
	std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
	std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
	std::mt19937 generator(seq);
	uuids::uuid_random_generator gen{generator};
	uuids::uuid const id = gen();

	joypadID = -1;
	PlayerID = uuids::to_string(id);
	SongsPlayed = 0;
	LeftyFlip = false;
	Online = false;
	ClassicMode = false;
	stats = new PlayerGameplayStats;
	stats->Difficulty = Difficulty;
	stats->Instrument = Instrument;
};

void Player::ResetGameplayStats() {
	stats->Quit = false;
	stats->FC = true;
	stats->Paused = false;
	stats->GoldStars = false;
	stats->Overdrive = false;
	stats->FAS = false;
	stats->Overstrum = false;

	stats->curFill = 0;
	stats->curSolo = 0;
	stats->curBeatLine = 0;
	stats->curNoteInt = 0;
	stats->curODPhrase = 0;
	stats->curNoteIdx = {0,0,0,0,0};
	stats->curBPM = 0;

	stats->StartTime = 0.0;
	stats->SongStartTime = 0.0;

	std::vector<int> curNoteIdx = { 0,0,0,0,0 };
	stats->Score = 0;
	stats->Combo = 0;
	stats->MaxCombo = 0;
	stats->Overhits = 0;
	stats->Notes = 0;
	stats->NotesHit = 0;
	stats->NotesMissed = 0;
	stats->PerfectHit = 0;
	stats->Health = 100.0f;

	stats->overdriveFill = 0.0f;
	stats->overdriveActiveFill = 0.0f;
	stats->overdriveActiveTime = 0.0;
	stats->overdriveActivateTime = 0.0;

	stats->BaseScore = 0;
}

BandGameplayStats::BandGameplayStats() {
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
	NotesHit = 0;
	NotesMissed = 0;
	PerfectHit = 0;
	Health = 100.0f;

	overdriveFill = 0.0f;
	overdriveActiveFill = 0.0f;
	overdriveActiveTime = 0.0;
	overdriveActivateTime = 0.0;

	BaseScore = 0;

	StartTime = 0.0;
	SongStartTime = 0.0;

	Health = 100;

	BaseScore = 0;
	EligibleForGoldStars = true;
}

void BandGameplayStats::ResetBandGameplayStats() {
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
	NotesHit = 0;
	NotesMissed = 0;
	PerfectHit = 0;
	Health = 100.0f;

	overdriveFill = 0.0f;
	overdriveActiveFill = 0.0f;
	overdriveActiveTime = 0.0;
	overdriveActivateTime = 0.0;

	BaseScore = 0;

	StartTime = 0.0;
	SongStartTime = 0.0;

	Health = 100;

	BaseScore = 0;
	EligibleForGoldStars = false;
}

void BandGameplayStats::AddNotePoint(bool perfect, int playerMult) {
	Combo += 1;
	if (Combo > MaxCombo)
		MaxCombo = Combo;
	float perfectMult = perfect ? 1.2f : 1.0f;
	Score += (int)(((30.0f) * playerMult * perfectMult * OverdriveMultiplier[PlayersInOverdrive]));
	// mute = false;
}

void BandGameplayStats::AddClassicNotePoint(bool perfect, int playerMult, int chordSize) {
	Combo += 1;
	if (Combo > MaxCombo)
		MaxCombo = Combo;
	float perfectMult = perfect ? 1.2f : 1.0f;
	Score += (int)((chordSize * (30.0f) * playerMult * perfectMult * OverdriveMultiplier[PlayersInOverdrive]));
	// mute = false;
}

void BandGameplayStats::DrumNotePoint(bool perfect, int playerMult, bool cymbal) {
	Combo += 1;
	if (Combo > MaxCombo)
		MaxCombo = Combo;
	float perfectMult = perfect ? 1.2f : 1.0f;
	float cymbMult = cymbal ? 1.3f : 1.0f;
	Score += (int)(((30.0f) * playerMult * perfectMult * OverdriveMultiplier[PlayersInOverdrive]) * cymbMult);
	// mute = false;
}

PlayerGameplayStats::PlayerGameplayStats() {
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
	NotesHit = 0;
	NotesMissed = 0;
	PerfectHit = 0;
	Health = 100.0f;

	overdriveFill = 0.0f;
	overdriveActiveFill = 0.0f;
	overdriveActiveTime = 0.0;
	overdriveActivateTime = 0.0;

	BaseScore = 0;
}

using json = nlohmann::json;

void PlayerManager::LoadPlayerList(std::filesystem::path PlayerListSaveFile) {
	std::ifstream f(PlayerListSaveFile);
	json PlayerListJson = json::parse(f);
	TraceLog(LOG_INFO, "Loading player list");
	for (auto jsonObject: PlayerListJson) {
		Player newPlayer;
		newPlayer.Name = jsonObject.at("name").get<std::string>();
		newPlayer.PlayerID = jsonObject.at("UUID").get<std::string>();
		newPlayer.Difficulty = jsonObject.at("diff").get<int>();
		newPlayer.Instrument = jsonObject.at("inst").get<int>();
		newPlayer.NoteSpeed = jsonObject.at("NoteSpeed").get<float>();
		newPlayer.InputCalibration = jsonObject.at("inputOffset").get<float>();
		newPlayer.Bot = jsonObject.at("bot").get<bool>();
		newPlayer.ClassicMode = jsonObject.at("classic").get<bool>();
		newPlayer.ProDrums = jsonObject.at("proDrums").get<bool>();
		newPlayer.LeftyFlip = jsonObject.at("lefty").get<bool>();
		if (!jsonObject["accentColor"].is_null()) {
			int r  = 0, g  = 0, b = 0;
			r = jsonObject["accentColor"].at("r").get<int>();
			g = jsonObject["accentColor"].at("g").get<int>();
			b = jsonObject["accentColor"].at("b").get<int>();
			newPlayer.AccentColor = Color{static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b),255};
		} else newPlayer.AccentColor = {255,0,255,255};

		TraceLog(LOG_INFO, ("Successfully loaded player " + newPlayer.Name).c_str());
		PlayerList.push_back(newPlayer);
	};
}; // make player, load player stuff to PlayerList

void PlayerManager::SavePlayerList(std::filesystem::path PlayerListSaveFile) {
	json PlayerListJson;

}; // ough this is gonna be complicated

void PlayerManager::CreatePlayer(Player player) {

}; // set it as the next one in PlayerList

void PlayerManager::DeletePlayer(Player PlayerToDelete) {

}; // remove player, reload playerlist
void PlayerManager::RenamePlayer(Player PlayerToRename) {

}; // rename player
