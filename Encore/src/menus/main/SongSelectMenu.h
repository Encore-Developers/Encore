//
// Created by marie on 16/11/2024.
//

#ifndef SONGSELECTMENU_H
#define SONGSELECTMENU_H
#include "../overshell/OvershellMenu.h"
#include "menus/util/ButtonAction.h"
#include "menus/util/ButtonActionRegistry.h"
#include "song/song.h"

class SongSelectMenu : public OvershellMenu {
public:
    SongSelectMenu() = default;
    ~SongSelectMenu() override;
    void ScrollUpHeader();
    void ScrollDownHeader();
    void KeyboardInputCallback(SDL_KeyboardEvent* event) override;
    void LoadPreview(Song& song);
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) override;
    void ScrollToCurrentSong();
    void Draw() override;
    void Load() override;
    void StopPreview();
    void Unload();
    void UpdatePreviewVolume(double currentTime);
    void ScrollSongSelect(int val);
private:
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
    size_t curSongMenuPos = -1;
    double curTime = 0.0;
    double selectionTime = 0.0;
};



#endif //SONGSELECTMENU_H