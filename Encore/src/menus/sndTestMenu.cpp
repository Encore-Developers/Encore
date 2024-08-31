#include "sndTestMenu.h"
#include "../song/audio.h"
#include "gameMenu.h"
#include "../menus/uiUnits.h"
#include "raylib.h"
#include "raygui.h"
#include <filesystem>

auto& TheAudioMgr = AudioManager::getInstance();

SoundTestMenu::SoundTestMenu() {}

SoundTestMenu::~SoundTestMenu() {
    for (auto snd : mSoundIds) { TheAudioMgr.unloadSample(snd); }
}

void SoundTestMenu::Draw() {
    static int selectedSound;
    Units u = Units::getInstance();
    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), BLACK);
    DrawTextEx(mFont, "SOUND TEST", {u.wpct(0.33), u.hpct(0.2)}, u.hinpct(0.1), 0, WHITE);
    DrawTextEx(mFont, mSoundIds[selectedSound].c_str(), {u.wpct(0.33), u.hpct(0.8)}, u.hinpct(0.1), 0, WHITE);

    if (GuiButton({u.wpct(0.35), u.hpct(0.4), u.winpct(0.04), u.hinpct(0.2)}, "-")) { selectedSound--; }
    if (GuiButton({u.wpct(0.4), u.hpct(0.4), u.winpct(0.2), u.hinpct(0.2)}, "Play sound")) {
        TheAudioMgr.playSample(mSoundIds[selectedSound], 0.8f);
    }
    if (GuiButton({u.wpct(0.61), u.hpct(0.4), u.winpct(0.04), u.hinpct(0.2)}, "+")) { selectedSound++; }

    if (GuiButton({0,0, u.winpct(0.3),u.hinpct(0.15)}, "Return")) TheGameMenu.SwitchScreen(MENU);

    if (selectedSound < 0) selectedSound = mSoundIds.size() - 1;
    if (selectedSound > mSoundIds.size() - 1) selectedSound = 0;
}

void SoundTestMenu::Load() {
    std::filesystem::path assetsdir = GetApplicationDirectory();
    assetsdir /= "Assets";
    mFont = LoadFont((assetsdir / "fonts/Rubik-Regular.ttf").string().c_str());
    
    mSoundIds.push_back(std::string("combobreak"));
    TheAudioMgr.loadSample((assetsdir / "combobreak.mp3").string(), "combobreak");
    mSoundIds.push_back(std::string("kick"));
    TheAudioMgr.loadSample((assetsdir / "kick.wav").string(), "kick");
}
