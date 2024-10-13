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

bool InputHandler::isNoteMatch(const Note &curNote, int pressedMask, PlayerGameplayStats *stats) {
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
    } else {
        if (pressedMask > 0) {
            return false;
        } else {
            return true;
        }
    }
}

void InputHandler::CheckPlasticInputs(
    Player *player,
    int lane,
    int action,
    float eventTime,
    PlayerGameplayStats *stats
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
    bool frettingInput = action == GLFW_PRESS && lane != 8008135 && lane != -1;
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

    if (lane == 8008135 && action == GLFW_PRESS && !stats->FAS) {
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
            if (!curChart.overdrive.events.empty()
                && !curChart.overdrive[stats->curODPhrase].missed
                && curNote.time >= curChart.overdrive[stats->curODPhrase].StartSec
                && curNote.time < curChart.overdrive[stats->curODPhrase].EndSec)
                curChart.overdrive[stats->curODPhrase].missed = true;
        }
    } else if (lane == 8008135 && action == GLFW_RELEASE) {
        stats->DownStrum = false;
        stats->UpStrum = false;
        return;
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

    if (!curNote.hit && frettingInput
        /*!IsInWindow && (curNote.phopo || curNote.pTap)
        && greaterThanLastNoteMatch && lastNote.hit && frettingInput
        && lastNote.time + 0.0375 < calibratedTime
        */
    ) {
        curNote.GhostCount += 1;
    }
    
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
            if (lane == -1) {
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
                if (action == GLFW_PRESS && stats->overdriveHitAvailable) {
                    if (curNote->isGood(eventTime, player->InputCalibration)
                        && !curNote->hit) {
                        for (int newlane = 0; newlane < 5; newlane++) {
                            int chordLane = curChart.findNoteIdx(curNote->time, newlane);
                            if (chordLane != -1) {
                                Note &chordNote = curChart.notes[chordLane];
                                if (!chordNote.accounted) {
                                    chordNote.hit = true;
                                    stats->overdriveLanesHit[newlane] = true;
                                    chordNote.hitTime = eventTime;

                                    if ((chordNote.len) > 0 && !chordNote.lift) {
                                        chordNote.held = true;
                                    }
                                    if (chordNote.isPerfect(
                                            eventTime, player->InputCalibration
                                        )) {
                                        chordNote.perfect = true;
                                    }
                                    chordNote.HitOffset = chordNote.time - eventTime;
                                    stats->HitNote(chordNote.perfect);
                                    playerManager.BandStats.AddNotePoint(
                                        chordNote.perfect, stats->multiplier()
                                    );
                                    chordNote.accounted = true;
                                }
                            }
                        }
                        stats->overdriveHitAvailable = false;
                        stats->overdriveLiftAvailable = true;
                    }
                }
                else if (action == GLFW_RELEASE && stats->overdriveLiftAvailable) {
                    if (curNote->isGood(eventTime, player->InputCalibration)
                        && !curNote->hit) {
                        for (int newlane = 0; newlane < 5; newlane++) {
                            if (stats->overdriveLanesHit[newlane]) {
                                int chordLane =
                                    curChart.findNoteIdx(curNote->time, newlane);
                                if (chordLane != -1) {
                                    Note &chordNote = curChart.notes[chordLane];
                                    if (chordNote.lift) {
                                        chordNote.hit = true;
                                        chordNote.HitOffset = chordNote.time - eventTime;
                                        stats->overdriveLanesHit[newlane] = false;
                                        chordNote.hitTime = eventTime;

                                        if (chordNote.isPerfect(
                                                eventTime, player->InputCalibration
                                            )) {
                                            chordNote.perfect = true;
                                        }
                                        chordNote.accounted = true;
                                    }
                                }
                            }
                        }
                        stats->overdriveLiftAvailable = false;
                    }
                }
                if (action == GLFW_RELEASE && curNote->held && (curNote->len) > 0
                    && stats->overdriveLiftAvailable) {
                    for (int newlane = 0; newlane < 5; newlane++) {
                        if (stats->overdriveLanesHit[newlane]) {
                            int chordLane = curChart.findNoteIdx(curNote->time, newlane);
                            if (chordLane != -1) {
                                Note &chordNote = curChart.notes[chordLane];
                                if (chordNote.held && chordNote.len > 0) {
                                    if (!((player->Difficulty == 3
                                           && settings.keybinds5K[chordNote.lane])
                                          || (player->Difficulty != 3
                                              && settings.keybinds4K[chordNote.lane])
                                        )) {
                                        chordNote.held = false;
                                        // player.score +=
                                        // player.sustainScoreBuffer[chordNote.lane];
                                        // player.sustainScoreBuffer[chordNote.lane] = 0;
                                        // player.mute = true;
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                for (int i = stats->curNoteIdx[lane];
                     i < curChart.notes_perlane[lane].size();
                     i++) {
                    Note &curNote = curChart.notes[curChart.notes_perlane[lane][i]];

                    if (lane != curNote.lane)
                        continue;
                    if (((curNote.lift && action == GLFW_RELEASE) || action == GLFW_PRESS)
                        && curNote.isGood(eventTime, player->InputCalibration)
                        && !curNote.hit && !curNote.accounted) {
                        if (curNote.lift && action == GLFW_RELEASE) {
                            stats->lastHitLifts[lane] = curChart.notes_perlane[lane][i];
                        }
                        stats->HitNote(curNote.perfect);
                        playerManager.BandStats.AddNotePoint(
                            curNote.perfect, stats->multiplier()
                        );
                        curNote.cHitNote(eventTime, player->InputCalibration);
                        break;
                    }
                    if ((!stats->HeldFrets[curNote.lane]
                         && !stats->HeldFretsAlt[curNote.lane])
                        && curNote.held && (curNote.len) > 0) {
                        curNote.held = false;
                        // stats->Score += player.sustainScoreBuffer[curNote.lane];
                        // player.sustainScoreBuffer[curNote.lane] = 0;
                        // player.mute = true;
                        // SetAudioStreamVolume(audioManager.loadedStreams[instrument].stream,
                        // missVolume);
                    }

                    if (action == GLFW_PRESS && eventTime > songList.curSong->music_start
                        && !curNote.isGood(eventTime, player->InputCalibration)
                        && !curNote.accounted
                        && eventTime > stats->overdriveHitTime + 0.05
                        && !stats->OverhitFrets[lane]) {
                        if (stats->lastHitLifts[lane] != -1) {
                            if (eventTime
                                    > curChart.notes[stats->lastHitLifts[lane]].time - 0.1
                                && eventTime
                                    < curChart.notes[stats->lastHitLifts[lane]].time
                                        + 0.1)
                                continue;
                        }
                        stats->OverHit();
                        if (!curChart.overdrive.events.empty()
                            && eventTime
                                >= curChart.overdrive[stats->curODPhrase].StartSec
                            && eventTime < curChart.overdrive[stats->curODPhrase].EndSec
                            && !curChart.overdrive[stats->curODPhrase].missed)
                            curChart.overdrive[stats->curODPhrase].missed = true;
                        stats->OverhitFrets[lane] = true;
                    }
                }
            }
        } else {
            CheckPlasticInputs(player, lane, action, eventTime, player->stats);
        }
    }
}
