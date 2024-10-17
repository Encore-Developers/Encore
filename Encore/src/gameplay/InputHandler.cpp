//
// Created by marie on 15/09/2024.

#include "InputHandler.h"

#include "enctime.h"
#include "settings-old.h"
#include "GLFW/glfw3.h"
#include "song/songlist.h"

int InputHandler::calculatePressedMask(PlayerGameplayStats *stats) {
    int mask = 0;
    for (int pressedButtons = 0; pressedButtons < stats->HeldFrets.size();
         pressedButtons++) {
        if (stats->HeldFrets[pressedButtons] || stats->HeldFretsAlt[pressedButtons])
            mask += PlasticFrets[pressedButtons];
    }
    return mask;
}

bool InputHandler::isNoteMatch(
    const Note &curNote, int pressedMask, PlayerGameplayStats *stats
) {
    bool maskGreater = pressedMask >= curNote.mask;
    bool maskEqual = pressedMask == curNote.mask;
    bool maskLess = pressedMask < (curNote.mask * 2);
    if (!curNote.pOpen) {
        bool chordMatch = stats->extendedSustainActive ? maskGreater : maskEqual;
        bool singleMatch =
            stats->extendedSustainActive ? maskGreater : maskGreater && maskLess;
        if (curNote.chord) {
            return chordMatch;
        }
        return singleMatch;
    }
    if (pressedMask > 0) {
        return false;
    }
    return true;
}

void InputHandler::CheckPlasticInputs(
    Player *player, int lane, int action, float eventTime, PlayerGameplayStats *stats
) {
    // basic shit so that its easier to Do Things lol
    PlayerManager &playerManager = PlayerManager::getInstance();
    SongList &songList = SongList::getInstance();
    Chart &curChart =
        songList.curSong->parts[player->Instrument]->charts[player->Difficulty];

    if (stats->curNoteInt >= curChart.notes.size())
        stats->curNoteInt = curChart.notes.size() - 1;
    Note &curNote = curChart.notes[stats->curNoteInt];
    int pressedMask = calculatePressedMask(stats);
    bool inCoda = songList.curSong->BRE.IsCodaActive(eventTime);
    Note &lastNote = curChart.notes[stats->curNoteInt == 0 ? 0 : stats->curNoteInt - 1];

    // TODO: BRE logic
    if (inCoda)
        return;

    bool firstNote = stats->curNoteInt == 0;

    bool greaterThanLastNoteMatch = pressedMask > (lastNote.mask << 2) + 1;
    bool frettingInput = action == GLFW_PRESS && lane != STRUM && lane != -1;
    bool noteMatch = isNoteMatch(curNote, pressedMask, stats);

    bool HopoOverstrumCheck =
        ((lastNote.phopo && lastNote.hit && !firstNote)
             ? (eventTime > lastNote.hitTime + 0.075f)
             : (true));
    float calibratedTime = eventTime + player->InputCalibration;
    bool fretHopoMatch = (curNote.phopo && (stats->Combo > 0 || stats->curNoteInt == 0));
    bool fretTapMatch = curNote.pTap;
    bool CouldTap = (fretHopoMatch || fretTapMatch) && curNote.GhostCount < 2;

    bool IsInWindow = curNote.isGood(eventTime, player->InputCalibration) && !curNote.hit
        && !curNote.accounted;
    if (lane == STRUM && action == GLFW_RELEASE) {
        stats->DownStrum = false;
        stats->UpStrum = false;
        return;
    }
    if (lane == STRUM && action == GLFW_PRESS && !stats->FAS) {
        stats->StrumNoFretTime = calibratedTime;
        curNote.strumCount++;
        if (IsInWindow && !curNote.hitWithFAS) {
            stats->FAS = true;
            curNote.hitWithFAS = true;
            stats->strummedNote = stats->curNoteInt;
        }
        if (!IsInWindow && HopoOverstrumCheck) {
            stats->OverHit();
            if (lastNote.held && !firstNote) {
                lastNote.held = false;
            }
            curChart.overdrive.UpdateEventViaNote(curNote, stats->curODPhrase);
        }
    }

    // is hopo hittable as tap
    if ((curNote.hitWithFAS || CouldTap) && IsInWindow && noteMatch) {
        curNote.cHitNote(eventTime, player->InputCalibration);
        // TODO: fix for plastic
        playerManager.BandStats.AddClassicNotePoint(
            curNote.perfect, stats->noODmultiplier(), curNote.chordSize
        );
        stats->HitPlasticNote(curNote);
        return;
    }
    if (!curNote.hit && frettingInput)
        curNote.GhostCount += 1;
}

void InputHandler::handleInputs(Player *player, int lane, int action) {
    PlayerGameplayStats *stats = player->stats;
    SettingsOld &settings = SettingsOld::getInstance();
    SongTime &enctime = TheSongTime;
    SongList &songList = SongList::getInstance();
    PlayerManager &playerManager = PlayerManager::getInstance();

    if (stats->Paused)
        return;
    if (lane == -2)
        return;
    if (settings.mirrorMode && lane != -1 && !player->ClassicMode) {
        lane = (player->Difficulty == 3 ? 4 : 3) - lane;
    }
    if (!enctime.Running()) {
        return;
    }
    Chart &curChart =
        songList.curSong->parts[player->Instrument]->charts[player->Difficulty];
    float eventTime = enctime.GetSongTime();
    if (true) {
        if (action == GLFW_PRESS && (lane == -1) && stats->overdriveFill > 0
            && !stats->Overdrive) {
            stats->overdriveActiveTime = eventTime;
            stats->overdriveActiveFill = stats->overdriveFill;
            stats->Overdrive = true;
            stats->overdriveHitAvailable = true;
            stats->overdriveHitTime = eventTime;

            playerManager.BandStats.PlayersInOverdrive += 1;
            playerManager.BandStats.Overdrive = true;
        }

        if (!player->ClassicMode) {
            CheckPadInputs(player, lane, action, eventTime, player->stats);
        } else {
            CheckPlasticInputs(player, lane, action, eventTime, player->stats);
        }
    }
}
void InputHandler::CheckPadInputs(
    Player *player, int lane, int action, double eventTime, PlayerGameplayStats *stats
) {
    PlayerManager &playerManager = PlayerManager::getInstance();
    SongList &songList = SongList::getInstance();
    Chart &curChart =
        songList.curSong->parts[player->Instrument]->charts[player->Difficulty];
    SettingsOld &settings = SettingsOld::getInstance();

    if (lane == OVERDRIVE_ACT) {
        if ((action == GLFW_PRESS && !stats->overdriveHitAvailable)
            || (action == GLFW_RELEASE && !stats->overdriveLiftAvailable))
            return;
        Note *curNote = &curChart.notes[0];
        for (auto &note : curChart.notes) {
            if (note.isGood(eventTime, player->InputCalibration) && !note.hit) {
                curNote = &note;
                break;
            }
        }
        if (action == GLFW_PRESS && !stats->overdriveHeld) {
            stats->overdriveHeld = true;
        } else if (action == GLFW_RELEASE && stats->overdriveHeld) {
            stats->overdriveHeld = false;
        }

        for (int newlane = 0; newlane < 5; newlane++) {
            int chordLane = curChart.findNoteIdx(curNote->time, newlane);
            if (chordLane == -1)
                break;

            Note &chordNote = curChart.notes[chordLane];
            bool PressToHitNote = !chordNote.accounted && action == GLFW_PRESS
                && stats->overdriveHitAvailable;

            bool ReleaseToHitNote = !chordNote.accounted && action == GLFW_RELEASE
                && stats->overdriveLiftAvailable && stats->overdriveLanesHit[newlane];

            bool SustainReleased = action == GLFW_RELEASE && curNote->held
                && (curNote->len) > 0 && stats->overdriveLiftAvailable;

            bool Expert = player->Difficulty == 3;

            bool StillHeldCheck = Expert ? settings.keybinds5K[chordNote.lane]
                                         : settings.keybinds4K[chordNote.lane];

            if ((PressToHitNote || ReleaseToHitNote) && !SustainReleased) {
                if (!chordNote.padHitNote(eventTime, player->InputCalibration))
                    break;
                stats->overdriveLanesHit[newlane] = PressToHitNote;
                stats->HitNote(chordNote.perfect);
                playerManager.BandStats.AddNotePoint(
                    chordNote.perfect, stats->multiplier()
                );
                stats->overdriveHitAvailable = !PressToHitNote;
                stats->overdriveLiftAvailable = !ReleaseToHitNote;
                return;
            }

            // this is for releasing sustains
            if (!StillHeldCheck) {
                chordNote.held = false;
                player->stats->Mute = true;
            }
        }
        return;
    }
    for (int i = stats->curNoteIdx[lane]; i < curChart.notes_perlane[lane].size(); i++) {
        Note &curNote = curChart.notes[curChart.notes_perlane[lane][i]];

        if (lane != curNote.lane)
            continue;
        bool LiftHit = (curNote.lift && action == GLFW_RELEASE);
        bool TapHit = action == GLFW_PRESS;

        if ((LiftHit || TapHit) && !curNote.accounted) {
            if (!curNote.padHitNote(eventTime, player->InputCalibration))
                break;
            if (LiftHit) {
                stats->lastHitLifts[lane] = curChart.notes_perlane[lane][i];
            }
            stats->HitNote(curNote.perfect);
            playerManager.BandStats.AddNotePoint(curNote.perfect, stats->multiplier());
            return;
        }
        if ((!stats->HeldFrets[curNote.lane] && !stats->HeldFretsAlt[curNote.lane])
            && curNote.held && (curNote.len) > 0) {
            curNote.held = false;
            player->stats->Mute = true;
            return;
        }
        bool AfterSongStart = eventTime > songList.curSong->music_start;

        if (TapHit && AfterSongStart
            && !curNote.isGood(eventTime, player->InputCalibration) && !curNote.accounted
            && eventTime > stats->overdriveHitTime + 0.05 && !stats->OverhitFrets[lane]) {
            if (stats->lastHitLifts[lane] != -1) {
                if (eventTime > curChart.notes[stats->lastHitLifts[lane]].time - 0.05
                    && eventTime < curChart.notes[stats->lastHitLifts[lane]].time + 0.05)
                    continue;
            }
            stats->OverHit();
            curChart.overdrive.MissCurrentEvent(eventTime, stats->curODPhrase);
            stats->OverhitFrets[lane] = true;
        }
    }
}