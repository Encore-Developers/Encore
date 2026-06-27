//
// Created by maria on 01/05/2026.
//

#include "LyricRenderer.h"

#include "raymath.h"

inline unsigned char remapAlpha(unsigned char value, unsigned char max) {
    const auto percent = static_cast<float>(value) / (255.0f);
    return static_cast<unsigned char>(percent * static_cast<float>(max));
}

void Encore::LyricRenderer::RenderLyrics() {
    ProcessAnimation();
    Units& u = Units::getInstance();
    if (!TheSongTime.Lyrics.empty() || displayState != HIDDEN
    ) {
        ZoneScopedN("Lyrics Display")
        auto easeInOut = getEasingFunction(EaseInOutSine);
        auto easeIn = getEasingFunction(EaseInQuad);
        auto easeOut = getEasingFunction(EaseOutQuad);
        DisplayAlpha = static_cast<unsigned char>(easeInOut(ShowHideTimer) * 255.0);
        {
            ZoneScopedN("Draw Next Lyric Phrase")
            DrawPhraseBackground(1, 0.06f, 0.06f * 0.75f);
            DrawPhraseBackground(0, 0, 0.06f);
            float iconWidth = u.hinpct(0.4);
            Rectangle rect {u.RightSide - iconWidth, u.hpct(0) - (iconWidth/2), iconWidth*1.25f, iconWidth*1.25f};
            Rectangle source {0,0,float(ASSET(InstIcons).at(4)->width), float(ASSET(InstIcons).at(4)->height)};
            //
            BeginScissorMode(0,u.hpct(0)+u.hinpct(0.04f)+1, GetRenderWidth(), u.hinpct(0.06f * 0.75f) + u.hinpct(0.06f));
            BeginBlendMode(BLEND_ADDITIVE);
            unsigned char c = static_cast<unsigned char>(easeInOut(ShowHideTimer) * 48.0);
            // if (ShowHideTimer > 0.5)
            DrawTexturePro(*ASSET(InstIcons).at(4), source, rect, {0,0}, 25, {c,c,c,c});
            EndBlendMode();
            EndScissorMode();
            //
            if (TheSongTime.GetNextLyric()) {
                DrawPhrase(
                    TheSongTime.GetNextLyric(),
                    0.05f,
                    0.0425f * 0.75f,
                    (200 - (easeOut(AnimTimer) * 200)));
            }
        }
        {
            ZoneScopedN("Draw Current Lyric Phrase")
            DrawPhrase(
                &TheSongTime.GetCurrentLyric(),
                0.00f + (easeInOut(AnimTimer) * 0.05f),
                0.0425f - (easeInOut(AnimTimer) * (0.0425f * 0.25f)),
                255 - (easeInOut(AnimTimer) * 55));
        }
        {
            ZoneScopedN("Animate Out Last Lyric Phrase")
            if (TheSongTime.GetPreviousLyric()) {
                DrawPhrase(
                    TheSongTime.GetPreviousLyric(),
                    0.00f, // + (AnimTimer * 0.05f),
                    (0.0425f), // + (AnimTimer * (0.0425f * 0.25f)),
                    0 + (easeIn(AnimTimer) * 200));
            }
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
    ZoneScopedN("Draw Phrase Background")
    Units &u = Units::getInstance();
    float baselineVox = u.hpct(0) + u.hinpct(0.04) + u.hinpct(pos);
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
    ZoneScopedN("Draw Lyric Phrase")
    Units &u = Units::getInstance();

    const float baselineVox = u.hpct(0) + u.hinpct(0.04) + u.hinpct(pos);
    const float voxHeight = u.hinpct(0.06f);
    const float FontSize = u.hinpct(size);
    const float padding = (voxHeight - FontSize) / 2;

    // god forbid i have a proper ui library
    float LyricLeft = 0;

    std::string allLyrics;
    TextDisplay lyricData;
    for (const auto &lyric : phrase->lyrics) {
        allLyrics += lyric.Lyric;
    }
    lyricData.Size(FontSize);
    LyricLeft += int(GetRenderWidth() / 2) - (lyricData.TextWidth(allLyrics)/2);
    lyricData.Pos(LyricLeft,baselineVox + padding);
    const Color PlayedColor = { 119, 183, 255, remapAlpha(alpha, DisplayAlpha) };
    const Color UnplayedColor = {255, 255, 255, remapAlpha(remapAlpha(alpha, DisplayAlpha), 196)};
    for (int i = 0; i < phrase->lyrics.size(); i++) {
        RhythmEngine::EncLyric& lyric = phrase->lyrics[i];
        float EndSec = phrase->EndSec;

        if (lyric.talkie) lyricData.font = ASSET(rubikItalic);
        else lyricData.font = ASSET(rubik);
        if (lyric.StartSec < TheSongTime.GetElapsedTime()) {
            if (i != phrase->lyrics.size() - 1) {
                EndSec = phrase->lyrics[i+1].StartSec;
            }
            if (EndSec < TheSongTime.GetElapsedTime()) {
                lyricData.Col( PlayedColor ).DrawText(lyric.Lyric);
            } else {
                const int padding2 = u.winpct(0.01f);
                const float percentage = Remap(TheSongTime.GetElapsedTime(), lyric.StartSec, EndSec, 0, 1);
                const int playedWidth = float(lyricData.TextWidth(lyric.Lyric)) * percentage;
                const int unplayedWidth = float(lyricData.TextWidth(lyric.Lyric)) * (1 - percentage);
                const int y = int(baselineVox + padding);
                const int x = lyricData.pos.x;
                BeginScissorMode(x + playedWidth, y, unplayedWidth + padding2, FontSize);
                lyricData.Col( UnplayedColor ).DrawText(lyric.Lyric);
                EndScissorMode();

                BeginScissorMode(x - padding2, y, playedWidth + padding2, FontSize);
                lyricData.Col( PlayedColor ).DrawText(lyric.Lyric);
                EndScissorMode();
            }
        } else {
            lyricData.Col( UnplayedColor );
            lyricData.DrawText(lyric.Lyric);
        }
        lyricData.pos.x += lyricData.TextWidth(lyric.Lyric);
    }
/*
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
        Text::DrawText(*font,
                             playedText,
                             { LyricLeft, baselineVox + padding },
                             FontSize,
                             { 119, 183, 255, remapAlpha(alpha, DisplayAlpha) },
                             LEFT);
        LyricLeft += MeasureTextEx(*font, playedText.c_str(), FontSize, 0).x;
    }
    Text::DrawText(*font,
                         unplayedText,
                         { LyricLeft, baselineVox + padding },
                         FontSize,
                         {255, 255, 255, remapAlpha(alpha, DisplayAlpha)},
                         LEFT);
                         */
}