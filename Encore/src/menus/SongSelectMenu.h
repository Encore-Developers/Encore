//
// Created by marie on 16/11/2024.
//

#ifndef SONGSELECTMENU_H
#define SONGSELECTMENU_H
#include "OvershellMenu.h"
#include <filesystem>
#include <map>

class SongSelectMenu : public OvershellMenu {
public:
    SongSelectMenu() = default;
    ~SongSelectMenu() override;
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) override;
    void Draw() override;
    void Load() override;
    void Unload();
    void UpdatePreviewVolume(double currentTime);
    void ScrollSongSelect(int val);
private:
    double previewStartTime = 0.0;
    float currentPreviewVolume = 0.0f;
    enum class PreviewState { FadeIn, Playing, FadeOut, Pause } previewState = PreviewState::FadeIn;
    const float fadeDuration = 2.5f;
    const float previewPlayDuration = 30.0f;
    const float pauseDuration = 2.5f;
    double phaseStartTime = 0.0;
    int curSongMenuPos = -1;
    double curTime = 0.0;
    double selectionTime = 0.0;
};



#endif //SONGSELECTMENU_H