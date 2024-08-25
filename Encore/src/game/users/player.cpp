//
// Created by marie on 04/05/2024.
//
#include <fstream>
#include <iostream>
#include <game/users/player.h>
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

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
	EligibleForGoldStars = false;
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

void PlayerManager::LoadPlayerList(std::filesystem::path PlayerListSaveFile) {
	if (std::filesystem::exists(PlayerListSaveFile)) {
		std::ifstream ifs(PlayerListSaveFile);
		if (!ifs.is_open()) {
			std::cerr << "Failed to open PlayerList JSON file" << std::endl;
		}
		std::string jsonString((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		PlayerListFile.Parse(jsonString.c_str());

		if (PlayerListFile.IsObject()) {
			for (auto it = PlayerListFile.MemberBegin(); it != PlayerListFile.MemberEnd(); ++it) {
				const rapidjson::Value& entry = it->value;
				if (entry.IsObject()) {
					Player player;
					// important string things
					if (entry.HasMember("name") && entry["name"].IsString()) {
						player.Name = entry["name"].GetString();
					}
					if (entry.HasMember("id") && entry["id"].IsString()) {
						player.PlayerID = entry["id"].GetString();
					}

					// int things
					if (entry.HasMember("difficulty") && entry["difficulty"].IsInt()) {
						player.Difficulty = entry["difficulty"].GetInt();
					}
					if (entry.HasMember("instrument") && entry["instrument"].IsInt()) {
						player.Instrument = entry["instrument"].GetInt();
					}
					if (entry.HasMember("songsPlayed") && entry["songsPlayed"].IsInt()) {
						player.SongsPlayed = entry["songsPlayed"].GetInt();
					}

					// float things
					if (entry.HasMember("inputOffset") && entry["inputOffset"].IsFloat()) {
						player.InputCalibration = entry["inputOffset"].IsFloat();
					}
					if (entry.HasMember("noteSpeed") && entry["noteSpeed"].IsFloat()) {
						player.NoteSpeed = entry["noteSpeed"].IsFloat();
					}

					// bool things
					if (entry.HasMember("bot") && entry["bot"].IsBool()) {
						player.Bot = entry["bot"].IsBool();
					}
					if (entry.HasMember("classic") && entry["classic"].IsBool()) {
						player.ClassicMode = entry["classic"].IsBool();
					}
					if (entry.HasMember("leftyFlip") && entry["leftyFlip"].IsBool()) {
						player.LeftyFlip = entry["leftyFlip"].IsBool();
					}
					PlayerList.push_back(player);
				}
			}
		}
	}
}; // make player, load player stuff to PlayerList

void PlayerManager::SavePlayerList(std::filesystem::path PlayerListSaveFile) {

}; // ough this is gonna be complicated

void PlayerManager::CreatePlayer(Player player) {

}; // set it as the next one in PlayerList

void PlayerManager::DeletePlayer(Player PlayerToDelete) {

}; // remove player, reload playerlist
void PlayerManager::RenamePlayer(Player PlayerToRename) {

}; // rename player
