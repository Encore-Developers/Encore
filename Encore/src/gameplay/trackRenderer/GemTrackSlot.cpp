
#include "GemTrackSlot.h"

#include "assets.h"

void Encore::GemTrackSlot::DrawNote(RhythmEngine::EncNote *note) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    float finalWidth = 1;

    if (note->LengthSeconds > 0) {
        DrawSustainTail(note->StartSeconds, note->StartSeconds + note->LengthSeconds);
    }
    // this is kinda nasty, just wanted a quick Thing
    Color color = track->player.QueryColorProfile(colorSlot);
    Vector4 vec4color = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
                          color.a / 255.0f };
    if (note->NoteType == 2) {
        vec4color = { 0.25f, 0.25f, 0.25f, 1.0f };
        Vector4 frameColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
                               color.a / 255.0f };
        SetShaderValue(ASSET(noteShader),
                       ASSET(noteShader).GetUniformLoc("frameColor"),
                       &frameColor,
                       SHADER_UNIFORM_VEC4);
    } else {
        Vector4 frameColor = { 1,1,1,1 };
        SetShaderValue(ASSET(noteShader),
                       ASSET(noteShader).GetUniformLoc("frameColor"),
                       &frameColor,
                       SHADER_UNIFORM_VEC4);
    }
    SetShaderValue(ASSET(noteShader),
                   ASSET(noteShader).GetUniformLoc("noteColor"),
                   &vec4color,
                   SHADER_UNIFORM_VEC4);
    if (note->NoteType == 1 || note->NoteType == 2) {
        DrawModelEx(ASSET(hopoNote),
                    { xPos, 0.0, pos },
                    { 0, 0, 0 },
                    0,
                    { width, 1, 1 },
                    WHITE);
    } else {
        DrawModelEx(ASSET(regularNote),
                    { xPos, 0.0, pos },
                    { 0, 0, 0 },
                    0,
                    { width, 1, 1 },
                    WHITE);
    }
    //DrawCube({xPos, 0.2, pos}, finalWidth, 0.4, 0.5, track->player.QueryColorProfile(colorSlot));
}

void Encore::GemTrackSlot::DrawSustainTail(double startTime, double endTime) {
    if (endTime <= startTime) {
        return;
    }
    float midPos = track->GetNotePos3D((endTime + startTime) / 2);
    float sustainLength = endTime - startTime;

    DrawCube({ xPos, 0.1, midPos },
             0.2,
             0.2,
             sustainLength * track->GetZPerSecond(),
             track->player.QueryColorProfile(colorSlot));
}

void Encore::GemTrackSlot::DrawSmasher(bool held) {
    // HACK: don't render open and kick smasher
    // REMOVE THIS LATER!
    if (colorSlot == SLOT_OPEN || colorSlot == SLOT_KICK) {
        return;
    }

    Color color = track->player.QueryColorProfile(colorSlot);

    if (held) {
        color.r /= 1.25;
        color.g /= 1.25;
        color.b /= 1.25;
    } else {
        color.r /= 3;
        color.g /= 3;
        color.b /= 3;
    }

    DrawCube({ xPos, 0.025, 0 }, width, 0.05, 1, color);
    for (auto note : track->player.engine->chart->HeldNotePointers) {
        if (!note)
            continue;
        bool matches = false;
        if (track->player.engine->UsesNoteMasks()) {
            if (note->Lane & RhythmEngine::PlasticFrets[index]) {
                matches = true;
            }
        } else {
            if (note->Lane == index) {
                matches = true;
            }
        }
        if (matches) {
            DrawSustainTail(TheSongTime.GetElapsedTime(),
                            note->StartSeconds + note->LengthSeconds);
            break;
        }
    }
}