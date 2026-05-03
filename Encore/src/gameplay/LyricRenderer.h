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

        void DrawPhraseBackground(int type, float pos, float size);

        void DrawPhrase(RhythmEngine::EncLyricPhrase *phrase,
                        float pos,
                        float size,
                        unsigned char alpha);
    };
}

extern Encore::LyricRenderer TheLyricRenderer;