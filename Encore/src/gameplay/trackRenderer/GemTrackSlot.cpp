
#include "GemTrackSlot.h"

#include "assets.h"

// first is color to use, second is name for converted vec4
#define COLOR_TO_VEC4(color, vec4out) \
        Vector4 vec4out = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, \
                               color.a / 255.0f };

#define NOTE_COLOR(vec4) SetShaderValue(ASSET(noteShader),\
                       ASSET(noteShader).GetUniformLoc("noteColor"),\
                       &vec4,\
                       SHADER_UNIFORM_VEC4);

#define FRAME_COLOR(vec4) SetShaderValue(ASSET(noteShader),\
ASSET(noteShader).GetUniformLoc("frameColor"),\
&vec4,\
SHADER_UNIFORM_VEC4);

void Encore::GemTrackSlot::DrawNote(RhythmEngine::EncNote *note) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    float finalWidth = 1;
    Vector3 position = { xPos, 0.0, pos };


    // this is kinda nasty, just wanted a quick Thing
    Color color = track->player.QueryColorProfile(colorSlot);
    COLOR_TO_VEC4(color, vec4color)
    if (note->NoteType == 2) {
        vec4color = { 0.25f, 0.25f, 0.25f, 1.0f };
        COLOR_TO_VEC4(color, frameColor)
        FRAME_COLOR(frameColor)
    } else {
        Vector4 frameColor = { 1, 1, 1, 1 };
        FRAME_COLOR(frameColor)
    }
    NOTE_COLOR(vec4color)
    if (note->NoteType == 1 || note->NoteType == 2) {
        DrawModelEx(ASSET(hopoNote), position,{ 0 }, 0,{ width, 1, 1 }, WHITE);
    } else {
        DrawModelEx(ASSET(regularNote), position,{ 0 }, 0, { width, 1, 1 }, WHITE);
    }

    if (note->LengthSeconds > 0) {
        DrawSustainTail(note->StartSeconds, note->StartSeconds + note->LengthSeconds);
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
        ASSET(smasherPiston).Fetch().materials[0].maps[0].texture = ASSET(smasherOnTex);
        //color.r /= 1.25;
        //color.g /= 1.25;
        //color.b /= 1.25;
    } else {
        ASSET(smasherPiston).Fetch().materials[0].maps[0].texture = ASSET(smasherOffTex);
        //color.r /= 3;
        //color.g /= 3;
        //color.b /= 3;
    }
    DrawModelEx(ASSET(smasherFrame), { xPos, 0.025, 0 }, {0}, 0, { width, 1, 1.3 }, WHITE);
    DrawModelEx(ASSET(smasherPiston), { xPos, 0.025, 0 }, {0}, 0, { width, 1, 1.3 }, color);
    // DrawCube({ xPos, 0.025, 0 }, width, 0.05, 1, color);
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