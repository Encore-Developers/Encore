//
// Created by marie on 04/05/2024.
//
#include <fstream>
#include <iostream>
#include "player.h"

#include "uuid.h"

#include <random>
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
    uuids::uuid_random_generator gen { generator };
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
    stats->UpStrum = false;
    stats->DownStrum = false;
    stats->StrumNoFretTime = -1.0;

    stats->curFill = 0;
    stats->curSolo = 0;
    stats->curBeatLine = 0;
    stats->curNoteInt = 0;
    stats->curODPhrase = 0;
    stats->curNoteIdx = { 0, 0, 0, 0, 0 };
    stats->curBPM = 0;
    stats->Mute = false;
    stats->StartTime = 0.0;
    stats->SongStartTime = 0.0;

    std::vector<int> curNoteIdx = { 0, 0, 0, 0, 0 };
    stats->Score = 0;
    stats->Combo = 0;
    stats->MaxCombo = 0;
    stats->Overhits = 0;
    stats->Notes = 0;
    stats->NotesHit = 0;
    stats->NotesMissed = 0;
    stats->PerfectHit = 0;
    stats->Health = 100.0f;

    stats->uvOffsetX = 0;
    stats->uvOffsetY = 0;

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
    EligibleForGoldStars = true;
}

void BandGameplayStats::AddNotePoint(bool perfect, int playerMult) {
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float perfectMult = perfect ? 1.2f : 1.0f;
    Score += (int)((
        (30.0f) * playerMult * perfectMult * OverdriveMultiplier[PlayersInOverdrive]
    ));
    // mute = false;
}

void BandGameplayStats::AddClassicNotePoint(bool perfect, int playerMult, int chordSize) {
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float perfectMult = perfect ? 1.2f : 1.0f;
    Score += (int)((
        chordSize * (30.0f) * playerMult * perfectMult
        * OverdriveMultiplier[PlayersInOverdrive]
    ));
    // mute = false;
}

void BandGameplayStats::DrumNotePoint(bool perfect, int playerMult, bool cymbal) {
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float perfectMult = perfect ? 1.2f : 1.0f;
    float cymbMult = cymbal ? 1.3f : 1.0f;
    Score += (int)(((30.0f) * playerMult * perfectMult
                    * OverdriveMultiplier[PlayersInOverdrive])
                   * cymbMult);
    // mute = false;
}

PlayerGameplayStats::PlayerGameplayStats() {
    Quit = false;
    FC = true;
    Paused = false;
    GoldStars = false;
    Overdrive = false;
    Mute = false;

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

