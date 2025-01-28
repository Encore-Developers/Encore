//
// Created by marie on 04/05/2024.
//
#include <fstream>
#include <iostream>
#include "player.h"

#include "uuid.h"
#include "song/scoring.h"
#include "song/song.h"

#include <random>
#include <nlohmann/json.hpp>

void PlayerGameplayStats::AddHealth() {
    if (Overdrive) {
        Health += (healthGainPerNote * healthOverdriveGainMult);
    } else {
        Health += healthGainPerNote;
    }
    if (Health > 1.0f)
        Health = 1.0f;
    Encore::EncoreLog(LOG_INFO, "Health gained");
    Encore::EncoreLog(LOG_INFO, TextFormat("Health: %4.2f", Health));
}

void PlayerGameplayStats::LoseHealth() {
    Health -= healthLossPerNote;

    if (Health < 0.0f)
        Health = 0.0f;
    Encore::EncoreLog(LOG_INFO, "Health lost");
    Encore::EncoreLog(LOG_INFO, TextFormat("Health: %4.2f", Health));
}

PlayerGameplayStats::PlayerGameplayStats(int difficulty, int instrument)
    : Quit(false), FC(true), Paused(false), Overdrive(false), Mute(false), Score(0),
      Combo(0), MaxCombo(0), Overhits(0), Notes(0), NotesHit(0), GoodHit(0),
      PerfectHit(0), NotesMissed(0), strummedNote(0), overdriveFill(0),
      overdriveActiveFill(0), overdriveActiveTime(0), overdriveActivateTime(0),
      BaseScore(0) {
    Difficulty = difficulty;
    Instrument = instrument;
}
void PlayerGameplayStats::HitNote(bool perfect) {
    NotesHit += 1;
    Notes += 1;
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float perfectMult = perfect ? PERFECT_MULTIPLIER : 1.0f;
    Score += (int)((BASE_NOTE_POINT * (multiplier()) * perfectMult));
    PerfectHit += perfect ? 1 : 0;
    GoodHit += perfect ? 0 : 1;
    AddHealth();
    Mute = false;
    if (Combo >= 3)
        Miss = false;
}
void PlayerGameplayStats::HitDrumsNote(bool perfect, bool cymbal) {
    NotesHit += 1;
    Notes += 1;
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    // set to +5 and not *1.25
    float cymbMult = cymbal ? CYMBAL_MULTIPLIER : 1;
    float perfectMult = perfect ? PERFECT_MULTIPLIER : 1.0f;
    Score += int(BASE_NOTE_POINT * multiplier() * perfectMult) * cymbMult;
    PerfectHit += perfect ? 1 : 0;
    GoodHit += perfect ? 0 : 1;
    AddHealth();
    Mute = false;
    if (Combo >= 3)
        Miss = false;
}
void PlayerGameplayStats::HitPlasticNote(Note note) {
    FAS = false;
    NotesHit += 1;
    Notes += 1;
    Combo += 1;
    if (note.perfect)
        LastPerfectTime = note.hitTime;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    double BaseNoteScore = (note.chordSize * BASE_NOTE_POINT);
    double OverdriveNoteScore = (BaseNoteScore * noODmultiplier());
    NoteScore += BaseNoteScore;
    MultiplierScore += (BaseNoteScore * noODmultiplier()) - BaseNoteScore;
    OverdriveScore += (BaseNoteScore * multiplier()) - OverdriveNoteScore;
    float perfectMult = note.perfect ? PERFECT_MULTIPLIER : 1.0f;
    PerfectScore += (BaseNoteScore * perfectMult) - BaseNoteScore;
    Score += (note.chordSize * (BASE_NOTE_POINT * multiplier() * perfectMult));
    PerfectHit += note.perfect ? 1 : 0;
    GoodHit += note.perfect ? 0 : 1;
    AddHealth();
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
    Miss = true;
    LoseHealth();
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
    Miss = true;
    LoseHealth();
}
int PlayerGameplayStats::maxMultForMeter() {
    if (Instrument == PartBass || Instrument == PartVocals || Instrument == PlasticBass)
        return 5;
    else
        return 3;
}
int PlayerGameplayStats::maxComboForMeter() {
    if (Instrument == PartBass || Instrument == PartVocals || Instrument == PlasticBass)
        return 50;
    else
        return 30;
}
int PlayerGameplayStats::Stars() {
    float starPercent = (float)Score / (float)BaseScore;
    if (starPercent < STAR_THRESHOLDS[Instrument][0]) {
        return 0;
    } else if (starPercent < STAR_THRESHOLDS[Instrument][1]) {
        return 1;
    } else if (starPercent < STAR_THRESHOLDS[Instrument][2]) {
        return 2;
    } else if (starPercent < STAR_THRESHOLDS[Instrument][3]) {
        return 3;
    } else if (starPercent < STAR_THRESHOLDS[Instrument][4]) {
        return 4;
    } else if (starPercent < STAR_THRESHOLDS[Instrument][5]) {
        return 5;
    } else if (starPercent >= STAR_THRESHOLDS[Instrument][5]) {
        return 5;
    } else
        return 5;

    return 0;
}

void PlayerGameplayStats::MultiplierUVCalculation() {
    if (Instrument == PartBass || Instrument == PartVocals || Instrument == PlasticBass) {
        if (Combo < 10) {
            uvOffsetX = 0;
            uvOffsetY = 0 + (Overdrive ? 0.5f : 0);
        } else if (Combo < 20) {
            uvOffsetX = 0.25f;
            uvOffsetY = 0 + (Overdrive ? 0.5f : 0);
        } else if (Combo < 30) {
            uvOffsetX = 0.5f;
            uvOffsetY = 0 + (Overdrive ? 0.5f : 0);
        } else if (Combo < 40) {
            uvOffsetX = 0.75f;
            uvOffsetY = 0 + (Overdrive ? 0.5f : 0);
        } else if (Combo < 50) {
            uvOffsetX = 0;
            uvOffsetY = 0.25f + (Overdrive ? 0.5f : 0);
        } else if (Combo >= 50) {
            uvOffsetX = 0.25f;
            uvOffsetY = 0.25f + (Overdrive ? 0.5f : 0);
        }
    } else {
        if (Combo < 10) {
            uvOffsetX = 0;
            uvOffsetY = 0 + (Overdrive ? 0.5 : 0);
        } else if (Combo < 20) {
            uvOffsetX = 0.25f;
            uvOffsetY = 0 + (Overdrive ? 0.5 : 0);
        } else if (Combo < 30) {
            uvOffsetX = 0.5f;
            uvOffsetY = 0 + (Overdrive ? 0.5 : 0);
        } else if (Combo >= 30) {
            uvOffsetX = 0.75f;
            uvOffsetY = 0 + (Overdrive ? 0.5 : 0);
        }
    };
}


int PlayerGameplayStats::multiplier() {
    int od = Overdrive ? 2 : 1;
    if (IsBassOrVox()) {
        if (Combo >= 50)
            return 6 * od;

    } else {
        if (Combo >= 30)
            return 4 * od;
    };
    return (Combo/10)+1 * od;


}

int PlayerGameplayStats::noODmultiplier() {
    if (IsBassOrVox()) {
        if (Combo >= 50)
            return 6;

    } else {
        if (Combo >= 30)
            return 4;
    };
    return (Combo/10)+1;
}

bool PlayerGameplayStats::IsBassOrVox() {
    if (Instrument == PartBass || Instrument == PartVocals || Instrument == PlasticBass) {
        return true;
    }
    return false;
}
float PlayerGameplayStats::comboFillCalc() {
    if (Combo == 0) {
        return 0;
    }
    if (IsBassOrVox()) {
        int ComboMod = Combo % 10;
        if (Combo >= 50 || ComboMod == 0) {
            return 1.0f;
        }
        return (static_cast<float>(ComboMod) / 10.0f);
    }
    int ComboMod = Combo % 10;
    if (Combo >= 30 || ComboMod == 0) {
        return 1.0f; // If combo is 30 or more, set float value to 1.0
    }
    return (static_cast<float>(ComboMod) / 10.0f);
}

Player::Player() {
    Name = "New Player";
    Difficulty = 0;
    Instrument = 0;
    ReadiedUpBefore = false;
    Bot = false;

    InputCalibration = 0;
    NoteSpeed = 1;
    ProDrums = false;
    HighwayLength = 1;
    BrutalMode = false;
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
};

void Player::ResetGameplayStats() {
    stats->Quit = false;
    stats->FC = true;
    stats->Paused = false;
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

    stats->SustainScore = 0;
    stats->MultiplierScore = 0;
    stats->OverdriveScore = 0;
    stats->PerfectScore = 0;
    stats->NoteScore = 0;

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

    SustainScore = 0;
    MultiplierScore = 0;
    OverdriveScore = 0;
    PerfectScore = 0;
    NoteScore = 0;

    BaseScore = 0;

    StartTime = 0.0;
    SongStartTime = 0.0;

    Health = 100;
    PlayersInOverdrive = 0;

    BaseScore = 0;
    EligibleForGoldStars = true;
}

void BandGameplayStats::AddNotePoint(bool perfect, int playerMult) {
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float perfectMult = perfect ? PERFECT_MULTIPLIER : 1.0f;
    Score += (int)((
        BASE_NOTE_POINT * playerMult * perfectMult
        * OverdriveMultiplier[PlayersInOverdrive]
    ));
    // mute = false;
}

void BandGameplayStats::AddClassicNotePoint(bool perfect, int playerMult, int chordSize) {
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float perfectMult = perfect ? PERFECT_MULTIPLIER : 1.0f;
    double BaseNoteScore = (chordSize * BASE_NOTE_POINT);
    NoteScore += BaseNoteScore;
    MultiplierScore += (BaseNoteScore * playerMult) - BaseNoteScore;
    OverdriveScore +=
        (BaseNoteScore * OverdriveMultiplier[PlayersInOverdrive]) - BaseNoteScore;
    PerfectScore += (BaseNoteScore * perfectMult) - BaseNoteScore;
    Score +=
        (chordSize
         * (BASE_NOTE_POINT * playerMult * OverdriveMultiplier[PlayersInOverdrive]
            * perfectMult));
    // mute = false;
}

void BandGameplayStats::DrumNotePoint(bool perfect, int playerMult, bool cymbal) {
    Combo += 1;
    if (Combo > MaxCombo)
        MaxCombo = Combo;
    float perfectMult = perfect ? PERFECT_MULTIPLIER : 1.0f;
    // used to be a 25% multiplier, just add 5 points instead lol
    float cymbMult = cymbal ? CYMBAL_MULTIPLIER : 1;
    Score += int(BASE_NOTE_POINT * playerMult * perfectMult
                 * OverdriveMultiplier[PlayersInOverdrive])
        * cymbMult;
    // mute = false;
}

PlayerGameplayStats::PlayerGameplayStats() {
    Quit = false;
    FC = true;
    Paused = false;
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
