
#include "GemTrackSlot.h"

#include "assets.h"
#include "easing/easing.h"
#include "rlgl.h"

#include "Track.h"

#include "song/song.h"

void Encore::GemTrackSlot::DrawNote(RhythmEngine::EncNote *note, bool missed) {
    auto pos = track->GetNotePos3D(note->StartSeconds);
    Vector3 position = { xPos, 0.0, pos };

    // this is kinda nasty, just wanted a quick Thing
    Color color = track->player.QueryColorProfile(colorSlot);
    if (note->NoteType == 2) {
        ASSET(noteShader).SetUniform("frameColor", color);
        ASSET(noteShader).SetUniform("noteColor", Color{ 60, 60, 60, 255 });
    } else {
        if (track->player.engine->chart->overdrive.RenderNotesAsOD(note->StartSeconds)) {
            color = track->player.QueryColorProfile(SLOT_OVERDRIVE);
            ASSET(noteShader).SetUniform("frameColor", GOLD);
        } else {
            ASSET(noteShader).SetUniform("frameColor", WHITE);
        }
        ASSET(noteShader).SetUniform("noteColor", color);
    }
    if (missed) {
        ASSET(noteShader).SetUniform("frameColor", Color{120, 120, 120, 255});
        ASSET(noteShader).SetUniform("noteColor", Color{ 255, 50, 50, 255 });
    }

    if (note->LengthSeconds > 0) {
        DrawSustainTail(note->StartSeconds, note->StartSeconds + note->LengthSeconds);
    }

    rlDrawRenderBatchActive();

    if (note->NoteType == 1 || note->NoteType == 2) {
        //todo: cymbal lane
        if (track->player.Instrument == PlasticDrums) {
            position.z -= 0.1f;
            DrawModelEx(ASSET(cymbalNote),
                        position,
                        { -1, 0, 0 },
                        10,
                        { width, track->NoteHeight, width * 0.75f },
                        WHITE);
        } else if (track->player.Instrument < PlasticDrums) {
            DrawModelEx(ASSET(liftNote),
                        position,
                        { 0 },
                        0,
                        { width, track->NoteHeight, width },
                        WHITE);
        } else {
            DrawModelEx(ASSET(hopoNote),
                        position,
                        { 0 },
                        0,
                        { width, track->NoteHeight, 0.85f },
                        WHITE);
        }
    } else {
        DrawModelEx(ASSET(regularNote),
                    position,
                    { 0 },
                    0,
                    { width, track->NoteHeight, 0.85f },
                    WHITE);
    }

    //DrawCube({xPos, 0.2, pos}, finalWidth, 0.4, 0.5, track->player.QueryColorProfile(colorSlot));
}

void Encore::GemTrackSlot::DrawSustainTail(double startTime, double endTime) {
    if (endTime <= startTime) {
        return;
    }
    Color color = track->player.QueryColorProfile(colorSlot);
    if (track->player.engine->chart->overdrive.RenderNotesAsOD(startTime)) {
        color = WHITE;
    }
    float midPos = track->GetNotePos3D((endTime + startTime) / 2);
    float sustainLength = endTime - startTime;
    DrawCube({ xPos, 0.1, midPos },
             0.2 + track->player.engine->whammy,
             0.1,
             sustainLength * track->GetZPerSecond(),
             color);
}

void Encore::GemTrackSlot::DrawSmasher(bool held) {
    if (parentSlot) {
        return;
    }
    Color color = track->player.QueryColorProfile(colorSlot);

    if (held) {
        ASSET(smasherPiston).Fetch().materials[0].maps[0].texture = ASSET(smasherOnTex);
    } else {
        ASSET(smasherPiston).Fetch().materials[0].maps[0].texture = ASSET(smasherOffTex);
    }

    if (animTimer < 1) {
        animTimer += GetFrameTime() * 4;
    } else {
        animTimer = 1;
    }

    if (overhitTimer > 0) {
        overhitTimer -= GetFrameTime() * 1.8;
    } else {
        overhitTimer = 0;
    }

    static auto easeOutBounce = getEasingFunction(EaseOutBounce);
    double ease = easeOutBounce(animTimer);
    float bounce = (1 - ease) * 0.32;

    DrawModelEx(ASSET(smasherFrame),
                { xPos, 0.025, 0 },
                { 0 },
                0,
                { width, 1, 1.3f * length },
                WHITE);
    DrawModelEx(ASSET(smasherPiston),
                { xPos, 0.025f + bounce, 0 - bounce * 0.2f },
                { 0 },
                0,
                { width, 1, 1.3f * length },
                ColorLerp(color, {50, 50, 50, 255}, overhitTimer));
    // DrawCube({ xPos, 0.025, 0 }, width, 0.05, 1, color);
    //for (auto note : track->player.engine->chart->PerfectNotePointers) {
    //    if (!note) {
    //        continue;
    //    }
    //    if (shockwaveParticle) {
    //        if (shockwaveParticle->id == shockwaveId)
    //            shockwaveParticle->color = GOLD;
    //    }
    //}
    for (auto note : track->player.engine->chart->HeldNotePointers) {
        if (!note)
            continue;
        if (note->StartSeconds + note->LengthSeconds < TheSongTime.GetElapsedTime()) {
            continue;
        }
        bool matches = false;
        if (note->Lane & RhythmEngine::PlasticFrets[index]) {
            matches = true;
        }

        if (matches) {
            DrawSustainTail(TheSongTime.GetElapsedTime(),
                            note->StartSeconds + note->LengthSeconds);
            if (hitFlare) {
                if (hitFlare->id == hitFlareId) {
                    hitFlare->time = (std::sin(TheSongTime.GetElapsedTime() * 100) + 1) *
                        0.5 * FLARE_LIFETIME * 0.1 + 0.02;
                    //hitFlare->time = 0;
                }
            }
            break;
        }
    }
}

void Encore::GemTrackSlot::AnimateHit(bool perfect, Color colorg) {
    if (parentSlot) {
        parentSlot->AnimateHit(perfect, colorg);
        return;
    }
    animTimer = 0;
    overhitTimer = 0;
    Particle part;
    part.active = true;
    part.type = FLARE;
    part.position = { xPos, 0.05, 0 };
    part.color = colorg;
    hitFlare = track->particleSystem->SpawnParticle(part);
    hitFlareId = hitFlare->id;

    Particle shockwave;
    Color color = WHITE;
    color.a = 32;

    if (perfect) {
        color = GOLD;
        color.a = 196;
    }

    shockwave.setActive(true)
             .setType(SHOCKWAVE)
             .pos({ xPos, 0.05, 0 })
             .col(color);
    shockwaveParticle = track->particleSystem->SpawnParticle(shockwave);
    shockwaveId = shockwaveParticle->id;
}
void Encore::GemTrackSlot::AnimateOverhit() {
    if (parentSlot) {
        parentSlot->AnimateOverhit();
    }
    animTimer = 0.25;
    overhitTimer = 1;
}