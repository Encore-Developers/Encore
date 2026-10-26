#pragma once

#include "../overshell/OvershellMenu.h"
#include "menus/settings/util/SimpleMenu/SimpleMenu.h"
#include "menus/util/ButtonAction.h"
#include "menus/util/ButtonActionRegistry.h"
#include "song/song.h"

class SongSelectMenu : public OvershellMenu {
public:
    SongSelectMenu() = default;
    ~SongSelectMenu() override;
    void DrawSongInformation(float leftPos, Song* curSong);
    void ScrollUpHeader();
    void ScrollDownHeader();
    void KeyboardInputCallback(SDL_KeyboardEvent* event) override;
    void LoadPreview(Song& song);
    std::string GetHeader();
    void ControllerInputCallback(Encore::ControllerEvent event) override;
    void ScrollToCurrentSong();
    void TogglePlaylistMode();
    void Draw() override;
    void Load() override;
    void StopPreview();
    void Unload();
    void UpdatePreviewVolume(double currentTime);
    void ScrollSongSelect(int val);
private:
    SimpleMenu::Instance menu;
    bool menuOpen = false;
    bool menuReleaseConfirm = false;

    void OpenContextMenu();
    void OpenSortMenu();
    void OpenSectionMenu();

    Encore::ButtonActionRegistry buttReg;
    // 5 for unbound controllers
    std::array<bool, 5> ControllerOrangeHeld = { false, false, false, false, false };
    double previewStartTime = 0.0;
    float currentPreviewVolume = 0.0f;
    enum class PreviewState { Hysteresis, FadeIn, Playing, FadeOut, Pause, Failed } previewState = PreviewState::Hysteresis;
    const float fadeDuration = 2.5f;
    const float previewPlayDuration = 30.0f;
    const float pauseDuration = 2.5f;
    double phaseStartTime = 0.0;
    size_t topOflistMenu = -1;
    size_t curSongMenuPos = -1;
    double curTime = 0.0;
    double selectionTime = 0.0;
};