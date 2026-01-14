//
// Created by maria on 14/01/2026.
//

#include "GuitarTrack.h"

#include "gameplay/enctime.h"

double GetNotePos3D(double noteTime, double songTime, float noteSpeed, float length) {
    return ((noteTime - songTime) * (noteSpeed * length));
}

std::array<Color, 5> grybo = { GREEN, RED, YELLOW, BLUE, ORANGE };
std::array<Color, 5> orybg = { ORANGE, RED, YELLOW, BLUE, GREEN };

void Encore::GuitarTrack::DrawStrikeline() {
    DrawCube({ 0, 0, 0 }, 5, 0.1, 0.1, BLACK);
    for (int g = 0; g < player.engine->stats->HeldFrets.size(); g++) {
        if (player.engine->stats->HeldFrets[g]) {
            Color background = grybo[g];
            DrawCube(
                { 2.0f - g, 0, 0 },
                1,
                0.01,
                1,
                background
            );
        }
    }
    if (player.engine->stats->strumState == RhythmEngine::StrumState::UpStrum) {
        DrawCube({ 0, 0, 0.2 }, 5, 0.02, 0.4, ColorAlpha(WHITE, 0.75));
    }
    if (player.engine->stats->strumState == RhythmEngine::StrumState::DownStrum) {
        DrawCube({ 0, 0, -0.2 }, 5, 0.02, 0.4, ColorAlpha(WHITE, 0.75));
    }
    // DrawCube({0,0,0}, 5, 0.1, 0.1, BLACK);
}

void Encore::GuitarTrack::DrawNotes() {
    if (player.engine->chart->at(0).empty()) {
        return;
    }

    std::pair<int, int> NotePoolSize = player.engine->GetNotePoolSize();
    for (int curNote = NotePoolSize.first; curNote < NotePoolSize.second; curNote++) {
        auto &note = player.engine->chart->at(0).at(curNote);
        float ScrollPos = GetNotePos3D(note.StartSeconds,
                                       TheSongTime.GetElapsedTime(),
                                       1,
                                       20);
        float NoteLength = GetNotePos3D(
            note.StartSeconds + note.LengthSeconds,
            TheSongTime.GetElapsedTime(),
            1,
            20
        );
        float ScrollEndPos = ScrollPos - NoteLength;
        bool sust = false;
        int sustLength = ScrollPos - 1;
        if (note.LengthTicks == 0) {
            NoteLength = ScrollPos - 1;
            ScrollEndPos = 1;
        } else {
            sust = true;
        }

        uint8_t x = note.Lane;

        // DrawRectangle(0, 1, 1, 1 * 2, GREEN);
        if (x == 0) {
            float pos = 0;
            float width = 1.0f;
            Color color = PURPLE;
            width = 1 * 4;
            Vector3 position = { pos, 0 + 0.125, ScrollPos};
            if (sust) {
                DrawCube({ position.x, position.y, position.z - (ScrollEndPos / 2) },
                         0.2,
                         0.2,
                         ScrollEndPos,
                         color);
            }
            DrawCube(position, width, 0.25, 0.5, color);
        }
        while (x) {
            uint8_t y = x & ~(x - 1);
            float pos = 2.0f;
            float width = 1;
            Color color = GREEN;
            if (x == 0) {
                color = PURPLE;
                width = 1 * 4;
            } else if (y == Encore::RhythmEngine::PlasticFrets[1]) {
                pos -= 1;
                color = RED;
            } else if (y == Encore::RhythmEngine::PlasticFrets[2]) {
                pos -= 1 * 2;
                color = YELLOW;
            } else if (y == Encore::RhythmEngine::PlasticFrets[3]) {
                pos -= 1 * 3;
                color = BLUE;
            } else if (y == Encore::RhythmEngine::PlasticFrets[4]) {
                pos -= 1 * 4;
                color = ORANGE;
            }
            if (note.NotePassed)
                color = MAROON;
            // DrawRectangle(pos, NoteLength, NoteXWidth, ScrollEndPos, color);
            Vector3 position = { pos, 0 + 0.125, ScrollPos};
            if (sust) {
                DrawCube({ position.x, position.y, position.z - (ScrollEndPos / 2) },
                         0.2,
                         0.2,
                         ScrollEndPos,
                         color);
            }

            switch (note.NoteType) {
            case 1: {
                color = ColorBrightness(color, 0.75);
                width = 0.5f;
                break;
            };
            case 2: {
                color = ColorBrightness(color, -0.75);
                width = 0.5f;
                break;
            };
            default:
                break;
            }
            DrawCube(position, width, 0.25, 0.5, color);
            //if (note.NoteType == 1) {
            //    DrawRectangle(
            //        pos + 5,
            //        sustLength + 5,
            //        width - 10,
            //        NoteHeight - 10,
            //        WHITE
            //    );
            //}
            x &= (x - 1);
        }
    }
    if (player.engine->chart->HeldNotePointers.at(0)) {
            auto &note = player.engine->chart->HeldNotePointers.at(0);
        float ScrollPos = GetNotePos3D(note->StartSeconds,
                                    TheSongTime.GetElapsedTime(),
                                    1,
                                    20);
        float NoteLength = GetNotePos3D(
            note->StartSeconds + note->LengthSeconds,
            TheSongTime.GetElapsedTime(),
            1,
            20
        );
            float ScrollEndPos = 0 - NoteLength;
            float ScrollStartPos = ScrollPos;

            uint8_t x = note->Lane;
            while (x) {
                uint8_t y = x & ~(x - 1);
                float pos = 2.0f;
                Color color = GREEN;
                if (y == Encore::RhythmEngine::PlasticFrets[1]) {
                    pos -= 1;
                    color = RED;
                } else if (y == Encore::RhythmEngine::PlasticFrets[2]) {
                    pos -= 1 * 2;
                    color = YELLOW;
                } else if (y == Encore::RhythmEngine::PlasticFrets[3]) {
                    pos -= 1 * 3;
                    color = BLUE;
                } else if (y == Encore::RhythmEngine::PlasticFrets[4]) {
                    pos -= 1 * 4;
                    color = ORANGE;
                }
                if (note->NotePassed)
                    color = MAROON;
                Vector3 position = { pos, 0 + 0.125, 0};
                DrawCube({ position.x, position.y, position.z - (ScrollEndPos /2 ) },
                         0.2,
                         0.2,
                         ScrollEndPos,
                         color);
                //if (note->NoteType == 1) {
                //    DrawRectangle(
                //        pos + 5,
                //        NoteLength + 5,
                //        NoteXWidth - 10,
                //        ScrollEndPos - 10,
                //        WHITE
                //    );
                //}
                x &= (x - 1);
            }
        }
    };

    void Encore::GuitarTrack::DrawBeatlines()
    {
        if (!TheSongTime.Beatlines.empty()) {
            if (TheSongTime.Beatlines.front().time < TheSongTime.GetElapsedTime() - 1) {
                TheSongTime.Beatlines.erase(TheSongTime.Beatlines.begin());
            }

            int beatlinePoolMaxSize = NOTE_POOL_SIZE / 4;
            size_t BeatlinePoolSize = TheSongTime.Beatlines.size() > beatlinePoolMaxSize
                ? beatlinePoolMaxSize
                : TheSongTime.Beatlines.size();
            // because i have to do bounds checks myself
            BeatlinePool = { TheSongTime.Beatlines.begin(), BeatlinePoolSize };

            for (auto &beatline : BeatlinePool) {
                float ScrollPos = GetNotePos3D(
                    beatline.time,
                    TheSongTime.GetElapsedTime(),
                    1,
                    20
                );
                float Size = 0;
                Color beatlineColor = WHITE;
                switch (beatline.type) {
                case Major: {
                    beatlineColor = GRAY;
                    Size = 0.05;
                    break;
                }
                case Minor: {
                    beatlineColor = DARKGRAY;
                    Size = 0.01;
                    break;
                }
                case Measure: {
                    beatlineColor = WHITE;
                    Size = 0.1;
                    break;
                }
                }
                DrawCube(
                    { 0, 0, ScrollPos },
                    5,
                    Size,
                    Size,
                    beatlineColor
                );
            }
        }
    };