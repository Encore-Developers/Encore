//
// Created by marie on 04/05/2024.
//
#include <fstream>
#include <iostream>
#include "player.h"

#include "uuid.h"

#include <random>
#include <nlohmann/json.hpp>

void PlayerGameplayStats::HitNote(bool perfect) {
    NotesHit += 1;
    Notes += 1;
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float perfectMult = perfect ? 1.2f : 1.0f;
    Score += (int)((30.0f * (multiplier()) * perfectMult));
    PerfectHit += perfect ? 1 : 0;
    GoodHit += perfect ? 0 : 1;
    Mute = false;
}
void PlayerGameplayStats::HitDrumsNote(bool perfect, bool cymbal) {
    NotesHit += 1;
    Notes += 1;
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float cymbMult = cymbal ? 1.3f : 1.0f;
    float perfectMult = perfect ? 1.2f : 1.0f;
    Score += (int)((30.0f * (multiplier()) * perfectMult) * cymbMult);
    PerfectHit += perfect ? 1 : 0;
    GoodHit += perfect ? 0 : 1;
    Mute = false;
}
void PlayerGameplayStats::HitPlasticNote(Note note) {
    FAS = false;
    NotesHit += 1;
    Notes += 1;
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float perfectMult = note.perfect ? 1.2f : 1.0f;
    Score += (note.chordSize * (int)(30.0f * (multiplier()) * perfectMult));
    PerfectHit += note.perfect ? 1 : 0;
    GoodHit += note.perfect ? 0 : 1;
    curNoteInt++;
    Mute = false;
}
void PlayerGameplayStats::MissNote() {
    NotesMissed += 1;
    Notes += 1;
    // if (combo != 0)
    //     playerAudioManager.playSample("miss", sfxVolume);
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    Combo = 0;
    FAS = false;
    FC = false;
    curNoteInt++;
    Mute = true;
}
void PlayerGameplayStats::OverHit() {
    // if (combo != 0)
    //     playerAudioManager.playSample("miss", sfxVolume);
    Overstrum = true;
    FAS = false;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    Combo = 0;
    Overhits += 1;
    FC = false;
    Mute = true;
}
int PlayerGameplayStats::maxMultForMeter() {
    if (Instrument == PAD_BASS || Instrument == PAD_VOCALS || Instrument == PLASTIC_BASS)
        return 5;
    else
        return 3;
}
int PlayerGameplayStats::maxComboForMeter() {
    if (Instrument == PAD_BASS || Instrument == PAD_VOCALS || Instrument == PLASTIC_BASS)
        return 50;
    else
        return 30;
}
int PlayerGameplayStats::Stars() {
    float starPercent = (float)Score / (float)BaseScore;
    if (starPercent < xStarThreshold[0]) {
        return 0;
    } else if (starPercent < xStarThreshold[1]) {
        return 1;
    } else if (starPercent < xStarThreshold[2]) {
        return 2;
    } else if (starPercent < xStarThreshold[3]) {
        return 3;
    } else if (starPercent < xStarThreshold[4]) {
        return 4;
    } else if (starPercent < xStarThreshold[5]) {
        return 5;
    } else if (starPercent >= xStarThreshold[5]) {
        GoldStars = true;
        return 5;
    } else
        return 5;

    return 0;
}
int PlayerGameplayStats::multiplier() {
    int od = Overdrive ? 2 : 1;

    if (Instrument == PAD_BASS || Instrument == PAD_VOCALS
        || Instrument == PLASTIC_BASS) {
        if (Combo < 10) {
            uvOffsetX = 0;
            uvOffsetY = 0 + (Overdrive ? 0.5f : 0);
            return 1 * od;
        } else if (Combo < 20) {
            uvOffsetX = 0.25f;
            uvOffsetY = 0 + (Overdrive ? 0.5f : 0);
            return 2 * od;
        } else if (Combo < 30) {
            uvOffsetX = 0.5f;
            uvOffsetY = 0 + (Overdrive ? 0.5f : 0);
            return 3 * od;
        } else if (Combo < 40) {
            uvOffsetX = 0.75f;
            uvOffsetY = 0 + (Overdrive ? 0.5f : 0);
            return 4 * od;
        } else if (Combo < 50) {
            uvOffsetX = 0;
            uvOffsetY = 0.25f + (Overdrive ? 0.5f : 0);
            return 5 * od;
        } else if (Combo >= 50) {
            uvOffsetX = 0.25f;
            uvOffsetY = 0.25f + (Overdrive ? 0.5f : 0);
            return 6 * od;
        } else {
            return 1 * od;
        };
    } else {
        if (Combo < 10) {
            uvOffsetX = 0;
            uvOffsetY = 0 + (Overdrive ? 0.5 : 0);
            return 1 * od;
        } else if (Combo < 20) {
            uvOffsetX = 0.25f;
            uvOffsetY = 0 + (Overdrive ? 0.5 : 0);
            return 2 * od;
        } else if (Combo < 30) {
            uvOffsetX = 0.5f;
            uvOffsetY = 0 + (Overdrive ? 0.5 : 0);
            return 3 * od;
        } else if (Combo >= 30) {
            uvOffsetX = 0.75f;
            uvOffsetY = 0 + (Overdrive ? 0.5 : 0);
            return 4 * od;
        } else {
            return 1 * od;
        }
    };
}
int PlayerGameplayStats::noODmultiplier() {
    if (Instrument == PAD_BASS || Instrument == PAD_VOCALS
        || Instrument == PLASTIC_BASS) {
        if (Combo < 10) {
            uvOffsetX = 0;
            uvOffsetY = 0;
            return 1;
        } else if (Combo < 20) {
            uvOffsetX = 0.25f;
            uvOffsetY = 0;
            return 2;
        } else if (Combo < 30) {
            uvOffsetX = 0.5f;
            uvOffsetY = 0;
            return 3;
        } else if (Combo < 40) {
            uvOffsetX = 0.75f;
            uvOffsetY = 0;
            return 4;
        } else if (Combo < 50) {
            uvOffsetX = 0;
            uvOffsetY = 0.25f;
            return 5;
        } else if (Combo >= 50) {
            uvOffsetX = 0.25f;
            uvOffsetY = 0.25f;
            return 6;
        } else {
            return 1;
        };
    } else {
        if (Combo < 10) {
            uvOffsetX = 0;
            uvOffsetY = 0;
            return 1;
        } else if (Combo < 20) {
            uvOffsetX = 0.25f;
            uvOffsetY = 0;
            return 2;
        } else if (Combo < 30) {
            uvOffsetX = 0.5f;
            uvOffsetY = 0;
            return 3;
        } else if (Combo >= 30) {
            uvOffsetX = 0.75f;
            uvOffsetY = 0;
            return 4;
        } else {
            return 1;
        }
    };
}
bool PlayerGameplayStats::IsBassOrVox() {
    if (Instrument == PAD_BASS || Instrument == PAD_VOCALS
        || Instrument == PLASTIC_BASS) {
        return true;
    }
    return false;
}
float PlayerGameplayStats::comboFillCalc() {
    if (Combo == 0) {
        return 0;
    }
    if (Instrument == PAD_DRUMS || Instrument == PAD_LEAD || Instrument == PAD_KEYS
        || Instrument == PLASTIC_DRUMS || Instrument == PLASTIC_LEAD
        || Instrument == PLASTIC_KEYS) {
        // For instruments 0 and 2, limit the float value to 0.0 to 0.4
        if (Combo >= 30) {
            return 1.0f; // If combo is 30 or more, set float value to 1.0
        } else {
            int ComboMod = Combo % 10;
            if (ComboMod == 0)
                return 1.0f;
            else {
                return (static_cast<float>(ComboMod) / 10.0f); // Float value from 0.0
                                                               // to 0.9 every 10
                                                               // notes
            }
        }
    } else {
        if (Combo >= 50) {
            return 1.0f; // If combo is 30 or more, set float value to 1.0
        }
        // For instruments 1 and 3, limit the float value to 0.0 to 0.6
        int ComboMod = Combo % 10;
        if (ComboMod == 0)
            return 1.0f;
        else {
            return (static_cast<float>(ComboMod) / 10.0f); // Float value from 0.0 to
                                                           // 0.9 every 10 notes
        }
    }
}

Player::Player()  {
    Name = "New Player";
    Difficulty = 0;
    Instrument = 0;
    ReadiedUpBefore = false;
    Bot = false;

    InputCalibration = 0;
    NoteSpeed = 1;
    ProDrums = false;
    HighwayLength = 1;

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
int PlayerGameplayStats::ScoreToDisplay() {
    int scoreToReturn = Score;
    for (auto buf : SustainScoreBuffer) {
        scoreToReturn += buf;
    }
    return scoreToReturn;
}

