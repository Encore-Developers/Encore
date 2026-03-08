//
// Created by marie on 16/11/2024.
//

#include "ReadyUpMenu.h"

#include "MenuManager.h"
#include "gameMenu.h"
#include "raygui.h"
#include "uiUnits.h"
#include "gameplay/gameplayRenderer.h"
#include "users/playerManager.h"
void ReadyUpMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {}
void ReadyUpMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {}

SongParts GetSongPart(smf::MidiEventList track) {
    for (int events = 0; events < track.getSize(); events++) {
        std::string trackName;
        if (!track[events].isMeta())
            continue;
        if ((int)track[events][1] == 3) {
            for (int k = 3; k < track[events].getSize(); k++) {
                trackName += track[events][k];
            }
            if (TheSongList.curSong->ini)
                return partFromStringINI(trackName);
            return partFromString(trackName);
        }
    }
    return Invalid;
}

std::vector<std::vector<int> > pDiffRangeNotes = {
    { 60, 64 }, { 72, 76 }, { 84, 88 }, { 96, 100 }
};

void IsPartValid(smf::MidiEventList track, SongParts songPart, int trackNumber) {
    if (songPart == Invalid || songPart == PitchedVocals || songPart == BeatLines) {
        return;
    }
    for (int diff = 0; diff < 4; diff++) {
        bool StopSearching = false;

        for (int i = 0; i < track.getSize(); i++) {
            if (track[i].isNoteOn() && !track[i].isMeta()
                && track[i][1] >= pDiffRangeNotes[diff][0]
                && track[i][1] <= pDiffRangeNotes[diff][1] && !StopSearching) {
                TheSongList.curSong->parts[songPart]->ValidDiffs.at(diff) = true;
                TheSongList.curSong->parts[songPart]->TrackInt = trackNumber;
                TheSongList.curSong->parts[songPart]->Valid = true;
                StopSearching = true;
            }
            if (StopSearching)
                break;
        }
    }
}

void ReadyUpMenu::Draw() {}
void ReadyUpMenu::Load() {}