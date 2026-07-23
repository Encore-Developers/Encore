#pragma once
#include "enctime.h"
#include "easing/easing.h"
#include "../menus/main/MainMenu.h"
#include "../menus/util/uiUnits.h"

namespace Encore {
    class LyricRenderer {
        enum DisplayState {
            SHOWN,
            FADE_IN,
            FADE_OUT,
            HIDDEN
        };
        DisplayState displayState = HIDDEN;
        void ShowLyrics() {
            displayState = FADE_IN;
            ShowHideTimer = 0;
        };

        void HideLyrics() {
            displayState = FADE_OUT;
            ShowHideTimer = 1;
        }
    public:
        void RenderLyrics();

        float AnimTimer = 0.0f;
        float ShowHideTimer = 0.0f;
        unsigned char DisplayAlpha = 255;



        void ProcessAnimation() {
            if (AnimTimer > 0)
                AnimTimer -= GetFrameTime() * 3;
            else {
                AnimTimer = 0;
            }
            if (displayState == FADE_IN) {
                if (ShowHideTimer < 1)
                    ShowHideTimer += GetFrameTime() * 3;
                else {
                    displayState = SHOWN;
                    ShowHideTimer = 1;
                }
            }
            if (displayState == FADE_OUT) {
                if (ShowHideTimer > 0)
                    ShowHideTimer -= GetFrameTime() * 3;
                else {
                    displayState = HIDDEN;
                    ShowHideTimer = 0;
                }
            }
        };

        void DrawPhraseBackground(int type, float pos, float size);

        void DrawPhrase(RhythmEngine::LyricPhrase *phrase,
                        float pos,
                        float size,
                        unsigned char alpha);
    };
}

extern Encore::LyricRenderer TheLyricRenderer;