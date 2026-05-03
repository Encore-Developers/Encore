//
// Created by maria on 01/05/2026.
//

#include "LyricRenderer.h"

void Encore::LyricRenderer::RenderLyrics() {
    ProcessAnimation();
    if (!TheSongTime.Lyrics.empty()
    ) {
        ZoneScopedN("Lyrics Display")
        auto easeInOut = getEasingFunction(EaseInOutSine);
        auto easeIn = getEasingFunction(EaseInQuad);
        auto easeOut = getEasingFunction(EaseOutQuad);

        if (TheSongTime.CurrentLyricPhrase < TheSongTime.Lyrics.size() - 1) {
            DrawPhraseBackground(1, 0.11f, 0.06f * 0.75f);
            DrawPhrase(
                &TheSongTime.Lyrics.at(TheSongTime.CurrentLyricPhrase + 1),
                0.10f,
                0.0425f * 0.75f,
                (200 - (easeOut(AnimTimer) * 200)));
        }
        DrawPhraseBackground(0, 0.05, 0.06f);
        DrawPhrase(
            &TheSongTime.Lyrics.at(TheSongTime.CurrentLyricPhrase),
            0.05f + (easeInOut(AnimTimer) * 0.05f),
            0.0425f - (easeInOut(AnimTimer) * (0.0425f * 0.25f)),
            255 - (easeInOut(AnimTimer) * 55));

        if (TheSongTime.CurrentLyricPhrase > 0) {
            DrawPhrase(
                &TheSongTime.Lyrics.at(TheSongTime.CurrentLyricPhrase-1),
                0.05f, // + (AnimTimer * 0.05f),
                (0.0425f), // + (AnimTimer * (0.0425f * 0.25f)),
                0 + (easeIn(AnimTimer) * 200));
        }
        if (TheSongTime.Lyrics.at(TheSongTime.CurrentLyricPhrase).EndSec <
            TheSongTime.
            GetElapsedTime()) {
            if (TheSongTime.CurrentLyricPhrase < TheSongTime.Lyrics.size() - 1) {
                AnimTimer = 1;
                TheSongTime.CurrentLyricPhrase++;
            }
        }
    }
}