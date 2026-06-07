#pragma once
#include "GameplayMenu.h"
#include "menus/settings/util/SimpleMenu/SimpleMenu.h"


class PracticeMenu : public GameplayMenu {
public:
    enum State {
        GAMEPLAY,
        OPTIONS,
        SECTIONLIST
    };

    State state = SECTIONLIST;
    SimpleMenu::Instance optionsMenu;
    SimpleMenu::Instance sectionsMenu;


    PracticeMenu(Song* song);

    void KeyboardInputCallback(SDL_KeyboardEvent* event) override;
    void ControllerInputCallback(Encore::ControllerEvent event) override;
    void Draw() override;
    void DrawGameplay();
    void Load() override;
    bool IsPaused() override;
    void UpdatePauseState() override;
    bool CheckPauseInput(Encore::ControllerEvent event) override;
    void PopulateSections();
};

class PracticeSectionOption : public SimpleMenu::Option {
public:
    Section* section;
    PracticeMenu* practiceMenu;

    PracticeSectionOption(SimpleMenu::Instance* instance, PracticeMenu* practiceMenu, Section* section);

    bool Input(Encore::ControllerEvent event) override;
};