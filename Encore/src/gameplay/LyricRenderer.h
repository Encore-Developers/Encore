#pragma once
#include "enctime.h"
#include "easing/easing.h"
#include "menus/gameMenu.h"
#include "menus/uiUnits.h"
#include "RhythmEngine/Notes/EncNote.h"

namespace Encore {
    class LyricRenderer {
    public:
        void RenderLyrics();

        float AnimTimer = 0.0f;

        void ProcessAnimation() {
            if (AnimTimer > 0)
                AnimTimer -= GetFrameTime() * 3;
            else {
                AnimTimer = 0;
            }
        };

        void DrawPhraseBackground(int type, float pos, float size) {
            Units &u = Units::getInstance();
            float baselineVox = u.hpct(0.0025f) + u.hinpct(pos);
            float voxHeight = u.hinpct(size);
            Rectangle imageRec{ 0, 0, float(ASSET(mainLyricBar).width),
                                float(ASSET(mainLyricBar).height) };
            Rectangle dest{ 0, baselineVox, float(GetRenderWidth()), voxHeight };
            auto texture = ASSETPTR(mainLyricBar);

            if (type == 1)
                texture = ASSETPTR(secLyricBar);

            DrawTexturePro(*texture, imageRec, dest, { 0, 0 }, 0, WHITE);
        }

        void DrawPhrase(RhythmEngine::EncLyricPhrase *phrase,
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
                                     { 119, 183, 255, alpha },
                                     ASSET(sdfShader),
                                     LEFT);
                LyricLeft += MeasureTextEx(*font, playedText.c_str(), FontSize, 0).x;
            }
            GameMenu::mhDrawText(*font,
                                 unplayedText,
                                 { LyricLeft, baselineVox + padding },
                                 FontSize,
                                 {255, 255, 255, alpha},
                                 ASSET(sdfShader),
                                 LEFT);
        }
    };
}

extern Encore::LyricRenderer TheLyricRenderer;