//
// Created by marie on 16/11/2024.
//

#ifndef SONGSELECTMENU_H
#define SONGSELECTMENU_H
#include "OvershellMenu.h"
#include "uiUnits.h"
#include "song/song.h"
#include <filesystem>
#include <map>

class SongSelectMenu : public OvershellMenu {
public:
    SongSelectMenu() = default;
    ~SongSelectMenu() override;
    void KeyboardInputCallback(int key, int scancode, int action, int mods) override;
    void ControllerInputCallback(int joypadID, GLFWgamepadstate state) override;
    void Draw() override;
    void Load() override;
    void Unload();
    void UpdatePreviewVolume(double currentTime);

private:
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
    std::filesystem::path getDirectory() const {
        return directory;
    }
    double previewStartTime = 0.0;
    float currentPreviewVolume = 0.0f;
    enum class PreviewState { FadeIn, Playing, FadeOut, Pause } previewState = PreviewState::FadeIn;
    const float fadeDuration = 2.5f;
    const float previewPlayDuration = 30.0f;
    const float pauseDuration = 2.5f;
    double phaseStartTime = 0.0;
    int animatingSongID = -1;
    int prevAnimatingSongID = -1;
    double animationStartTime = 0.0;
    const float animationDuration = 0.5f;
    int pendingSongID = -1;
    double selectionTime = 0.0;
    double seekPendingTime = -1.0;
    struct TextMetrics {
        float titleFontSize;
        float artistFontSize;
        float titleTextWidth;
        float artistTextWidth;
    };
    std::map<int, TextMetrics> songTextMetrics;

    void ComputeSongTextMetrics(Song& song);
    static void DrawAlbumArtBackgroundPro(const Texture2D& texture, const Rectangle sourceRect) {
        Units u = Units::getInstance();
        if (IsTextureValid(texture)) {
            float diagonalLength = sqrtf((float)(GetScreenWidth() * GetScreenWidth()) + (float)(GetScreenHeight() * GetScreenHeight()));
            float RectXPos = GetScreenWidth() / 2;
            float RectYPos = diagonalLength / 2;
            DrawTexturePro(
                texture,
                sourceRect,
                { RectXPos, -RectYPos * 2, diagonalLength * 2, diagonalLength * 2 },
                { 0, 0 },
                45,
                Color{255, 255, 255, 128}
            );
        }
    }
};

#endif //SONGSELECTMENU_H
