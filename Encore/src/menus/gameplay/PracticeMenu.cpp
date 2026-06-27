#include "PracticeMenu.h"

#include "ChartLoadingMenu.h"
#include "resultsMenu.h"
#include "debug/EncoreDebug.h"
#include "menus/MenuManager.h"
#include "menus/main/MainMenu.h"
#include "song/OpenSource.h"
#include "menus/main/SongSelectMenu.h"

PracticeMenu::PracticeMenu(Song *song) : GameplayMenu(song) {}
void PracticeMenu::KeyboardInputCallback(SDL_KeyboardEvent *event) {
    if (state == GAMEPLAY) {
        GameplayMenu::KeyboardInputCallback(event);
    }
}
void PracticeMenu::ControllerInputCallback(Encore::ControllerEvent event) {
    if (state == GAMEPLAY) {
        GameplayMenu::ControllerInputCallback(event);
    }
    if (state == OPTIONS) {
        optionsMenu.Input(event);
    } else if (state == SECTIONLIST) {
        sectionsMenu.Input(event);
        if (sectionsMenu.selectingRange && sectionsMenu.rangeStart < 2) {
            sectionsMenu.rangeStart = 2;
        }
        if (!sectionsMenu.selectingRange && event.action == Encore::Action::PRESS && event.channel == Encore::InputChannel::LANE_2) {
            state = OPTIONS;
        }
    }
}
void PracticeMenu::Load() {
    GameplayMenu::Load();
    for (auto& track : tracks) {
        track->IntroTimer = 0;
    }
    optionsMenu.displayParams.Fnt(ASSET(rubik));
    sectionsMenu.displayParams.Fnt(ASSET(rubik));
    sectionsMenu.inputWrapping = false;

    optionsMenu.CreateOption<SimpleMenu::FuncOption>(LOCALIZE("overshell.return"), [this]() {
        state = GAMEPLAY;
    });
    optionsMenu.CreateOption<SimpleMenu::FuncOption>(LOCALIZE("practice.selectSections"), [this]() {
        state = SECTIONLIST;
    });
    optionsMenu.CreateOption<SimpleMenu::FuncOption>(LOCALIZE("practice.exit"), [this]() {
       TheMenuManager.CreateAndSwitchMenu<ChartLoadingMenu>(curSong);
    });
    optionsMenu.CreateOption<SimpleMenu::FuncOption>(LOCALIZE("overshell.exitSong"), [this]() {
        TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
    });

    PopulateSections();
}
bool PracticeMenu::IsPaused() {
    return state != GAMEPLAY;
}
void PracticeMenu::UpdatePauseState() {
    if (IsPaused()) {
        if (!streamsPaused) {
            TheAudioManager.pauseStreams();
            streamsPaused = true;
        }
    } else {
        if (streamsPaused) {
            TheAudioManager.playStreams();
            streamsPaused = false;
        }
    }
}
bool PracticeMenu::CheckPauseInput(Encore::ControllerEvent event) {
    if (event.action == Encore::Action::PRESS && event.channel == Encore::InputChannel::PAUSE) {
        if (state == GAMEPLAY) {
            state = OPTIONS;
            return true;
        }
    }
    return false;
}
void PracticeMenu::PopulateSections() {
    sectionsMenu.options.clear();

    sectionsMenu.CreateOption<SimpleMenu::FuncOption>(LOCALIZE("generic.back"), [this]() {
        state = OPTIONS;
    });
    sectionsMenu.CreateOption<SimpleMenu::FuncOption>(LOCALIZE("practice.wholeSong"), [this]() {
        sectionsMenu.rangeStart = 2;
        sectionsMenu.rangeEnd = sectionsMenu.options.size()-1;
    });
    for (auto& section : TheSongTime.Sections) {
        sectionsMenu.CreateOption<PracticeSectionOption>(this, &section);
    }
}
PracticeSectionOption::PracticeSectionOption(
    SimpleMenu::Instance *instance, PracticeMenu *practiceMenu, Section *section
) : Option(instance, section->name), practiceMenu(practiceMenu) {}

bool PracticeSectionOption::Input(Encore::ControllerEvent event) {
    if (!instance->selectingRange) {
        if (event.IsAccept()) {
            instance->StartRangeSelect();
            return true;
        }
    } else {
        if (event.IsAccept()) {
            instance->EndRangeSelect();
            return true;
        }
    }
    return false;
}

void PracticeMenu::Draw() {
    UIInput = state != GAMEPLAY;
    DrawGameplay();
    Units& u = Units::getInstance();
    if (state == OPTIONS) {
        optionsMenu.displayParams
        .Pos(u.wpct(0.015), u.hpct(0.015))
        .Bounds(u.winpct(0.3), u.hinpct(0.9))
        .Padding(u.hinpct(0.010), u.hinpct(0.005))
        .Size(u.hinpct(0.03))
        .Col(WHITE);
        optionsMenu.Draw();
    }
    if (state == SECTIONLIST) {
        if (sectionsMenu.selectingRange && sectionsMenu.rangeStart < 2) {
            sectionsMenu.rangeStart = 2;
        }
        sectionsMenu.displayParams
        .Pos(u.wpct(0.015), u.hpct(0.015))
        .Bounds(u.winpct(0.3), u.hinpct(0.9))
        .Padding(u.hinpct(0.010), u.hinpct(0.005))
        .Size(u.hinpct(0.03))
        .Col(WHITE);
        sectionsMenu.Draw();
    }
}

void PracticeMenu::DrawGameplay() {
    UpdatePauseState();
    if (IsPaused() || EncoreDebug::showDebug) {
        ShowCursor();
    } else {
        HideCursor();
    }
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    TheSongTime.UpdateTick();
    TheSongTime.UpdateBeatlines();
    ClearBackground(BLACK);
    unsigned char BackgroundColor = 0;
    if (!songPlaying && tracks.back()->IntroTimer==0) {
        TheSongTime.Reset();
        double songEnd = floor(TheAudioManager.GetMusicTimeLength());
        TheAudioManager.UpdateAudioStreamVolumes();
        TheSongTime.Start(songEnd);
        TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[0].handle);
        songPlaying = true;
    }
    GameMenu::DrawAlbumArtBackground();
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color{ 0, 0, 0, 128 });
    DrawRectangle(
        0,
        0,
        GetRenderWidth(),
        GetRenderHeight(),
        Color{ 255, 255, 255, BackgroundColor }
    );

    if (TheSongTime.GetElapsedTime() > curSong->end - 0.1) {
        // TODO: endgame
        TheSongTime.FullReset();
        TheAudioManager.unloadStreams();
        songPlaying = false;
        TheMenuManager.CreateAndSwitchMenu<resultsMenu>(curSong);
        return;
    }

    for (auto &stream : TheAudioManager.loadedStreams) {
        float volume = TheGameSettings.GetInactiveVolume();
        if (stream.instrument == AUDIOSTEM(Vocals))
            volume = TheGameSettings.GetVocalsVolume();
        if (stream.instrument == AUDIOSTEM(Crowd))
            volume = TheGameSettings.GetCrowdVolume();
        stream.volume = volume;
    }
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (ThePlayerManager.ActivePlayers[i] == -1) continue;
        Player &player = ThePlayerManager.GetActivePlayer(i);

        if (!IsPaused()) {
            ZoneScopedN("Engine Update")
            if (player.engine->IsWithinPracticeSection(TheSongTime.GetElapsedTime()) || !
                player.engine->practice) {
            }
            player.engine->UpdateOnFrame(TheSongTime.GetElapsedTime());
            player.engine->UpdateStats(player.Instrument, player.Difficulty);
        }

        tracks.at(i)->Draw();
        auto chart = player.engine->chart;
        float volume = TheGameSettings.GetActiveVolume();
        if (player.engine.get()->stats.get()->AudioMuted) {
            volume = TheGameSettings.GetMuteVolume();
        }
        TheAudioManager.SetAudioStreamVolume(GetStemFromInstrument(SongPart(player.Instrument)), volume);
    }
    TheAudioManager.UpdateAudioStreamVolumes();

    GameMenu::DrawTopBarText(true);
}