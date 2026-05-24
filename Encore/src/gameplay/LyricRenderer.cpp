//
// Created by maria on 01/05/2026.
//

#include "LyricRenderer.h"


inline unsigned char remapAlpha(unsigned char value, unsigned char max) {
    const auto percent = static_cast<float>(value) / (255.0f);
    return static_cast<unsigned char>(percent * static_cast<float>(max));
}

void Encore::LyricRenderer::RenderLyrics() {
    ProcessAnimation();
    if (!TheSongTime.Lyrics.empty()
    ) {
        ZoneScopedN("Lyrics Display")
        auto easeInOut = getEasingFunction(EaseInOutSine);
        auto easeIn = getEasingFunction(EaseInQuad);
        auto easeOut = getEasingFunction(EaseOutQuad);
        DisplayAlpha = static_cast<unsigned char>(easeInOut(ShowHideTimer) * 255.0);
        DrawPhraseBackground(1, 0.11f, 0.06f * 0.75f);
        if (TheSongTime.GetNextLyric()) {
            DrawPhrase(
                TheSongTime.GetNextLyric(),
                0.10f,
                0.0425f * 0.75f,
                (200 - (easeOut(AnimTimer) * 200)));
        }
        DrawPhraseBackground(0, 0.05, 0.06f);
        DrawPhrase(
            &TheSongTime.GetCurrentLyric(),
            0.05f + (easeInOut(AnimTimer) * 0.05f),
            0.0425f - (easeInOut(AnimTimer) * (0.0425f * 0.25f)),
            255 - (easeInOut(AnimTimer) * 55));

        if (TheSongTime.GetPreviousLyric()) {
            DrawPhrase(
                TheSongTime.GetPreviousLyric(),
                0.05f, // + (AnimTimer * 0.05f),
                (0.0425f), // + (AnimTimer * (0.0425f * 0.25f)),
                0 + (easeIn(AnimTimer) * 200));
        }
        if (TheSongTime.Lyrics.at(TheSongTime.CurrentLyricPhrase).EndSec <
            TheSongTime.GetElapsedTime()) {
            if (TheSongTime.CurrentLyricPhrase < TheSongTime.Lyrics.size() - 1) {
                AnimTimer = 1;
                TheSongTime.CurrentLyricPhrase++;
            }
        }

        if (TheSongTime.GetNextLyric()) {
            RhythmEngine::EncLyricPhrase* NextLyric = TheSongTime.GetNextLyric();
            RhythmEngine::EncLyricPhrase& CurrentLyric = TheSongTime.GetCurrentLyric();
            if (CurrentLyric.lyrics.empty() && NextLyric->lyrics.empty() && NextLyric->StartSec - 3 > TheSongTime.GetElapsedTime() && (displayState == FADE_IN || displayState == SHOWN)) {
                HideLyrics();
            }
            else if (!NextLyric->lyrics.empty() && NextLyric->StartSec - 3 < TheSongTime.GetElapsedTime() && (displayState == FADE_OUT || displayState == HIDDEN)) {
                ShowLyrics();
            }
        }
        if (TheSongTime.CurrentLyricPhrase == TheSongTime.Lyrics.size() - 1 && (displayState == FADE_IN || displayState == SHOWN)) {
            HideLyrics();
        }
    }
}

void Encore::LyricRenderer::DrawPhraseBackground(int type, float pos, float size) {
    Units &u = Units::getInstance();
    float baselineVox = u.hpct(0.0025f) + u.hinpct(pos);
    float voxHeight = u.hinpct(size);
    Rectangle imageRec{ 0, 0, float(ASSET(mainLyricBar).width),
                        float(ASSET(mainLyricBar).height) };
    Rectangle dest{ 0, baselineVox, float(GetRenderWidth()), voxHeight };
    auto texture = ASSETPTR(mainLyricBar);

    if (type == 1)
        texture = ASSETPTR(secLyricBar);

    DrawTexturePro(*texture, imageRec, dest, { 0, 0 }, 0, {255,255,255, DisplayAlpha});
}

void Encore::LyricRenderer::DrawPhrase(RhythmEngine::EncLyricPhrase *phrase,
    float pos,
    float size,
    unsigned char alpha) {
    Units &u = Units::getInstance();

    float baselineVox = u.hpct(0.0025f) + u.hinpct(pos);
    float voxHeight = u.hinpct(0.06f);
    float FontSize = u.hinpct(size);
    float padding = (voxHeight - FontSize) / 2;

    // god forbid i have a proper ui library
    float LyricLeft = 0;

    std::string playedText;
    std::string unplayedText;

    auto font = ASSETPTR(rubik);

    for (const auto &lyric : phrase->lyrics) {
        if (lyric.StartSec < TheSongTime.GetElapsedTime()) {
            playedText += lyric.Lyric;
        } else {
            unplayedText += lyric.Lyric;
        }
        // also this needs to be fixed (talkies dont display properly)
        //if (lyric.talkie)
        //    font = ASSETPTR(rubikBoldItalic);
    }
    LyricLeft += int(GetRenderWidth() / 2) - (MeasureTextEx(
        *font,
        (playedText + unplayedText).c_str(),
        FontSize,
        0).x / 2);

    if (!playedText.empty()) {
        GameMenu::mhDrawText(*font,
                             playedText,
                             { LyricLeft, baselineVox + padding },
                             FontSize,
                             { 119, 183, 255, remapAlpha(alpha, DisplayAlpha) },
                             ASSET(sdfShader),
                             LEFT);
        LyricLeft += MeasureTextEx(*font, playedText.c_str(), FontSize, 0).x;
    }
    GameMenu::mhDrawText(*font,
                         unplayedText,
                         { LyricLeft, baselineVox + padding },
                         FontSize,
                         {255, 255, 255, remapAlpha(alpha, DisplayAlpha)},
                         ASSET(sdfShader),
                         LEFT);
}