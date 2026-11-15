#include "PracticeMenu.h"

#include "ChartLoadingMenu.h"
#include "resultsMenu.h"
#include "debug/EncoreDebug.h"
#include "menus/MenuManager.h"
#include "menus/util/GameMenu.h"
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
    if (state == OPTIONS) {
        optionsMenu.displayParams
        .Pos(Units::wpct(0.015), Units::hpct(0.015))
        .Bounds(Units::winpct(0.3), Units::hinpct(0.9))
        .Padding(Units::hinpct(0.010), Units::hinpct(0.005))
        .Size(Units::hinpct(0.03))
        .Col(WHITE);
        optionsMenu.Draw();
    }
    if (state == SECTIONLIST) {
        if (sectionsMenu.selectingRange && sectionsMenu.rangeStart < 2) {
            sectionsMenu.rangeStart = 2;
        }
        sectionsMenu.displayParams
        .Pos(Units::wpct(0.015), Units::hpct(0.015))
        .Bounds(Units::winpct(0.3), Units::hinpct(0.9))
        .Padding(Units::hinpct(0.010), Units::hinpct(0.005))
        .Size(Units::hinpct(0.03))
        .Col(WHITE);
        sectionsMenu.Draw();
    }
}

void PracticeMenu::DrawGameplay() {
    // TODO
}